#include <iostream>
#include "CppPool.h"

std::string test(int i) {
	// srand(time(0));
	// 生成随机数
	// int millisec = rand() % 10 + 1;
	int millisec = 0;
	// 随机停留一些时间 0 - 1000ms.
	std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
	return std::to_string(i);
}

std::string funcExp(int i) {
	if (i < 0) {
		throw std::runtime_error("this is a test for exception!!!");
	}
	return std::to_string(i);
}

void test_task() {

	ljPool::CppPool pool(20);
	pool.run();
	// 执行次数.
	int num = 10000;
	// 结果
	std::vector<std::future<std::string>> results(num);
	for (int i = 0; i < num; ++i) {
		results[i] = pool.submit(std::function<std::string(int)>(test), std::forward<int>(i));
	}
	
	// 打印结果.
	for (size_t i = 0; i < num; ++i) {
		std::cout << "test" << results[i].get() << std::endl;
	}
	std::cout << "Pool total worker: " << pool.getWorkerNum() << " running worker:" << pool.getRunningWorkerNum() << std::endl;
	system("pause");
}

void test_exception()
{
	ljPool::CppPool pool;
	pool.run();
	auto result = pool.submit(std::function<std::string(int)>(funcExp), -1);
	auto result2 = pool.submit(std::function<std::string(int)>(funcExp), 8);
	try {
		std::cout << "result:" << result.get() << std::endl;
	}
	catch (std::runtime_error& e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
	std::cout << "result2:" << result2.get() << std::endl;
}

void test_multi_pools()
{
	ljPool::CppPool pool_one(10, 1);
	pool_one.run();

	ljPool::CppPool pool_two(10, 2);
	pool_two.run();

	// 执行次数.
	int num = 10;
	// 结果
	std::vector<std::future<std::string>> resultsOne(num);
	std::vector<std::future<std::string>> resultsTwo(num);
	for (int i = 0; i < num; ++i) {
		resultsOne[i] = pool_one.submit(std::function<std::string(int)>(test), std::forward<int>(i));
		resultsTwo[i] = pool_two.submit(std::function<std::string(int)>(test), i + 100);
	}

	// 打印结果.
	for (size_t i = 0; i < num; ++i) {
		std::cout << "Result one:" << resultsOne[i].get() << std::endl;
		std::cout << "Result two:" << resultsTwo[i].get() << std::endl;
	}
	std::cout << "Pool one total worker: " << pool_one.getWorkerNum() << " running worker:" << pool_one.getRunningWorkerNum() << std::endl;
	std::cout << "Pool two total worker: " << pool_two.getWorkerNum() << " running worker:" << pool_two.getRunningWorkerNum() << std::endl;
	system("pause");
}

int main() {

	// 异常测试
	// test_exception();
	// 多任务 worker 自动回收测试.
	// test_task();
	// 多线程池测试.
	test_multi_pools();
	return 0;
}
