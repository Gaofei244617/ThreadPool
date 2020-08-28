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
	ThreadPool(size_t thread_num);             // 构造线程队列

	template<typename F, typename ...Args>
	auto addTask(F&& f, Args&& ...args)->std::future<decltype(f(args...))>;  // 向任务队列添加任务

	~ThreadPool();

private:
	void schedual();                         // 任务调度

private:
	bool stop;
	std::mutex mtx;
	std::condition_variable condition;
	std::vector<std::thread> pool;            // 线程池
	std::queue<std::function<void()>> tasks;  // 任务队列
};

/****************************************************************************/
inline ThreadPool::ThreadPool(size_t thread_num) : stop(false)
{
	// 初始化线程池容器, 并启动thread_num个线程
	for (size_t i = 0; i < thread_num; ++i)
	{
		pool.emplace_back(&ThreadPool::schedual, this);
	}
}

template<typename F, typename ...Args>
auto ThreadPool::addTask(F&& f, Args&& ...args) -> std::future<decltype(f(args...))>
{
	using ReturnType = decltype(f(args...));    // 任务返回类型

	if (this->stop)
	{
		throw std::runtime_error("ThreadPool has been stopped!");
	}

	// 封装任务以实现异步调用 (packaged_task禁止拷贝)
	auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<ReturnType> res = task->get_future();  // 异步获取任务结果

	// 临界区
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		// 向任务队列添加任务, 类型为funtion<void()>
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

		// 临界区
		{
			std::unique_lock<std::mutex> lock(this->mtx);

			// 等待至线程池结束或任务队列中存在任务
			this->condition.wait(lock, [this]() {return this->stop || !tasks.empty(); });

			// 如果当前线程池已经结束且等待任务队列为空, 则应该直接返回
			if (this->stop && this->tasks.empty())
			{
				return;
			}

			// 从任务队列中取出一个任务
			task = std::move(tasks.front());
			tasks.pop();
		}
		task(); //执行任务
	}
}

inline ThreadPool::~ThreadPool()
{
	// 临界区
	{
		std::unique_lock<std::mutex> lock(this->mtx);
		this->stop = true;
	}

	this->condition.notify_all();
	// 阻塞线程池所在线程, 等待执行完任务队列的所有任务
	for (std::thread& t : pool)
	{
		t.join();
	}
}

#endif