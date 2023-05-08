#pragma once

#include <queue>
#include <pthread.h>

using namespace std;
using callback = void(*)(void*);	// 函数指针

// 任务结构体
template <typename T>
struct Task{
	Task<T>() {
		this->function = nullptr;
		this->arg = nullptr;
	}
	Task<T>(callback f, void* arg) {
		this->function = f;
		this->arg = (T*)arg;
	}
	callback function;	// 函数指针
	T* arg;			// 参数指针
};

// 任务队列
template <typename T>
class TaskQueue {
public:
	TaskQueue();
	~TaskQueue();

	// 添加任务
	void addTask(Task<T> task);
	void addTask(callback f, void* arg);
	// 取出任务
	Task<T> getTask();
	// 获取队列任务数
	size_t getSize(); 

private:
	pthread_mutex_t m_mutex;	// 互斥锁
	queue<Task<T>> m_queue;		// 任务队列
};