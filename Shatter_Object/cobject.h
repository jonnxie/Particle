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
#include "../Shatter_Item/shatter_enum.h"
#include "../Shatter_Item/shatter_macro.h"
/*
 * compute object
 */
class CObject {
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
