## 简介
一个高性能、轻量级的C++线程池，以面向对象方式开发实现，旨在帮助开发者轻松管理多线程任务。它支持异步任务调度，提供了一个简单易用的接口，适用于各种并发场景。

## 功能
- 动态调整线程数量。自定义核心线程数量，线程池的线程上限设置。
- 自动管理线程生命周期，线程池运行结束，通过RAII机制自动回收线程资源。
- 异常安全，对任务处理有异常的，通过 future.get() 可以捕获执行中的异常。后续任务仍可以继续正常执行。
- 跨平台兼容（Linux, macOS, Windows），不依赖系统的特殊实现，通过c++11 标准库实现。

## 快速开始
### 方式一:
为了使用这个线程池库，你需要将其克隆到你的项目中，并包含相应的头文件。
`#include "CppPool.h"`
### 方式二：
可以通过cmake 编译生成的静态库文件，对线程池进行引入。
通过生成的静态库文件CppPool.lib（windows下的静态文件，linux 下为 libcpppool.s），并且在编译你的源代码时候，通过 -L指定 CppTool 库的路径。

示例代码：
```cpp

#include <iostream>
#include "CppPool.h"
ljPool::CppPool pool;
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

```
