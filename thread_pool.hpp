/**
 * 描述：线程池类
 * 作者：guchenfeng
 * 日期：2022/12/6
 * 
*/
#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <iostream>
#include <exception>
#include <pthread.h>
#include <list>
#include "locker.hpp"

//线程池类
//模板参数T：任务类
template<typename T>
class ThreadPool {
public:
    /*methods: 构造函数
    args: thread_sum -- 线程数量
            max_req -- 请求队列最多允许的等待请求数量
    */
    ThreadPool(int thread_sum = 0, int max_req = 0);

    ~ThreadPool();

    /*扩展任务*/
    bool append(T* task);

    /*工作线程，内部调用run执行具体任务*/
    static void* worker(void* obj);

    /*线程运行*/
    void run();


private:
    //线程数量
    int m_thread_num;

    //线程池数组头指针，大小为m_thread_num
    pthread_t* m_threads;

    //最大请求数
    int m_max_req;

    //线程池任务队列
    std::list<T*> m_work_queue;

    //队列互斥锁
    Locker m_locker_queue;

    //队列信号量
    Sem m_sem_queue;

    //队列条件变量
    //Condition m_cond_queue;

    //线程池停止标志
    bool m_is_stop;

};

//类定义区
/*构造函数*/
template<typename T>
ThreadPool<T>::ThreadPool(int thread_num, int max_req) :
        m_thread_num(thread_num), m_max_req(max_req),
        m_is_stop(false), m_threads(NULL) {
    //如果线程池参数不合法
    if (thread_num <= 0 || max_req <= 0) {
        std::cout << "ThreadPool: init threadpool fail : args invalid!" << std::endl;
        throw std::exception();
    }
    //分配线程池空间
    m_threads = new pthread_t[m_thread_num];
    //如果线程池空间分配失败
    if (!m_threads) {
        std::cout << "ThreadPool: init threadpool fail : no memory!" << std::endl;
        throw std::exception();
    }

    //创建线程池线程
    for (int i = 0; i < thread_num; i ++ ) {
        std::cout << "ThreadPool: creating the " << i << "th thread!" << std::endl;

        //创建线程失败
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            std::cout << "ThreadPool: creating the " << i << "th thread fail!" << std::endl;
            std::cout << "ThreadPool: delete all threads!" << std::endl;
            delete[] m_threads;
            throw std::exception();
        }

        //将线程分离
        if (pthread_detach(m_threads[i]) != 0) {
            std::cout << "ThreadPool: detach the " << i << "th thread fail!" << std::endl;
            std::cout << "ThreadPool: delete all threads!" << std::endl;
            delete[] m_threads;
            throw std::exception();
        }
    }
    std::cout << "ThreadPool: create threadspool succeeded!" << std::endl;
}

/*析构函数*/
template<typename T>
ThreadPool<T>::~ThreadPool() {
    //回收所有线程
    delete[] m_threads;
    //停止运行线程池
    m_is_stop = true;
}

/*添加任务至队列*/
template<typename T>
bool ThreadPool<T>::append(T* req) {
    //加锁，保证访问任务队列互斥
    m_locker_queue.lock();
    if (m_work_queue.size() >= m_max_req) {
        //任务队列已满
        std::cout << "ThreadPool: append fail : requests is full!" << std::endl;
        m_locker_queue.unlock();
        return false;
    }
    //添加任务
    m_work_queue.push_back(req);
    m_locker_queue.unlock();
    m_sem_queue.post();
    return true;
}

/*工作线程*/
template<typename T>
void* ThreadPool<T>::worker(void* obj) {
    ThreadPool* pool = (ThreadPool*) obj;
    pool->run();
    return pool;
}


/*工作线程执行内容*/
template<typename T>
void ThreadPool<T>::run() {
    //线程池还在运行
    while (!m_is_stop) {
        //尝试获取任务
        m_sem_queue.wait();
        m_locker_queue.lock();
        if (m_work_queue.empty()) {
            m_locker_queue.unlock();
            continue;
        }
        //获取任务
        T* req = m_work_queue.front();
        m_work_queue.pop_front();
        m_locker_queue.unlock();

        //任务为空
        if (!req) {
            continue;
        }

        //任务不为空，处理任务
        req->process();
    }
}

#endif