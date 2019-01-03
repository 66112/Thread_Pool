#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__ 

#include <iostream>
#include <unistd.h>
#include <queue>
#include <pthread.h>
using namespace std;
template<class T>
class Task             //任务对象 
{
typedef T (*func)(T, T);    //函数指针
private:
    T _x;            //操作数
    T _y;
    T _ret;      
    func _func;      //操作数要执行的函数指针
public:     
    Task(T x,T y,func f)   //初始化任务对象
        :_x(x),_y(y),_func(f)
    {}
    void Run()
    {
        _ret = _func(_x , _y);   //运行该函数
    }
    void Show()               //打印结果
    {
        cout << "thread: "<< pthread_self()<< " result:" << _ret << endl;
    }
    ~Task()
    {}
};

template<class T>
class ThreadPool       //线程池
{
public:
    typedef Task<T> Task;    //属于该线程池特有的任务
private:
    queue<Task> _qt;         //任务队列
    int _thread_nums;        //线程个数
    pthread_cond_t _cond;    //条件变量
    pthread_mutex_t _mutex;  //互斥锁
    bool _is_stop;           //线程是否退出
private:
    static void* thread_routine(void* arg)  //线程执行函数
    {
        ThreadPool* tp=(ThreadPool*)arg;  
        pthread_detach(pthread_self());     //线程分离
        while(1){
            tp->LockQueue();        //队列是临界资源，上锁
            while(tp->Empty()){     //判断是否有任务
                tp->IdleThread();   //查看线程状态
            }
            Task t = tp->GetTask(); //在任务队列中拿一个任务
            tp->UnlockQueue();      //解锁
            t.Run();                //运行任务
            t.Show();               //打印结果
        }
    }
    void NotifyAllThreads()         //唤醒所有线程
    {
        pthread_cond_broadcast(&_cond);
    }
    void NotifyOneThread()          //唤醒一个线程
    {
        pthread_cond_signal(&_cond);
    }
public:
    ThreadPool(int num)             //初始化列表
        :_thread_nums(num),_is_stop(false) 
    {}
    ~ThreadPool()
    {
        pthread_mutex_destroy(&_mutex);  //销毁锁
        pthread_cond_destroy(&_cond);    //销毁条件变量
    }
    void InitThreadPool()
    {
        pthread_cond_init(&_cond,NULL);   //初始化条件变量
        pthread_mutex_init(&_mutex,NULL); //初始化锁
        for(int i=0;i<_thread_nums;i++){
            pthread_t tid;
            pthread_create(&tid,NULL,thread_routine,(void*)this);  //创建线程
        }
    }
    void LockQueue()
    {
        pthread_mutex_lock(&_mutex);
    }
    void UnlockQueue()
    {
        pthread_mutex_unlock(&_mutex);
    }
    bool Empty()              //判断队列里是否有任务
    {
        return _qt.size()==0 ? true:false;
    }
    void IdleThread()
    {
        if(_is_stop){         //线程如果退出了
            UnlockQueue();
            _thread_nums--;   //线程数减一
            pthread_exit((void*)0); //线程退出
            return;
        }
        pthread_cond_wait(&_cond,&_mutex);  //线程释放锁并挂起
    }
    void AddTask(Task& t)     //添加一个任务
    {
        LockQueue();
        if(_is_stop){         //查看线程是否退出
            UnlockQueue();
            return;
        }
        _qt.push(t);          //push到任务队列中
        NotifyOneThread();    //唤醒一个线程
        UnlockQueue();
    }
    Task GetTask()            //拿出一个任务
    {
        Task ret = _qt.front();
        _qt.pop();
        return ret;
    }
    void Stop()
    {
        LockQueue();
        _is_stop=true;        //线程状态置为退出态
        UnlockQueue();
        while(_thread_nums > 0){  
            NotifyAllThreads();   //唤醒所有线程
        }
    }
};

#endif 
