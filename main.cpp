#include <iostream>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <windows.h>

#include "ThreadPool.h"

int main()
{
	ThreadPool pool(4);  // 创建一个能够并发执行四个线程的线程池
	std::vector<std::future<int>> results;  // 创建并发执行线程的结果列表

	// 添加100个需要执行的线程任务
	for (int i = 0; i < 100; i++)
	{
		results.emplace_back(pool.addTask([](int x, int y) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			return x + y;
			}, i, i)
		);
	}

	// 输出线程任务的结果
	for (auto&& result : results)
	{
		std::cout << result.get() << std::endl;
	}

	system("pause");
	return 0;
}