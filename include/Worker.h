#pragma once
#include "TaskQueue.h"
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <chrono>
#include <list>

namespace ljPool {

	struct TaskQueueSource;

	class CppPool;

	class Worker {

	private:

		// 记录当前 worker 是否正在运行.
		std::atomic<bool> m_running;

		// 当前 worker 的线程资源.
		std::thread m_thread;

		// 任务队列.
		TaskQueue& m_queue;

		// 任务队列互斥量.
		std::mutex& m_mutexQueue;

		// 任务队列为空条件变量.
		std::condition_variable& m_cvQueueEmpty;

		// 任务队列满条件变量.
		std::condition_variable& m_cvQueueFull;

		// 进程是否正在忙碌（处理业务中）
		bool m_busy;

		// worker 的空闲超时时间.
		std::chrono::duration<long long, std::milli> m_timeout;

		// 最大 worker 数量.
		const int& m_coreWorkerNum;

		CppPool& m_pool;

	public:

		Worker(TaskQueueSource&, CppPool&, const int&, long long = 2000ll);

		~Worker();

		Worker(const Worker&) = delete;

		Worker& operator=(const Worker&) = delete;

		void start();

		void stop();

		bool isActive() const;

		bool isRunning() const;

	protected:

		void run();

	};
}