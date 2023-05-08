#include "ThreadPool.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>

template <typename T>
ThreadPool<T>::ThreadPool(int minNum, int maxNum)
{
    // 实例化任务队列
    m_taskQ = new TaskQueue<T>;
    do {
        // 初始化普通变量
        m_minNum = minNum;
        m_maxNum = maxNum;
        m_busyNum = 0;
        m_aliveNum = minNum;
        m_exitNum = 0;
        m_shutdown = false;

        // 实例化工作线程（最大数量）
        m_workers = new pthread_t[maxNum];
        if (m_workers == nullptr) {
            cout << "malloc thread_t[] 失败...." << endl;
            break;
        }
        memset(m_workers, 0, sizeof(pthread_t) * maxNum);

        // 初始化互斥量和条件变量
        if (pthread_mutex_init(&m_lock, NULL) != 0
            || pthread_cond_init(&m_isEmpty, NULL) != 0) {
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        // 创建管理者线程和工作线程
        pthread_create(&m_manager, NULL, manage, this);
        for (int i = 0; i < minNum; ++i) {
            pthread_create(&m_workers[i], NULL, work, this);
            cout << "创建子线程, ID: " << to_string(m_workers[i]) << endl;
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
   
    // 添加任务，不需要加锁，任务队列中有锁
    m_taskQ->addTask(task);
    // 唤醒工作的线程
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
    // 每个线程将不断循环工作
    while (true) {
        pthread_mutex_lock(&pool->m_lock);

        // 如果任务队列空了 并且 线程池未关闭，则阻塞等待
        while (pool->m_taskQ->getSize() == 0 && !pool->m_shutdown) {
            cout << "thread " << to_string(pthread_self()) << " waiting..." << endl;
            // 将一直阻塞线程于此
            pthread_cond_wait(&pool->m_isEmpty, &pool->m_lock);
            
            // 解除阻塞（唤醒）后，判断是否需要销毁线程
            if (pool->m_exitNum > 0) {
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum) {
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);    //线程自杀后要开锁
                    pool->threadExit();
                }
            }
        }

        // 判断线程池是否关闭
        if (pool->m_shutdown) {
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        // 取出一个任务
        Task<T> task = pool->m_taskQ->getTask();
        pthread_mutex_unlock(&pool->m_lock);    // 共享变量操作完毕，解锁
        // 执行任务
        cout << "thread " << to_string(pthread_self()) << " start working..." << endl;
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;

        // 任务结束处理
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
    // 线程池未关闭，则管理者将一直循环工作
    while (!pool->m_shutdown) {
        sleep(5);       // 每个5s检测一次
        
        // 取出相关变量
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = (int)pool->m_taskQ->getSize();
        int busyNum = pool->m_busyNum;
        int aliveNum = pool->m_aliveNum;
        pthread_mutex_unlock(&pool->m_lock);

        // 需要创建线程
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

        // 需要删除线程
        if (busyNum * 2 < aliveNum && aliveNum > pool->m_minNum) {
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;       //需要删除的线程数
            pthread_mutex_unlock(&pool->m_lock);

            // 唤醒阻塞的线程，唤醒两个阻塞的线程去自杀
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
