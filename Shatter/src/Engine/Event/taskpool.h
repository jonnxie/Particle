//
// Created by jonnxie on 2021/9/23.
//

#ifndef SHATTER_ENGINE_TASKPOOL_H
#define SHATTER_ENGINE_TASKPOOL_H

#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <map>
#include <unordered_map>
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_enum.h"

class TaskPool{
public:
    static void pushTask(const Task_id& _id,std::function<void()> _task);
    static void pushTask(std::function<void()> _task);
    static void pushUpdateTask(const Task_id& _id, std::function<void(float)> _task);
    static void pushBarrierTask(const Task_id& _id,std::function<void(VkCommandBuffer)> _task,InsertTaskState _state);
    static void pushComputeBarrierTask(const Task_id& _id,std::function<void(VkCommandBuffer)> _task,InsertTaskState _state);

    static void popUpdateTask(const Task_id& _id);

    static void execute();
    static void executeMultiple();

    static void update(float _abs_time);
    static void updateMultiple(float _abs_time);

    static void barrierRequire(VkCommandBuffer _cb);
    static void barrierRequireMultiple(VkCommandBuffer _cb);
    static void barrierRelease(VkCommandBuffer _cb);
    static void barrierReleaseMultiple(VkCommandBuffer _cb);

    static void computeBarrierRequire(VkCommandBuffer _cb);
    static void computeBarrierRequireMultiple(VkCommandBuffer _cb);
    static void computeBarrierRelease(VkCommandBuffer _cb);
    static void computeBarrierReleaseMultiple(VkCommandBuffer _cb);

    static std::unordered_map<Task_id,std::function<void()>> m_tasks;
    static std::vector<std::function<void()>> m_pure_task;
    static std::unordered_map<Task_id,std::function<void(float)>> m_update_tasks;
    static std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> m_barrier_require_tasks;
    static std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> m_barrier_release_tasks;
    static std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> m_compute_barrier_require_tasks;
    static std::unordered_map<Task_id,std::function<void(VkCommandBuffer)>> m_compute_barrier_release_tasks;
};

#endif //SHATTER_ENGINE_TASKPOOL_H
