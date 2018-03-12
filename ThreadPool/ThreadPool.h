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
	//构造线程队列
	ThreadPool(size_t threads);

	//向任务队列添加任务
	template<typename F, typename ...Args>
	auto addTask(F&& f, Args&& ...args)->std::future<decltype(f(args...))>;

	//销毁线程
	~ThreadPool();

private:
	//线程池容器
	std::vector<std::thread> pool;
	//任务队列
	std::queue<std::function<void()>> tasks;
	//互斥量
	std::mutex mtx;
	//条件变量
	std::condition_variable condition;
	//线程池状态
	bool stop;

	//任务调度函数
	void schedual();
};

//////////////////////////////////////////////////////////////////////////////

inline ThreadPool::ThreadPool(size_t threads) : stop(false)
{
	//初始化线程池容器，并启动threads个线程
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

			//等待至线程池结束或任务队列中存在任务
			this->condition.wait(lock, [this]() {return this->stop || !tasks.empty(); });

			// 如果当前线程池已经结束且等待任务队列为空, 则应该直接返回
			if (this->stop && this->tasks.empty())
				return;

			//从任务队列中取出一个任务
			task = std::move(tasks.front());
			tasks.pop();
		}
		task(); //执行任务
	}
}

inline ThreadPool::~ThreadPool()
{
	//临界区
	{
		//互斥锁
		std::unique_lock<std::mutex> lock(this->mtx);
		//设置线程池状态
		this->stop = true;
	}
	//通知所有线程
	this->condition.notify_all();

	// 阻塞线程池所在线程，等待执行完任务队列的所有任务
	for (std::thread& t : pool)
		t.join();
}

template<typename F, typename ...Args>
auto ThreadPool::addTask(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
{
	//推导任务返回类型
	using returnType = decltype(f(args...));

	//判断线程池是否停止
	if (this->stop)
		throw std::runtime_error("ThreadPool has been stopped!");

	//封装任务以实现异步调用，因为packaged_task禁止拷贝，考虑到需要用lambda表达式进一步进行封装，因此使用智能指针
	auto task = std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	//异步获取任务结果
	std::future<returnType> res = task->get_future();

	//临界区
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		//向任务队列添加任务，类型为funtion<void()>
		this->tasks.emplace([task]() { (*task)(); });
	}
	//通知一个线程
	condition.notify_one();
	return res;
}

#endif