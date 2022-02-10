//
// Created by AnWell on 2021/8/16.
//

#ifndef SHATTER_ENGINE_THREADPOOL_H
#define SHATTER_ENGINE_THREADPOOL_H

#include <thread>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <future>
#include <type_traits>
#include <condition_variable>
#include "../Shatter_Item/shatter_item.h"


class Thread
{
public:
    Thread();
    ~Thread();
public:
    void wait();
    void work();
    void addTask(std::function<void()> _task);
public:
    std::thread m_work;
    bool m_destroyed = false;
    std::mutex m_task_mutex;
    std::condition_variable m_condition;
    std::queue<std::function<void()>> m_tasks;
};

class ThreadPool
{
public:
    void addTask(const std::function<void()>& _task);

    void setThreadCount(uint32_t count = std::thread::hardware_concurrency())
    {
        threads.clear();
        for(auto i = 0; i < count; i++)
        {
            threads.emplace_back(new Thread);
        }
    }

    void wait()
    {
        for(auto &thread : threads)
        {
            thread->wait();
        }
    }

    static ThreadPool* pool();

    static void release();

public:
    int m_index = 0;
    std::vector<Thread*> threads;
private:
    ThreadPool();
    ~ThreadPool();
    static ThreadPool *m_pool;
};



#endif //SHATTER_ENGINE_THREADPOOL_H
