#pragma once

#include <queue>
#include <pthread.h>

using namespace std;
using callback = void(*)(void*);	// 函数指针

// 任务结构体
struct Task {
	Task() {
		this->function = nullptr;
		this->arg = nullptr;
	}
	Task(callback f, void* arg) {
		this->function = f;
		this->arg = arg;
	}
	callback function;	// 函数指针
	void* arg;			// 参数指针
};

// 任务队列
class TaskQueue {
public:
	TaskQueue();
	~TaskQueue();

	// 添加任务
	void addTask(Task task);
	void addTask(callback f, void* arg);
	// 取出任务
	Task getTask();
	// 获取队列任务数
	inline int getSize();

private:
	pthread_mutex_t m_mutex;	// 互斥锁
	queue<Task> m_queue;		// 任务队列
};