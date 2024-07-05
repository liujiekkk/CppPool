#include "Worker.h"
#include "TaskQueue.h"
#include <iostream>

int ljPool::Worker::m_runningWorkerNum = 0;

ljPool::Worker::Worker(TaskQueueSource& source, const int& maxNum,long long timeout)
	: m_running(false), m_thread(), m_queue(source.taskQueue), m_mutexQueue(source.mutexQueue),
	m_cvQueueEmpty(source.cvQueueEmpty), m_cvQueueFull(source.cvQueueFull), m_busy(false), 
	m_timeout(timeout), m_coreWorkerNum(maxNum)
{
}

ljPool::Worker::~Worker()
{
	// 停止当前 worker 线程.
	stop();
	// 回收当前 worker 线程资源.
	if (m_thread.joinable()) {
		m_thread.join();
	}
}

void ljPool::Worker::start()
{
	// 期望值 false.
	bool expected = false;
	if (!m_running.compare_exchange_weak(expected, true)) {
		return;
	}
	// 运行 worker 数+1.
	++m_runningWorkerNum;
	// worker 的工作函数.
	auto func = [this]() {
		run();
		};
	// 启动线程.
	m_thread = std::thread(func);
}

void ljPool::Worker::stop()
{
	bool expected = true;
	if (!m_running.compare_exchange_weak(expected, false)) {
		return;
	}
	// 通知所有 worker 线程检测停止状态，完成子线程.
	m_cvQueueEmpty.notify_all();
}

bool ljPool::Worker::isActive() const
{
	// 判断空闲时间是否超过最大空闲时间.
	if (m_busy) {
		return true;
	}
	return false;
}

bool ljPool::Worker::isRunning() const
{
	return m_running;
}

void ljPool::Worker::run()
{
	Task func;
	while (true) {
		{
			std::unique_lock<std::mutex> ulock(m_mutexQueue);
			// 当线程池处于运行态，且队列无任务时需要阻塞,其他情况需要放行（执行任务，或者线程完成）.
			auto ret = m_cvQueueEmpty.wait_for(ulock, m_timeout, [this]() {
					return !(m_running && m_queue.empty());
				});
			// 如果是超时情况，自动修改线程状态为停止.
			if (!ret) {
				m_running.store(false);
			}
			// 此处判断是否要退出，且退出时候应该保证投递的任务全部被执行完.
			if (!m_running && m_queue.empty()) {
				--m_runningWorkerNum;
				return;
			}
			// 当前 worker 处于忙碌状态.
			m_busy = true;
			// 取出一个任务.
			func = m_queue.front();
			// 删除任务节点.
			m_queue.pop();
			// 通知满队列条件变量.
			m_cvQueueFull.notify_one();
		}
		// 执行任务.
		func();
		// 当前 worker 结束忙碌状态.
		m_busy = false;
	}
}
