#include "ThreadPool.hpp"
#define NUM 10
int Add(int x , int y)
{
    return x + y;
}
int main()
{
    ThreadPool<int>* tp = new ThreadPool<int>(NUM);  //创建一个线程池，有 NUM 个线程
    tp->InitThreadPool();                    //初始化线程池
    int count = 1;
    while(1){
        ThreadPool<int>::Task t(count,count+1,Add);   //创建任务
        count++;
        tp->AddTask(t);          //把该任务添加到线程池中
        sleep(1);
    }
    return 0;
}
