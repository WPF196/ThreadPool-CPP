#include "TaskQueue.h"

TaskQueue::TaskQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
}

TaskQueue::~TaskQueue()
{
	pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::addTask(Task task)
{
	pthread_mutex_lock(&m_mutex);
	m_queue.push(task);
	pthread_mutex_unlock(&m_mutex);
}

void TaskQueue::addTask(callback f, void* arg)
{
	pthread_mutex_lock(&m_mutex);
	m_queue.push(Task(f, arg));
	pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::getTask()
{
	pthread_mutex_lock(&m_mutex);
	Task t;
	if (m_queue.size() > 0) {
		t = m_queue.front();
		m_queue.pop();
		pthread_mutex_unlock(&m_mutex);
	}
	return t;
}

inline int TaskQueue::getSize()
{
	return m_queue.size();
}
