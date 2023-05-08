#include "ThreadPool.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>

template <typename T>
ThreadPool<T>::ThreadPool(int minNum, int maxNum)
{
    // ʵ�����������
    m_taskQ = new TaskQueue<T>;
    do {
        // ��ʼ����ͨ����
        m_minNum = minNum;
        m_maxNum = maxNum;
        m_busyNum = 0;
        m_aliveNum = minNum;
        m_exitNum = 0;
        m_shutdown = false;

        // ʵ���������̣߳����������
        m_workers = new pthread_t[maxNum];
        if (m_workers == nullptr) {
            cout << "malloc thread_t[] ʧ��...." << endl;
            break;
        }
        memset(m_workers, 0, sizeof(pthread_t) * maxNum);

        // ��ʼ������������������
        if (pthread_mutex_init(&m_lock, NULL) != 0
            || pthread_cond_init(&m_isEmpty, NULL) != 0) {
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        // �����������̺߳͹����߳�
        pthread_create(&m_manager, NULL, manage, this);
        for (int i = 0; i < minNum; ++i) {
            pthread_create(&m_workers[i], NULL, work, this);
            cout << "�������߳�, ID: " << to_string(m_workers[i]) << endl;
        }
    } while (0);
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    m_shutdown = true;
    pthread_join(m_manager, NULL);
    
}

template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
    if (m_shutdown) return;
   
    // ������񣬲���Ҫ�������������������
    m_taskQ->addTask(task);
    // ���ѹ������߳�
    pthread_cond_signal(&m_isEmpty);
}

template <typename T>
int ThreadPool<T>::getBusyNum()
{
    int busyNum = 0;
    pthread_mutex_lock(&m_lock);
    busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}

template <typename T>
int ThreadPool<T>::getAliveNum()
{
    int aliveNum = 0;
    pthread_mutex_lock(&m_lock);
    aliveNum = m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return aliveNum;
}

template <typename T>
void* ThreadPool<T>::work(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    // ÿ���߳̽�����ѭ������
    while (true) {
        pthread_mutex_lock(&pool->m_lock);

        // ���������п��� ���� �̳߳�δ�رգ��������ȴ�
        while (pool->m_taskQ->getSize() == 0 && !pool->m_shutdown) {
            cout << "thread " << to_string(pthread_self()) << " waiting..." << endl;
            // ��һֱ�����߳��ڴ�
            pthread_cond_wait(&pool->m_isEmpty, &pool->m_lock);
            
            // ������������ѣ����ж��Ƿ���Ҫ�����߳�
            if (pool->m_exitNum > 0) {
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum) {
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);    //�߳���ɱ��Ҫ����
                    pool->threadExit();
                }
            }
        }

        // �ж��̳߳��Ƿ�ر�
        if (pool->m_shutdown) {
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        // ȡ��һ������
        Task<T> task = pool->m_taskQ->getTask();
        pthread_mutex_unlock(&pool->m_lock);    // �������������ϣ�����
        // ִ������
        cout << "thread " << to_string(pthread_self()) << " start working..." << endl;
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;

        // �����������
        cout << "thread " << to_string(pthread_self()) << " end working...";
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }
    return nullptr;
}

template <typename T>
void* ThreadPool<T>::manage(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    // �̳߳�δ�رգ�������߽�һֱѭ������
    while (!pool->m_shutdown) {
        sleep(5);       // ÿ��5s���һ��
        
        // ȡ����ر���
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = (int)pool->m_taskQ->getSize();
        int busyNum = pool->m_busyNum;
        int aliveNum = pool->m_aliveNum;
        pthread_mutex_unlock(&pool->m_lock);

        // ��Ҫ�����߳�
        if (queueSize > aliveNum && aliveNum < pool->m_maxNum) {
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER 
                && pool->m_aliveNum < pool->m_maxNum; ++i) {
                if (pool->m_workers[i] == 0) {
                    pthread_create(&pool->m_workers[i], NULL, work, pool);
                    pool->m_aliveNum++;
                    num++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        // ��Ҫɾ���߳�
        if (busyNum * 2 < aliveNum && aliveNum > pool->m_minNum) {
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;       //��Ҫɾ�����߳���
            pthread_mutex_unlock(&pool->m_lock);

            // �����������̣߳����������������߳�ȥ��ɱ
            for (int i = 0; i < NUMBER; ++i)
                pthread_cond_signal(&pool->m_isEmpty);
        }
    }

    return nullptr;
}

template <typename T>
void ThreadPool<T>::threadExit()
{
    pthread_t t_id = pthread_self();
    for (int i = 0; i < m_maxNum; ++i) {
        if (m_workers[i] == t_id) {
            cout << "threadExit() function: thread "
                << to_string(pthread_self()) << " exiting..." << endl;
            m_workers[i] = 0;
            break;
        }
    }
    pthread_exit(NULL);
}
