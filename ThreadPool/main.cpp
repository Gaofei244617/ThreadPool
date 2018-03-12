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
	//// ����һ���ܹ�����ִ���ĸ��̵߳��̳߳�
	//ThreadPool pool(4);
	//// ��������ִ���̵߳Ľ���б�
	//std::vector< std::future<std::string> > results;

	//// �����˸���Ҫִ�е��߳�����
	//for (int i = 0; i < 8; ++i) {
	//	// ������ִ������ķ���ֵ��ӵ�����б���
	//	results.emplace_back(
	//		// ������Ĵ�ӡ������ӵ��̳߳��в���ִ��
	//		pool.addTask([i] {
	//		std::cout << "hello " << i << std::endl;
	//		// ��һ�������, ���̻߳�ȴ�1����
	//		std::this_thread::sleep_for(std::chrono::seconds(1));
	//		// Ȼ���ټ������������ִ�����
	//		std::cout << "world " << i << std::endl;
	//		return std::string("---thread ") + std::to_string(i) + std::string(" finished.---");
	//	})
	//	);
	//}

	//// ����߳�����Ľ��
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