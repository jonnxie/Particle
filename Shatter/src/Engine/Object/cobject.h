//
// Created by maybe on 2021/8/5.
//

#ifndef SHATTER_ENGINE_COBJECT_H
#define SHATTER_ENGINE_COBJECT_H

#include <glm.hpp>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <future>
#include <type_traits>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"

template<class CommandBuffer>
class CObjectBase{
public:
    virtual void compute(CommandBuffer _cb){};

    void prepare(const std::vector<P_id>& _pipeline,
                 std::vector<std::vector<Set_id>>& _set,
                 std::vector<int>& _group,
                 const std::vector<std::function<void(CommandBuffer)>>& _tasks)
    {
        m_cps = _pipeline;

        /*
         * descriptor set
         */
        m_descriptorSets = _set;

        m_groups = _group;

        m_tasks = _tasks;
    };

    void insert(const std::function<void(CommandBuffer)>& _task,InsertTaskState _state)
    {
        switch (_state) {
            case InsertTaskState::Pre:{
                m_pre_tasks.push_back(_task);
                break;
            }
            case InsertTaskState::Later:{
                m_later_tasks.push_back(_task);
                break;
            }
            default:
            {
                break;
            }
        }
    };

    void loopPre(CommandBuffer _cb){
        for(auto &task : m_pre_tasks){
            task(_cb);
        }
    };

    void loopLater(CommandBuffer _cb){
        for(auto &task : m_later_tasks){
            task(_cb);
        }
    };
    /*
    * compute pipeline
    */
    std::vector<P_id> m_cps;

    /*
     * descriptor set
     */
    std::vector<std::vector<Set_id>> m_descriptorSets;

    std::vector<int> m_groups;

    std::vector<std::function<void(CommandBuffer)>> m_tasks;

    /*
     * task used in pre compute
     */
    std::vector<std::function<void(CommandBuffer)>> m_pre_tasks;


    /*
     * task used in later compute
     */
    std::vector<std::function<void(CommandBuffer)>> m_later_tasks;
};

/*
 * compute object
 */
class CObject {
public:
    CObject() = default;
    ~CObject() = default;
public:
    virtual void compute(VkCommandBuffer _cb);

    void prepare(const std::vector<P_id>& _pipeline,
                 std::vector<std::vector<Set_id>>& _set,
                 std::vector<int>& _group,
                 const std::vector<std::function<void(VkCommandBuffer)>>& _tasks);

    void insert(const std::function<void(VkCommandBuffer)>&,InsertTaskState);

    void loopPre(VkCommandBuffer);

    void loopLater(VkCommandBuffer);
    /*
    * compute pipeline
    */
    std::vector<P_id> m_cps;

    /*
     * descriptor set
     */
    std::vector<std::vector<Set_id>> m_descriptorSets;

    std::vector<int> m_groups;

    std::vector<std::function<void(VkCommandBuffer)>> m_tasks;

    /*
     * task used in pre compute
     */
    std::vector<std::function<void(VkCommandBuffer)>> m_pre_tasks;


    /*
     * task used in later compute
     */
    std::vector<std::function<void(VkCommandBuffer)>> m_later_tasks;
};



#endif //SHATTER_ENGINE_COBJECT_H
