//
// Created by AnWell on 2022/2/21.
//

#ifndef MAIN_VULKANDOBJECT_H
#define MAIN_VULKANDOBJECT_H

#include "Engine/Object/dobject.h"
#include <vulkan/vulkan.h>
#include "Engine/Buffer/shatterbuffer.h"
#include "Engine/pool/ppool.h"
#include "Engine/pool/bpool.h"
#include "Engine/pool/mpool.h"
#include "Engine/pool/setpool.h"
#include "Engine/Object/device.h"
#include "Engine/Render/pipeline.h"

class VulkanDObject : public DObjectBase<VkCommandBuffer>
{
public:
    virtual void draw(VkCommandBuffer _cb,int _imageIndex){
        loopPre(_cb);
        m_ray_task(_cb,_imageIndex);
        loopLater(_cb);
    };

    virtual void drawDepth(VkCommandBuffer _cb){
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS,(*PPool::getPool()[m_depthGP])());

        if(m_drawType != DrawType::Indirect){
            VkBuffer bf = (*BPool::getPool().getBuffer(m_vertex_buffer,Buffer_Type::Vertex_Buffer))();
            vkCmdBindVertexBuffers(_cb, 0, 1, &bf, &m_offsets);
            if(m_drawType == DrawType::Index){
                bf = (*BPool::getPool().getBuffer(m_index_buffer,Buffer_Type::Index_Buffer))();
                vkCmdBindIndexBuffer(_cb,bf,m_index_offsets,VK_INDEX_TYPE_UINT32);
            }
        }
        VkViewport tmp = getViewPort();
        vkCmdSetViewport(_cb,0,1,&tmp);

        VkRect2D scissor = getScissor();
        vkCmdSetScissor(_cb,0,1,&scissor);
        std::vector<VkDescriptorSet> set_vec;
        auto set_pool = MPool<VkDescriptorSet>::getPool();
        set_vec.push_back(*(*set_pool)[m_model_index]);
        for(auto & i: m_depthDescriptorSet)
        {
            set_vec.emplace_back(SetPool::getPool()[i]);
        }
        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()[m_depthGP]->getPipelineLayout(),
                                0,
                                set_vec.size(),
                                set_vec.data(),
                                0,
                                nullptr);
        if(m_drawType == DrawType::Index){
            vkCmdDrawIndexed(_cb,m_index_num,1,0,m_offsets,0);
        }else{
            vkCmdDraw(_cb,m_vertex_num,1,0,0);
        }
    };

    virtual void draw(VkCommandBuffer _cb) {
        loopPre(_cb);

        if(m_type == DType::Normal)
        {
            vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS,(*PPool::getPool()[m_gp])());

            if(m_drawType != DrawType::Indirect){
                VkBuffer bf = (*BPool::getPool().getBuffer(m_vertex_buffer,Buffer_Type::Vertex_Buffer))();
                vkCmdBindVertexBuffers(_cb, 0, 1, &bf, &m_offsets);
                if(m_drawType == DrawType::Index){
                    bf = (*BPool::getPool().getBuffer(m_index_buffer,Buffer_Type::Index_Buffer))();
                    vkCmdBindIndexBuffer(_cb,bf,m_index_offsets,VK_INDEX_TYPE_UINT32);
                }
            }
            VkViewport tmp = getViewPort();
            vkCmdSetViewport(_cb,0,1,&tmp);

            VkRect2D scissor = getScissor();
            vkCmdSetScissor(_cb,0,1,&scissor);
            std::vector<VkDescriptorSet> set_vec;
            set_vec.resize(0);
            auto set_pool = MPool<VkDescriptorSet>::getPool();
            set_vec.push_back(*(*set_pool)[m_model_index]);
            for(auto & i: m_descriptorSet)
            {
                set_vec.emplace_back(SetPool::getPool()[i]);
            }

            vkCmdBindDescriptorSets(_cb,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    PPool::getPool()[m_gp]->getPipelineLayout(),
                                    0,
                                    set_vec.size(),
                                    set_vec.data(),
                                    0,
                                    nullptr);
            if(m_drawType == DrawType::Index){
                vkCmdDrawIndexed(_cb,m_index_num,1,0,m_offsets,0);
            }else{
                vkCmdDraw(_cb,m_vertex_num,1,0,0);
            }
        }else if(m_type == DType::Instance)
        {
            m_instance_task(_cb);
        }

        loopLater(_cb);
    };

    virtual void g(VkCommandBuffer _cb) {
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS,(*PPool::getPool()[m_gGP])());

        if(m_drawType != DrawType::Indirect){
            VkBuffer bf = (*BPool::getPool().getBuffer(m_vertex_buffer,Buffer_Type::Vertex_Buffer))();
            vkCmdBindVertexBuffers(_cb, 0, 1, &bf, &m_offsets);
            if(m_drawType == DrawType::Index){
                bf = (*BPool::getPool().getBuffer(m_index_buffer,Buffer_Type::Index_Buffer))();
                vkCmdBindIndexBuffer(_cb,bf,m_index_offsets,VK_INDEX_TYPE_UINT32);
            }
        }
        VkViewport tmp = getViewPort();
        vkCmdSetViewport(_cb,0,1,&tmp);

        VkRect2D scissor = getScissor();
        vkCmdSetScissor(_cb,0,1,&scissor);
        std::vector<VkDescriptorSet> set_vec;
        auto set_pool = MPool<VkDescriptorSet>::getPool();
        set_vec.push_back(*(*set_pool)[m_model_index]);
        for(auto & i: m_gDescriptorSet)
        {
            set_vec.emplace_back(SetPool::getPool()[i]);
        }

        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()[m_gGP]->getPipelineLayout(),
                                0,
                                set_vec.size(),
                                set_vec.data(),
                                0,
                                nullptr);
        if(m_drawType == DrawType::Index){
            vkCmdDrawIndexed(_cb,m_index_num,1,0,m_offsets,0);
        }else{
            vkCmdDraw(_cb,m_vertex_num,1,0,0);
        }
    };
};



#endif //MAIN_VULKANDOBJECT_H
