#include <iostream>
#include <cstdlib>
#include <vector>   // std::vector
#include <string>   // std::string
#include <future>   // std::future
#include <thread>   // std::this_thread::sleep_for
#include <chrono>   // std::chrono::seconds
#include <windows.h>

#include "ThreadPool.h"
using namespace std;

void func(int i)
{
	double temp = 0;
	for (int j = 0; j < 10000; j++)
		for (int k = 0; k < 1000; k++)
			temp = temp + 1;
	cout << "Task " << i << " stop...\n";
}

int main()
{
	//// 创建一个能够并发执行四个线程的线程池
	//ThreadPool pool(4);
	//// 创建并发执行线程的结果列表
	//std::vector< std::future<std::string> > results;

	//// 启动八个需要执行的线程任务
	//for (int i = 0; i < 8; ++i) {
	//	// 将并发执行任务的返回值添加到结果列表中
	//	results.emplace_back(
	//		// 将下面的打印任务添加到线程池中并发执行
	//		pool.addTask([i] {
	//		std::cout << "hello " << i << std::endl;
	//		// 上一行输出后, 该线程会等待1秒钟
	//		std::this_thread::sleep_for(std::chrono::seconds(1));
	//		// 然后再继续输出并返回执行情况
	//		std::cout << "world " << i << std::endl;
	//		return std::string("---thread ") + std::to_string(i) + std::string(" finished.---");
	//	})
	//	);
	//}

	//// 输出线程任务的结果
	//for (auto && result : results)
	//	std::cout << result.get() << ' ';

	//std::cout << std::endl;
	//system("pause");

	thread *t = new thread[10000];
	DWORD start = GetTickCount();
	for (int i = 0; i < 10000; i++)
	{
		t[i] = thread(func, i);
	}
	for (int i = 0; i < 10000; i++)
	{
		t[i].join();
	}
	DWORD end = GetTickCount();
	cout << "The run time is:" << (end - start) / 1000.0 << " s" << endl;

	//DWORD start = GetTickCount();
	//{
	//	ThreadPool pool(8);
	//	for (int i = 0; i < 10000; i++)
	//	{
	//		pool.addTask(func, i);
	//	}
	//}
	//DWORD end = GetTickCount();
	//cout << "The run time is:" << (end - start)/1000.0 << " s" << endl;
	//system("pause");

	return 0;
}