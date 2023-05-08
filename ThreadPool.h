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

	void addTask(Task<T> task);		// �������
	int getBusyNum();				// ��ȡæ��������
	int getAliveNum();				// ��ȡ����������

private:
	/*Ϊ���߳��ܹ����ã���Ҫ����Ϊstatic����ʱ�޷�ֱ�ӷ������ڳ�Ա*/
	static void* work(void* arg);	// �����̺߳���
	static void* manage(void* arg);	// �����̺߳���
	void threadExit();				// ���ٹ����̣߳���ǰ���̣߳�

private:
	TaskQueue<T>* m_taskQ;				// �������
	pthread_mutex_t m_lock;			// ������
	pthread_cond_t m_isEmpty;		// ������������������Ƿ�Ϊ��
	pthread_t m_manager;			// �������߳�
	pthread_t* m_workers;			// �������߳�
	int m_minNum;					// ��С�߳���
	int m_maxNum;					// ����߳���
	int m_busyNum;					// �����е��߳���
	int m_aliveNum;					// �����߳���
	int m_exitNum;					// ��Ҫɾ�����߳���
	bool m_shutdown;				// �̳߳��Ƿ�ر�

	static const int NUMBER = 2;	// ÿ����Ҫɾ�����߳�����
};