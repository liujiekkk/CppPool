#include <iostream>
#include "CppPool.h"
#include <math.h>

std::string test(int i) {
	// srand(time(0));
	// 生成随机数
	// int millisec = rand() % 10 + 1;
	int millisec = 0;
	// 随机停留一些时间 0 - 1000ms.
	std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
	return std::to_string(i);
}


int main() {

	ljPool::CppPool pool;
	pool.run();
	pool.run();
	pool.run();
	// 执行次数.
	int num = 200;
	// 结果
	std::vector<std::future<std::string>> results(num);
	for (int i = 0; i < num; ++i) {
		results[i] = pool.submit(std::function<std::string(int)>(test), std::forward<int>(i));
	}
	// 打印结果.
	for (size_t i = 0; i < num; ++i) {
		std::cout << "test" << results[i].get() << std::endl;
	}
	std::cout << "Pool has worker num: " << pool.getWorkerNum() << std::endl;
	system("pause");
	return 0;
}
