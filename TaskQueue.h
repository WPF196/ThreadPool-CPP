#pragma once

#include <queue>
#include <pthread.h>

using namespace std;
using callback = void(*)(void*);	// ����ָ��

// ����ṹ��
struct Task {
	Task() {
		this->function = nullptr;
		this->arg = nullptr;
	}
	Task(callback f, void* arg) {
		this->function = f;
		this->arg = arg;
	}
	callback function;	// ����ָ��
	void* arg;			// ����ָ��
};

// �������
class TaskQueue {
public:
	TaskQueue();
	~TaskQueue();

	// �������
	void addTask(Task task);
	void addTask(callback f, void* arg);
	// ȡ������
	Task getTask();
	// ��ȡ����������
	inline int getSize();

private:
	pthread_mutex_t m_mutex;	// ������
	queue<Task> m_queue;		// �������
};