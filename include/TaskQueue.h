#pragma once

#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

namespace ljPool {

	typedef std::queue<std::function<void(void)>> TaskQueue;

	typedef std::function<void(void)> Task;

	struct TaskQueueSource {

		// 队列存储的最大任务数量.
		size_t maxQueueSize;

		// 任务队列.
		std::queue<std::function<void(void)>> taskQueue;

		// 任务队列锁变量.
		std::mutex mutexQueue;

		// 任务队列满时候阻塞在此变量.
		std::condition_variable cvQueueFull;

		// 任务队列为空条件变量.
		std::condition_variable cvQueueEmpty;
	};
}