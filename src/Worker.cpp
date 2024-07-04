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
	while (m_running) {
		{
			std::unique_lock<std::mutex> ulock(m_mutexQueue);
			while (m_queue.empty()) {
				auto status = m_cvQueueEmpty.wait_for(ulock, m_timeout);
				// 如果是超时未收到任务,且当前worker 数量大于核心 worker 数,直接退出当前线程.
				if (status == std::cv_status::timeout && m_runningWorkerNum > m_coreWorkerNum) {
					// std::cout << "timeout!!!" << std::endl;
					// 运行状态修改为停止，等待监控进程回收.
					m_running.store(false);
				}
				// 判断线程池是否停止.平滑停止.
				if (!m_running) {
					--m_runningWorkerNum;
					return;
				}
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
