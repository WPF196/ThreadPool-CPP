#include "TaskQueue.h"

template <typename T>
TaskQueue<T>::TaskQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
}

template <typename T>
TaskQueue<T>::~TaskQueue()
{
	pthread_mutex_destroy(&m_mutex);
}

template <typename T>
void TaskQueue<T>::addTask(Task<T> task)
{
	pthread_mutex_lock(&m_mutex);
	m_queue.push(task);
	pthread_mutex_unlock(&m_mutex);
}

template <typename T>
void TaskQueue<T>::addTask(callback f, void* arg)
{
	pthread_mutex_lock(&m_mutex);
	m_queue.push(Task<T>(f, arg));
	pthread_mutex_unlock(&m_mutex);
}

template <typename T>
Task<T> TaskQueue<T>::getTask()
{
	pthread_mutex_lock(&m_mutex);
	Task<T> t;
	if (m_queue.size() > 0) {
		t = m_queue.front();
		m_queue.pop();
		pthread_mutex_unlock(&m_mutex);
	}
	return t;
}

template <typename T>

size_t TaskQueue<T>::getSize()
{
	return m_queue.size();
}
