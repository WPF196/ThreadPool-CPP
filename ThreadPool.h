#pragma once

#include <pthread.h>
#include "TaskQueue.h"
#include "TaskQueue.cpp"
using namespace std;

template <typename T>
class ThreadPool {
public:
	ThreadPool(int minNum, int maxNum);
	~ThreadPool();

	void addTask(Task<T> task);		// 添加任务
	int getBusyNum();				// 获取忙的任务数
	int getAliveNum();				// 获取存活的任务数

private:
	/*为了线程能够调用，需要声明为static，此时无法直接访问类内成员*/
	static void* work(void* arg);	// 工作线程函数
	static void* manage(void* arg);	// 管理线程函数
	void threadExit();				// 销毁工作线程（当前的线程）

private:
	TaskQueue<T>* m_taskQ;				// 任务队列
	pthread_mutex_t m_lock;			// 互斥量
	pthread_cond_t m_isEmpty;		// 条件变量，任务队列是否为空
	pthread_t m_manager;			// 管理者线程
	pthread_t* m_workers;			// 工作的线程
	int m_minNum;					// 最小线程数
	int m_maxNum;					// 最大线程数
	int m_busyNum;					// 工作中的线程数
	int m_aliveNum;					// 存活的线程数
	int m_exitNum;					// 需要删除的线程数
	bool m_shutdown;				// 线程池是否关闭

	static const int NUMBER = 2;	// 每次需要删除的线程数量
};