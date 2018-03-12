#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <utility>

class ThreadPool
{
public:
	//�����̶߳���
	ThreadPool(size_t threads);

	//����������������
	template<typename F, typename ...Args>
	auto addTask(F&& f, Args&& ...args)->std::future<decltype(f(args...))>;

	//�����߳�
	~ThreadPool();

private:
	//�̳߳�����
	std::vector<std::thread> pool;
	//�������
	std::queue<std::function<void()>> tasks;
	//������
	std::mutex mtx;
	//��������
	std::condition_variable condition;
	//�̳߳�״̬
	bool stop;

	//������Ⱥ���
	void schedual();
};

//////////////////////////////////////////////////////////////////////////////

inline ThreadPool::ThreadPool(size_t threads) : stop(false)
{
	//��ʼ���̳߳�������������threads���߳�
	for (size_t i = 0; i < threads; ++i)
		pool.emplace_back(&ThreadPool::schedual, this);
}

void ThreadPool::schedual()
{
	while (true)
	{
		std::function<void()> task;

		{
			std::unique_lock<std::mutex> lock(this->mtx);

			//�ȴ����̳߳ؽ�������������д�������
			this->condition.wait(lock, [this]() {return this->stop || !tasks.empty(); });

			// �����ǰ�̳߳��Ѿ������ҵȴ��������Ϊ��, ��Ӧ��ֱ�ӷ���
			if (this->stop && this->tasks.empty())
				return;

			//�����������ȡ��һ������
			task = std::move(tasks.front());
			tasks.pop();
		}
		task(); //ִ������
	}
}

inline ThreadPool::~ThreadPool()
{
	//�ٽ���
	{
		//������
		std::unique_lock<std::mutex> lock(this->mtx);
		//�����̳߳�״̬
		this->stop = true;
	}
	//֪ͨ�����߳�
	this->condition.notify_all();

	// �����̳߳������̣߳��ȴ�ִ����������е���������
	for (std::thread& t : pool)
		t.join();
}

template<typename F, typename ...Args>
auto ThreadPool::addTask(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
{
	//�Ƶ����񷵻�����
	using returnType = decltype(f(args...));

	//�ж��̳߳��Ƿ�ֹͣ
	if (this->stop)
		throw std::runtime_error("ThreadPool has been stopped!");

	//��װ������ʵ���첽���ã���Ϊpackaged_task��ֹ���������ǵ���Ҫ��lambda���ʽ��һ�����з�װ�����ʹ������ָ��
	auto task = std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	//�첽��ȡ������
	std::future<returnType> res = task->get_future();

	//�ٽ���
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		//��������������������Ϊfuntion<void()>
		this->tasks.emplace([task]() { (*task)(); });
	}
	//֪ͨһ���߳�
	condition.notify_one();
	return res;
}

#endif