#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
	ThreadPool(size_t);  //线程池大小
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args) //任务管道函数
		->std::future<typename std::result_of<F(Args...)>::type>;//利用尾置限定符  std future用来获取异步任务的结果
	~ThreadPool();
private:
	// need to keep track of threads so we can join them
	std::vector< std::thread > workers; //追踪线程
	// the task queue
	std::queue< std::function<void()> > tasks; //任务队列，用于存放没有处理的任务。提供缓冲机制

	// synchronization
	std::mutex queue_mutex; //互斥锁
	std::condition_variable condition; //条件变量
	bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads): stop(false)
{
	for (size_t i = 0; i < threads; ++i)
		workers.emplace_back( //构造线程
		[this]
		{
			for (;;)
			{
				std::function<void()> task;
				{ //lock存活时间控制
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock,
						[this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}
				task();
			}
		});
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) //&& 引用限定符，参数的右值引用，  此处表示参数传入一个函数
-> std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;
	//packaged_task是对任务的一个抽象，我们可以给其传递一个函数来完成其构造。之后将任务投递给任何线程去完成，通过
	//packaged_task.get_future()方法获取的future来获取任务完成后的产出值
	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)); //构造函数
	//future为期望，get_future获取任务完成后的产出值
	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex); //保持互斥性，避免多个线程同时运行一个任务
		// don't allow enqueueing after stopping the pool
		if (stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");
		tasks.emplace([task]() { (*task)(); });  //将task投递给线程去完成，vector尾部压入
	}
	condition.notify_one(); //选择一个wait状态的线程进行唤醒，并使他获得对象上的锁来完成任务(即其他线程无法访问对象)
	return res;
}



// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all(); //通知所有wait状态的线程竞争对象的控制权，唤醒所有线程执行
	for (std::thread &worker : workers)
		worker.join(); //因为线程都开始竞争了，所以一定会执行完，join可等待线程执行完
}

#endif
