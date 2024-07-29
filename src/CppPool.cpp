#include "CppPool.h"
#include "Worker.h"
#include <iostream>

ljPool::CppPool::CppPool(int maxNum, int minNum, size_t maxQueueSize)
	:m_coreNum(minNum), m_maxNum(maxNum), m_workerList(), m_mutexWorkerList(),
	m_queueSource{ maxQueueSize, {}, {}, {}, {} }, m_manager()
{
}

ljPool::CppPool::~CppPool()
{
	// std::cout << "Pool has worker num: " << getWorkerNum() << " task:" << m_queueSource.taskQueue.size() << std::endl;
	shutdown();
}

void ljPool::CppPool::run()
{
	// 启动线程池，则改变状态为false，然后停止线程池.
	bool expected = false;
	if (!m_running.compare_exchange_weak(expected, true)) {
		return;
	}
	// 初始化核心 worker.
	createWorker(m_coreNum);
	// 开启监控线程.
	auto func = [this]() {recycle(); };
	m_manager = std::thread(func);
}

void ljPool::CppPool::shutdown()
{
	bool expected = true;
	if (!m_running.compare_exchange_weak(expected, false)) {
		return;
	}
	// 回收监控线程.
	if (m_manager.joinable()) {
		m_manager.join();
	}
	// 停止 worker 线程.
	stopWorkers();
}

int ljPool::CppPool::createWorker(int num)
{
	if (num > 0) {
		std::lock_guard<std::mutex> lock(m_mutexWorkerList);
		// 计算可以新建的 worker 数量.
		if (num > m_maxNum - m_workerList.size()) {
			num = m_maxNum - (int)m_workerList.size();
		}
		for (int i = 0; i < num; ++i) {
			auto wptr = std::make_shared<Worker>(m_queueSource, *this, m_coreNum);
			m_workerList.emplace_back(wptr);
			wptr->start();
		}
		return num;
	}
	return 0;
}

size_t ljPool::CppPool::getWorkerNum()
{
	std::lock_guard<std::mutex> lock(m_mutexWorkerList);
	return m_workerList.size();
}

size_t ljPool::CppPool::getRunningWorkerNum()
{
	std::lock_guard<std::mutex> lock(m_mutexWorkerList);
	return std::count_if(m_workerList.cbegin(), m_workerList.cend(), [](const std::shared_ptr<Worker>& ptr) {
		// 判断活跃 worker.
		return ptr->isRunning();
		});
}

size_t ljPool::CppPool::getActiveWorkerNum()
{
	std::lock_guard<std::mutex> lock(m_mutexWorkerList);
	return std::count_if(m_workerList.cbegin(), m_workerList.cend(), [](const std::shared_ptr<Worker>& ptr) {
		// 判断活跃 worker.
		return ptr->isActive();
		});
}

size_t ljPool::CppPool::getInactiveWorkerNum()
{
	std::lock_guard<std::mutex> lock(m_mutexWorkerList);
	return std::count_if(m_workerList.cbegin(), m_workerList.cend(), [](const std::shared_ptr<Worker>& ptr) {
		// 判断活跃 worker.
		return !ptr->isActive();
		});
}

void ljPool::CppPool::enqueue(const std::function<void(void)>& func)
{
	// 加队列锁.
	std::unique_lock<std::mutex> lock(m_queueSource.mutexQueue);
	// 队列为满的时候要等待.
	while (m_queueSource.taskQueue.size() == m_queueSource.maxQueueSize) {
		m_queueSource.cvQueueFull.wait(lock);
	}
	// 添加任务到任务队列.
	m_queueSource.taskQueue.push(func);
	// 唤醒一个阻塞在空队列的 worker 线程.
	m_queueSource.cvQueueEmpty.notify_one();
}

void ljPool::CppPool::stopWorkers()
{
	std::lock_guard<std::mutex> lock(m_mutexWorkerList);
	// 等待 worker 线程 join.
	auto iter = m_workerList.begin();
	while (iter != m_workerList.end()) {
		(*iter)->stop();
		// 删除后迭代器自动指向后一个元素位置.
		iter = m_workerList.erase(iter);
	}
}

void ljPool::CppPool::recycle()
{
	// 跟线程池保持同一个状态.
	while (m_running)
	{
		// std::cout << "checking... woker: " << m_workerList.size() << std::endl;
		// 每秒检测一下是否有需要回收的 worker.
		std::this_thread::sleep_for(std::chrono::seconds(m_mgrLoopDuration));
		// 开始检测是否有需要回收的线程.
		std::lock_guard<std::mutex> lock(m_mutexWorkerList);
		auto iter = m_workerList.cbegin();
		while (iter != m_workerList.cend()) {
			if (!(*iter)->isRunning()) {
				iter = m_workerList.erase(iter);
				// std::cout << "recycle a worker." << std::endl;
			}
			else {
				++iter;
			}
		}
	}
}
