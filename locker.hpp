/**
 * 描述：线程锁类集合
 * 作者：guchenfeng
 * 日期：2022/12/5
 * 
*/

#ifndef LOCKER_HPP
#define LOCKER_HPP

#include <exception>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>

//互斥锁类
class Locker {

public:
    Locker() {
        //初始化互斥锁
        if (pthread_mutex_init(&m_mutex, NULL) != 0) {
            std::cout << "Locker:init mutex locker fail!" << std::endl;
            throw std::exception();
        }
    }
    ~Locker() {
        //销毁互斥锁
        pthread_mutex_destroy(&m_mutex);
    }

    /*加锁*/
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    /*解锁*/
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    /*获取互斥锁变量*/
    pthread_mutex_t* get() {
        return &m_mutex;
    }
private:
    //互斥锁
    pthread_mutex_t m_mutex;

};

//条件变量锁类
class Condition {
public:
    Condition() {
        if (pthread_cond_init(&m_cond, NULL) != 0) {
            std::cout << "Condition:init condition locker fail!" << std::endl;
            throw std::exception();
        }
    }
    ~Condition() {
        pthread_cond_destroy(&m_cond);
    }
    /*阻塞条件锁*/
    bool wait(pthread_mutex_t* t_mutex) {
        return pthread_cond_wait(&m_cond, t_mutex) == 0;
    }
    /*阻塞超期条件锁*/
    bool timeWait(pthread_mutex_t* t_mutex, struct timespec t) {
        return pthread_cond_timedwait(&m_cond, t_mutex, &t) == 0;
    }
    /*唤醒线程*/
    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }
    /*广播唤醒所有线程*/
    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }
private:
    //条件变量锁
    pthread_cond_t m_cond;
};

//信号量类
class Sem {
public:
    Sem() {
        if (sem_init(&m_sem, 0, 0) != 0) {
            std::cout << "Sem:init semaphore fail!" << std::endl;
            throw std::exception();
        }
    }
    Sem(int num) {
        if (sem_init(&m_sem, 0, num) != 0) {
            std::cout << "Sem:init semaphore with number fail!" << std::endl;
            throw std::exception();
        }
    }
    ~Sem() {
        sem_destroy(&m_sem);
    }

    /*P操作*/
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }

    /*V操作*/
    bool post() {
        return sem_post(&m_sem) == 0;
    }
private:
    //信号量
    sem_t m_sem;
};

#endif