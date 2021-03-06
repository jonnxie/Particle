//
// Created by maybe on 2021/7/25.
//
#include "precompiledhead.h"

#include "modelsetpool.h"
#include "bpool.h"
#include "mpool.h"
#include "setpool.h"
#include "Engine/Object/device.h"
#include "Engine/Buffer/shatterbufferinclude.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/Item/configs.h"

static std::mutex pool_mutex;
static bool if_get = false;


ModelSetPool& ModelSetPool::getPool()
{
    static ModelSetPool pool;
    std::lock_guard<std::mutex> guard_mutex(pool_mutex);
    if(!if_get){
        if_get = true;
        pool.init();
    }
    return pool;
}


void ModelSetPool::init()
{
    m_model_count = Config::getConfig("DefaultModelCount");

    VkBuffer buffer = (*BPool::getPool().getBuffer("Model", Buffer_Type::Uniform_Buffer))();

    std::vector<VkDescriptorBufferInfo> model_buffer(m_model_count);
    std::vector<VkWriteDescriptorSet> write_set(m_model_count);

    auto set_pool = MPool<VkDescriptorSet>::getPool();

    for(auto i = 0 ; i < m_model_count; i++)
    {
        model_buffer[i].buffer = buffer;
        model_buffer[i].offset = i * sizeof(glm::mat4);
        model_buffer[i].range = sizeof(glm::mat4);

        write_set[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_set[i].pNext = VK_NULL_HANDLE;
        write_set[i].dstSet = *(*set_pool)[i];
        write_set[i].dstBinding = 0;
        write_set[i].dstArrayElement = 0;
        write_set[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_set[i].descriptorCount = 1;
        write_set[i].pBufferInfo = &model_buffer[i];
    }
    VkDevice device = Device::getDevice()();
    vkUpdateDescriptorSets(device,
                           m_model_count,
                           write_set.data(),
                           0,
                           nullptr);
    m_idle.resize(m_model_count);
    for(auto i = 0 ; i < m_model_count ; i++){
        m_idle[i] = i;
    }
}

void ModelSetPool::reallocate()
{
    m_model_count*=2;
    auto set_pool = MPool<VkDescriptorSet>::getPool();
    set_pool->reallocated();
    vkQueueWaitIdle(SingleRender.graphics_queue);
    BPool::getPool().reallocateModel();
    SetPool::getPool().reallocateDefaultDescriptorSets();
    update();
}

void ModelSetPool::update()
{
    VkBuffer buffer = (*BPool::getPool().getBuffer("Model", Buffer_Type::Uniform_Buffer))();

    std::vector<VkDescriptorBufferInfo> model_buffer(m_model_count);
    std::vector<VkWriteDescriptorSet> write_set(m_model_count);


    auto set_pool = MPool<VkDescriptorSet>::getPool();

    for(auto i = 0 ; i < m_model_count; i++)
    {
        model_buffer[i].buffer = buffer;
        model_buffer[i].offset = i * sizeof(glm::mat4);
        model_buffer[i].range = sizeof(glm::mat4);

        write_set[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_set[i].pNext = VK_NULL_HANDLE;
        write_set[i].dstSet = *(*set_pool)[i];
        write_set[i].dstBinding = 0;
        write_set[i].dstArrayElement = 0;
        write_set[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_set[i].descriptorCount = 1;
        write_set[i].pBufferInfo = &model_buffer[i];
    }

    vkUpdateDescriptorSets(Device::getDevice()(),
                           m_model_count,
                           write_set.data(),
                           0,
                           nullptr);
    for(auto i = m_model_count / 2 ; i < m_model_count ; i++){
        m_idle.emplace_back(i);
    }
}

int ModelSetPool::malloc()
{
    std::lock_guard<std::mutex> guard_mutex(m_mutex);
    int val;
    if(m_idle.empty())
    {
        reallocate();
    }
    val = m_idle.front();
    m_idle.erase(m_idle.begin());
    return val;
}

void ModelSetPool::free(int _index)
{
    std::lock_guard<std::mutex> guard_mutex(m_mutex);
    if (_index != -1) {
        m_idle.insert(m_idle.end(),_index);
    }
}








