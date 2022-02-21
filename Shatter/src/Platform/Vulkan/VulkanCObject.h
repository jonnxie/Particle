//
// Created by AnWell on 2022/2/21.
//

#ifndef MAIN_VULKANCOBJECT_H
#define MAIN_VULKANCOBJECT_H

#include "Engine/Object/cobject.h"
#include <vulkan/vulkan.h>
#include "Engine/Render/pipeline.h"
#include "Engine/Buffer/shatterbuffer.h"
#include "Engine/Object/ppool.h"
#include "Engine/Object/bpool.h"
#include "Engine/Object/mpool.h"
#include "Engine/Object/setpool.h"
#include "Engine/Object/device.h"

class VulkanCObject : public CObjectBase<VkCommandBuffer>{
public:
    void compute(VkCommandBuffer _cb){
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
    };
};


#endif //MAIN_VULKANCOBJECT_H
