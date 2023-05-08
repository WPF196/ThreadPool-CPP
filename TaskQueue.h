#pragma once

#include <queue>
#include <pthread.h>

using namespace std;
using callback = void(*)(void*);	// ����ָ��

// ����ṹ��
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
	callback function;	// ����ָ��
	T* arg;			// ����ָ��
};

// �������
template <typename T>
class TaskQueue {
public:
	TaskQueue();
	~TaskQueue();

	// �������
	void addTask(Task<T> task);
	void addTask(callback f, void* arg);
	// ȡ������
	Task<T> getTask();
	// ��ȡ����������
	size_t getSize(); 

private:
	pthread_mutex_t m_mutex;	// ������
	queue<Task<T>> m_queue;		// �������
};