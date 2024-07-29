#pragma once

#include "TaskQueue.h"
#include <memory>
#include <future>
#include <atomic>
#include <list>

namespace ljPool {

	class Worker;

	class Manager;

	class CppPool {

	private:

		// 标志线程池是否在运行.
		std::atomic<bool> m_running = false;

		// 最大 worker 数.
		int m_maxNum;

		// 核心 worker 数.
		int m_coreNum;

		// worker 队列.
		std::list<std::shared_ptr<Worker>> m_workerList;

		// worker 队列互斥量.
		std::mutex m_mutexWorkerList;

		// 任务队列所需资源(最大任务数、队列、队列锁、空条件变量、满条件变量)
		TaskQueueSource m_queueSource;

		std::thread m_manager;

		const int m_mgrLoopDuration = 1;
		
	public:

		CppPool(int maxNum = 50, int minNum = 5, size_t maxQueueSize = 32);

		virtual ~CppPool();

		template<typename R, typename ...Args>
		auto submit(const std::function<R(Args...)>&, Args&& ...) -> std::future<R>;

		// 线程池开始执行.
		void run();

		// 结束线程池.
		void shutdown();

		// 获取当前 worker 进程数.
		size_t getWorkerNum();

		// 获取当前处在运行态的 woker 数.
		size_t getRunningWorkerNum();

		// 获取活跃进程数.
		size_t getActiveWorkerNum();

		// 获取非活跃进程数.
		size_t getInactiveWorkerNum();

	private:

		// 任务入队,自动给队列加锁.
		void enqueue(const std::function<void(void)>&);

		// 初始化 worker 线程.
		int createWorker(int);

		// 停止所有的 worker 线程.
		void stopWorkers();

		// 检测回收已退出的worker.
		void recycle();
	};

	template<typename R, typename ...Args>
	inline auto CppPool::submit(const std::function<R(Args...)>& func, Args&& ...args) -> std::future<R>
	{
		// 队列已满
		if (m_queueSource.taskQueue.size() == m_queueSource.maxQueueSize) {
			// 线程数未超过最大线程数，需要新建线程.达到最大线程则阻塞等待.
			createWorker(1);
		}
		// 用异步任务封装调用对象.这里需要堆内存.否则函数执行后会导致对象失效.
		auto pTask = std::make_shared<std::packaged_task<R(void)>>(std::bind(func, std::forward<Args>(args)...));
		// 通过类型擦除，将绑定实参的函数存储起来.
		std::function<void(void)> f = [pTask](void)-> void {
				(*pTask)();
			};
		// 加入任务队列.
		enqueue(f);
		return pTask->get_future();
	}
}