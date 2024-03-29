//
// Created by maybe on 2021/6/2.
//
#include "precompiledhead.h"

#include "dobject.h"
#include "Engine/Buffer/shatterbuffer.h"
#include "Engine/pool/ppool.h"
#include "Engine/pool/bpool.h"
#include "Engine/pool/mpool.h"
#include "Engine/pool/setpool.h"
#include "device.h"
#include "Engine/Renderer/pipeline.h"
#include <algorithm>
#include <utility>
#include <Engine/App/shatterapp.h>

DObject::DObject(){
    m_drawDepth =[this](VkCommandBuffer _cb){
        drawDepth(_cb);
    };

    m_newDraw = [this](VkCommandBuffer _cb){
        newDraw(_cb);
    };

    m_gGraphics = [this](VkCommandBuffer _cb){
        g(_cb);
    };
}

D_id DObject::getMId() const {
    return m_id;
}

void DObject::draw(VkCommandBuffer _cb) {
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
        tool::cmdDynamicState(_cb);
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
}

void DObject::newDraw(VkCommandBuffer _cb)
{
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
        tool::cmdDynamicState(_cb);
        std::vector<VkDescriptorSet> set_vec;
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
}


void DObject::draw(VkCommandBuffer _cb,int _imageIndex)
{
    loopPre(_cb);
    m_ray_task(_cb,_imageIndex);
    loopLater(_cb);
}


void DObject::update() {
    if(state_changed)
    {
        if(m_type == DType::Normal)
        {
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + m_model_index,&m_matrix,one_matrix);
        }
        state_changed = false;
    }
}


void DObject::prepare(const glm::mat4& _matrix, int _model_index, DrawType _draw_type, VkDeviceSize _vertex_offset,
                      const std::string &_vertex_buffer, uint32_t _vertex_num, const std::string& _index_buffer, uint32_t _index_num,
                      VkDeviceSize _index_offset, const std::string &_pipeline, std::vector<Set_id> &_set,const std::string& _gPipeline,std::vector<Set_id> _gSet) {
    m_matrix = _matrix;
    /*
     * 获取模型矩阵的偏移索引
     */
    m_model_index = _model_index;

    m_drawType = _draw_type;
    /*
    * vertex buffer
    */
    m_offsets = _vertex_offset;
    m_vertex_buffer = _vertex_buffer;
    m_vertex_num = _vertex_num;

    /*
     * index buffer
     */
//    m_index_buffer = std::string{};
    m_index_buffer = _index_buffer;
    m_index_num = _index_num;
    m_index_offsets = _index_offset;

    /*
    * graphics pipeline
    */
    m_gp = _pipeline;

    m_gGP = _gPipeline;

    /*
     * descriptor set
     */
    m_descriptorSet = _set;

    m_gDescriptorSet = std::move(_gSet);
}

void DObject::loopPre(VkCommandBuffer _cb) {
    for(auto &task : m_pre_tasks){
        task(_cb);
    }
}

void DObject::loopLater(VkCommandBuffer _cb) {
    for(auto &task : m_later_tasks){
        task(_cb);
    }
}

void DObject::insertPre(const std::function<void(VkCommandBuffer)>& _func) {
    m_pre_tasks.push_back(_func);
}

void DObject::insertLater(const std::function<void(VkCommandBuffer)> & _func) {
    m_later_tasks.push_back(_func);
}

void DObject::drawDepth(VkCommandBuffer _cb){
    vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS,(*PPool::getPool()[m_depthGP])());

    if(m_drawType != DrawType::Indirect){
        VkBuffer bf = (*BPool::getPool().getBuffer(m_vertex_buffer,Buffer_Type::Vertex_Buffer))();
        vkCmdBindVertexBuffers(_cb, 0, 1, &bf, &m_offsets);
        if(m_drawType == DrawType::Index){
            bf = (*BPool::getPool().getBuffer(m_index_buffer,Buffer_Type::Index_Buffer))();
            vkCmdBindIndexBuffer(_cb,bf,m_index_offsets,VK_INDEX_TYPE_UINT32);
        }
    }
    tool::cmdDynamicState(_cb);
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
}

void DObject::g(VkCommandBuffer _cb){
    vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS,(*PPool::getPool()[m_gGP])());

    if(m_drawType != DrawType::Indirect){
        VkBuffer bf = (*BPool::getPool().getBuffer(m_vertex_buffer,Buffer_Type::Vertex_Buffer))();
        vkCmdBindVertexBuffers(_cb, 0, 1, &bf, &m_offsets);
        if(m_drawType == DrawType::Index){
            bf = (*BPool::getPool().getBuffer(m_index_buffer,Buffer_Type::Index_Buffer))();
            vkCmdBindIndexBuffer(_cb,bf,m_index_offsets,VK_INDEX_TYPE_UINT32);
        }
    }
    tool::cmdDynamicState(_cb);
    std::vector<VkDescriptorSet> set_vec;
    auto set_pool = MPool<VkDescriptorSet>::getPool();
    set_vec.push_back(*(*set_pool)[m_model_index]);
    for(auto& i: m_gDescriptorSet)
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
}


