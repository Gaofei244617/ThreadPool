#include <iostream>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <windows.h>

#include "ThreadPool.h"

int main()
{
	ThreadPool pool(4);  // ����һ���ܹ�����ִ���ĸ��̵߳��̳߳�
	std::vector<std::future<int>> results;  // ��������ִ���̵߳Ľ���б�

	// ���100����Ҫִ�е��߳�����
	for (int i = 0; i < 100; i++)
	{
		results.emplace_back(pool.addTask([](int x, int y) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			return x + y;
			}, i, i)
		);
	}

	// ����߳�����Ľ��
	for (auto&& result : results)
	{
		std::cout << result.get() << std::endl;
	}

	system("pause");
	return 0;
}