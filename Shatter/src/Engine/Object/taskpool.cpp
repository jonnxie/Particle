//
// Created by jonnxie on 2021/9/23.
//

#include "taskpool.h"
#include "threadpool.h"

std::unordered_map<Task_id,std::function<void()>> TaskPool::m_tasks = std::unordered_map<Task_id,std::function<void()>>();
std::unordered_map<Task_id,std::function<void(float)>> TaskPool::m_update_tasks = std::unordered_map<Task_id,std::function<void(float)>>();
std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> TaskPool::m_barrier_require_tasks = std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>>();
std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> TaskPool::m_barrier_release_tasks = std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>>();
std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> TaskPool::m_compute_barrier_require_tasks = std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>>();
std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> TaskPool::m_compute_barrier_release_tasks = std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>>();


static std::mutex m_lock;

static std::mutex m_update_lock;

static std::mutex m_barrier_lock;

static std::mutex m_compute_barrier_lock;

void TaskPool::pushTask(const Task_id& _id,std::function<void()> _task)
{
    std::lock_guard<std::mutex> guard_lock(m_lock);
    if(m_tasks.count(_id) != 0)
    {
        WARNING(task is already exist);
    }else{
        m_tasks[_id] = std::move(_task);
    }
}

void TaskPool::pushUpdateTask(const Task_id &_id, std::function<void(float)> _task) {
    std::lock_guard<std::mutex> guard_lock(m_update_lock);
    if(m_update_tasks.count(_id) != 0)
    {
        WARNING(task is already exist);
    }else {
        m_update_tasks[_id] = std::move(_task);
    }
}

void TaskPool::pushBarrierTask(const Task_id& _id,std::function<void(VkCommandBuffer)> _task,InsertTaskState _state) {
    std::lock_guard<std::mutex> guard_lock(m_barrier_lock);

    if(_state == InsertTaskState::Pre)
    {
        if(m_barrier_require_tasks.count(_id) != 0)
        {
            WARNING(task is already exist);
        }else{
            m_barrier_require_tasks[_id] = std::move(_task);
        }
    }else{
        if(m_barrier_release_tasks.count(_id) != 0)
        {
            WARNING(task is already exist);
        }else{
            m_barrier_release_tasks[_id] = std::move(_task);
        }
    }
}

void TaskPool::pushComputeBarrierTask(const Task_id& _id,std::function<void(VkCommandBuffer)> _task,InsertTaskState _state)
{
    std::lock_guard<std::mutex> guard_lock(m_compute_barrier_lock);

    if(_state == InsertTaskState::Pre)
    {
        if(m_compute_barrier_require_tasks.count(_id) != 0)
        {
            WARNING(task is already exist);
        }else{
            m_compute_barrier_require_tasks[_id] = std::move(_task);
        }
    }else{
        if(m_compute_barrier_release_tasks.count(_id) != 0)
        {
            WARNING(task is already exist);
        }else{
            m_compute_barrier_release_tasks[_id] = std::move(_task);
        }
    }
}

void TaskPool::popUpdateTask(const Task_id& _id)
{
    std::lock_guard<std::mutex> guard_lock(m_update_lock);
    if(m_update_tasks.count(_id) == 0)
    {
        WARNING(task is not exist);
    }else {
        m_update_tasks.erase(_id);
    }
}

void TaskPool::execute()
{
    for(auto& [id,task] : m_tasks)
    {
        task();
    }
    m_tasks.clear();
}

void TaskPool::executeMultiple() {
    for(auto& [id,task] : m_tasks)
    {
        ThreadPool::pool()->addTask(task);
    }
    ThreadPool::pool()->wait();
    m_tasks.clear();
}

void TaskPool::update(float _abs_time)
{
    for(auto& [id,task] : m_update_tasks)
    {
        task(_abs_time);
    }
}

void TaskPool::updateMultiple(float _abs_time)
{
    for(auto& [id,task] : m_update_tasks)
    {
        std::function<void(float)> func = task;
        ThreadPool::pool()->addTask([=]{func(_abs_time);});
    }
    ThreadPool::pool()->wait();
}

void TaskPool::barrierRequire(VkCommandBuffer _cb) {
    for(auto& [id,task] : m_barrier_require_tasks)
    {
        task(_cb);
    }
}

void TaskPool::barrierRequireMultiple(VkCommandBuffer _cb) {
    for(auto& [id,task] : m_barrier_require_tasks)
    {
        std::function<void(VkCommandBuffer)> func = task;
        ThreadPool::pool()->addTask([=]{func(_cb);});
    }
    ThreadPool::pool()->wait();
}

void TaskPool::barrierRelease(VkCommandBuffer _cb)
{
    for(auto& [id,task] : m_barrier_release_tasks)
    {
        task(_cb);
    }
}

void TaskPool::barrierReleaseMultiple(VkCommandBuffer _cb)
{
    for(auto& [id,task] : m_barrier_release_tasks)
    {
        std::function<void(VkCommandBuffer)> func = task;
        ThreadPool::pool()->addTask([=]{func(_cb);});
    }
    ThreadPool::pool()->wait();
}

void TaskPool::computeBarrierRequire(VkCommandBuffer _cb)
{
    for(auto& [id,task] : m_compute_barrier_require_tasks)
    {
        task(_cb);
    }
}

void TaskPool::computeBarrierRequireMultiple(VkCommandBuffer _cb)
{
    for(auto& [id,task] : m_compute_barrier_require_tasks)
    {
        std::function<void(VkCommandBuffer)> func = task;
        ThreadPool::pool()->addTask([=]{func(_cb);});
    }
    ThreadPool::pool()->wait();
}

void TaskPool::computeBarrierRelease(VkCommandBuffer _cb)
{
    for(auto& [id,task] : m_compute_barrier_release_tasks)
    {
        task(_cb);
    }
}

void TaskPool::computeBarrierReleaseMultiple(VkCommandBuffer _cb)
{
    for(auto& [id,task] : m_compute_barrier_release_tasks)
    {
        std::function<void(VkCommandBuffer)> func = task;
        ThreadPool::pool()->addTask([=]{func(_cb);});
    }
    ThreadPool::pool()->wait();
}
