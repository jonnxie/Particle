//
// Created by maybe on 2021/8/5.
//
#include "precompiledhead.h"

#include "cobject.h"
#include "Engine/Renderer/pipeline.h"
#include "Engine/Buffer/shatterbuffer.h"
#include "Engine/pool/ppool.h"
#include "Engine/pool/bpool.h"
#include "Engine/pool/mpool.h"
#include "Engine/pool/setpool.h"
#include "device.h"
#include <algorithm>


void CObject::compute(VkCommandBuffer _cb)
{
    std::vector<VkDescriptorSet> set_vec;
    loopPre(_cb);
    for(auto i = 0 ; i < m_cps.size() ; i++)
    {
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_COMPUTE,(*PPool::getPool()[m_cps[i]])());

        set_vec.clear();
        for(auto & set: m_descriptorSets[i])
        {
            set_vec.emplace_back(SetPool::getPool()[set]);
        }

        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                PPool::getPool()[m_cps[i]]->getPipelineLayout(),
                                0,
                                set_vec.size(),
                                set_vec.data(),
                                0,
                                nullptr);

        vkCmdDispatch(_cb, m_groups[i], 1, 1);
        m_tasks[i](_cb);
    }
    loopLater(_cb);

}

void CObject::prepare(const std::vector<P_id>& _pipeline,
                      std::vector<std::vector<Set_id>>& _set,
                      std::vector<int>& _group,
                      const std::vector<std::function<void(VkCommandBuffer)>>& _tasks)
{
     m_cps = _pipeline;

    /*
     * descriptor set
     */
     m_descriptorSets = _set;

     m_groups = _group;

     m_tasks = _tasks;
}

void CObject::loopPre(VkCommandBuffer _cb) {

    for(auto &task : m_pre_tasks){
        task(_cb);
    }

}

void CObject::loopLater(VkCommandBuffer _cb) {
    for(auto &task : m_later_tasks){
        task(_cb);
    }

}

void CObject::insert(const std::function<void(VkCommandBuffer)>& _task, InsertTaskState _state) {
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
}
















