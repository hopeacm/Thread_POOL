# Thread_POOL
# 线程池原理介绍:

线程池是一种多线程处理形式，处理过程中将任务添加到队列，然后在创建线程后自动启动这些任务。线程池线程都是后台线程。每个线程都使用默认的堆栈大小，以默认的优先级运行，并处于多线程单元中。

# 线程池的组成部分：
线程池管理器（ThreadPoolManager）:用于创建并管理线程池
工作线程（WorkThread）: 线程池中线程
任务接口（Task）:每个任务必须实现的接口，以供工作线程调度任务的执行。
任务队列:用于存放没有处理的任务。提供一种缓冲机制。

# 涉及到的c++11的特性：
std::vector
std::thread
std::mutex
std::future
std::condition_variable

本文参考：
https://blog.csdn.net/gcola007/article/details/78750220 
