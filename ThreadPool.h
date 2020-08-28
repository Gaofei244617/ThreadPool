#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <utility>

class ThreadPool
{
public:
	ThreadPool(size_t thread_num);             // �����̶߳���

	template<typename F, typename ...Args>
	auto addTask(F&& f, Args&& ...args)->std::future<decltype(f(args...))>;  // ����������������

	~ThreadPool();

private:
	void schedual();                         // �������

private:
	bool stop;
	std::mutex mtx;
	std::condition_variable condition;
	std::vector<std::thread> pool;            // �̳߳�
	std::queue<std::function<void()>> tasks;  // �������
};

/****************************************************************************/
inline ThreadPool::ThreadPool(size_t thread_num) : stop(false)
{
	// ��ʼ���̳߳�����, ������thread_num���߳�
	for (size_t i = 0; i < thread_num; ++i)
	{
		pool.emplace_back(&ThreadPool::schedual, this);
	}
}

template<typename F, typename ...Args>
auto ThreadPool::addTask(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
{
	using ReturnType = decltype(f(args...));    // ���񷵻�����

	if (this->stop)
	{
		throw std::runtime_error("ThreadPool has been stopped!");
	}

	// ��װ������ʵ���첽���� (packaged_task��ֹ����)
	auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<ReturnType> res = task->get_future();  // �첽��ȡ������

	// �ٽ���
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		// ����������������, ����Ϊfuntion<void()>
		this->tasks.emplace([task]() { (*task)(); });
	}

	condition.notify_one();
	return res;
}

void ThreadPool::schedual()
{
	while (true)
	{
		std::function<void()> task;

		// �ٽ���
		{
			std::unique_lock<std::mutex> lock(this->mtx);

			// �ȴ����̳߳ؽ�������������д�������
			this->condition.wait(lock, [this]() {return this->stop || !tasks.empty(); });

			// �����ǰ�̳߳��Ѿ������ҵȴ��������Ϊ��, ��Ӧ��ֱ�ӷ���
			if (this->stop && this->tasks.empty())
			{
				return;
			}

			// �����������ȡ��һ������
			task = std::move(tasks.front());
			tasks.pop();
		}
		task(); //ִ������
	}
}

inline ThreadPool::~ThreadPool()
{
	// �ٽ���
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		this->stop = true;
	}

	this->condition.notify_all();
	// �����̳߳������߳�, �ȴ�ִ����������е���������
	for (std::thread& t : pool)
	{
		t.join();
	}
}

#endif