#include <iostream>
#include <unistd.h>
#include "ThreadPool.h"
#include "ThreadPool.cpp"

using namespace std;

void taskFunc(void* arg){
    int num = *(int*)arg;
    printf("thread %ld is working, number = %d\n",
        pthread_self(), num);
    sleep(1);
}

int main()
{
    // 创建线程池
    ThreadPool<int> pool(3, 10);
    for (int i = 0; i < 100; ++i)
    {
        int* num = new int(i + 100);
        pool.addTask(Task<int>(taskFunc, num));
    }

    sleep(20);      // 等待工作线程完成相关任务

    return 0;
}
