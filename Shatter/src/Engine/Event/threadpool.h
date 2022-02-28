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
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_macro.h"


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

    void addTasks(const std::array<std::function<void()>,4>& _array);

    void setThreadCount(uint32_t count = std::thread::hardware_concurrency())
    {
        m_thread_count = count;
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
    uint32_t m_index = 0;
    std::vector<Thread*> threads;
    uint32_t m_thread_count;
public:
    explicit ThreadPool(uint32_t count);
    ThreadPool();
    ~ThreadPool();
    DefineUnCopy(ThreadPool);
private:
    static ThreadPool *m_pool;
};

#define SingleThreadPool ThreadPool::pool()

#endif //SHATTER_ENGINE_THREADPOOL_H
