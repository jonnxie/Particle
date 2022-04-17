//
// Created by maybe on 2021/6/6.
//
#include "precompiledhead.h"

#include "setpool.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/Object/device.h"
#include "Engine/Item/shatter_enum.h"
#include <mutex>
#include "slbpool.h"
#include "mpool.h"
#include "Engine/Item/configs.h"

using namespace Shatter::render;
static std::mutex pool_mutex;
static bool created = false;


SetPool &SetPool::getPool() {
    static SetPool pool;
    std::lock_guard<std::mutex> guard_lock(pool_mutex);
    if(!created){
        created = true;
        pool.init();
    }
    return pool;
}

void SetPool::init() {
    m_count = Config::getConfig("DefaultModelCount");
    std::vector<VkDescriptorPoolSize> dps_vec{
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,100},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,100},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,100},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,100}};

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = dps_vec.size();
    poolInfo.pPoolSizes = dps_vec.data();
    poolInfo.maxSets = 400;

    if (vkCreateDescriptorPool(*ShatterRender::getRender().getDevice(), &poolInfo, nullptr, &m_set_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor Pool!");
    }

    std::map<Sl_id,VkDescriptorSetLayout> & tmp_map = SlbPool::getPool().m_sl_map;
    std::vector<VkDescriptorSetLayout> tmp_vec;
    for(auto &i : tmp_map){
        tmp_vec.emplace_back(i.second);
    }
    std::vector<VkDescriptorSet> tmp_set;
    tmp_set.resize(tmp_vec.size());
    AllocateDescriptorSets(tmp_vec,tmp_set.data());
    int index = 0;
    for(auto &i : tmp_map)
    {
        m_map[i.first] = tmp_set[index++];
    }
    allocateDefaultDescriptorSets();
}

void SetPool::refine() {
    std::map<Sl_id,VkDescriptorSetLayout> & tmp_map = SlbPool::getPool().m_sl_map;
    std::vector<VkDescriptorSetLayout> sl_vec;
    std::vector<Sl_id> id_vec;
    for(auto &i : tmp_map){
        if (m_map.find(i.first) == m_map.end())
        {
            sl_vec.emplace_back(i.second);
            id_vec.emplace_back(i.first);
        }
    }
    std::vector<VkDescriptorSet> set_vec;
    set_vec.resize(sl_vec.size());
    AllocateDescriptorSets(sl_vec, set_vec.data());
    for(int index = 0; index < sl_vec.size(); index++)
    {
        m_map[id_vec[index]] = set_vec[index];
    }
}

void SetPool::setPool(VkDescriptorPool _pool){
    m_set_pool = _pool;
}

void SetPool::AllocateDescriptorSets(const std::vector<Set_id>& _ids, VkDescriptorSet *_set) {
    std::vector<VkDescriptorSetLayout> sl_vec;
    for(auto& id : _ids)
    {
        sl_vec.emplace_back(SlbPool::getPool().m_sl_map[id]);
    }
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_set_pool;
    allocInfo.descriptorSetCount = sl_vec.size();
    allocInfo.pSetLayouts = sl_vec.data();

    if (vkAllocateDescriptorSets(Device::getDevice()(), &allocInfo, _set) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    VkDescriptorSet* set = _set;
    for(int i = 0; i < _ids.size(); i++)
    {
        addRelease(*set);
        set++;
    }
}

void SetPool::addRelease(VkDescriptorSet _set){
    static int count = 0;
    m_map[tool::combine("release",count++)] = _set;
}


void SetPool::AllocateDescriptorSets(const std::vector<VkDescriptorSetLayout> &_des_set_layout, VkDescriptorSet *_set) const {
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_set_pool;
    allocInfo.descriptorSetCount = _des_set_layout.size();
    allocInfo.pSetLayouts = _des_set_layout.data();

    if (vkAllocateDescriptorSets(*ShatterRender::getRender().getDevice(), &allocInfo, _set) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }
}

void SetPool::release() {
    vkDestroyDescriptorPool(Device::getDevice()(), m_set_pool, nullptr);
}

void SetPool::allocateDefaultDescriptorSets() const {
    auto set_pool = MPool<VkDescriptorSet>::getPool();

    std::vector<VkDescriptorSetLayout> tmp_vec;
    tmp_vec.assign(m_count,SlbPool::getPool().getSL("Default"));


    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_set_pool;
    allocInfo.descriptorSetCount = m_count;
    allocInfo.pSetLayouts = tmp_vec.data();

    if (vkAllocateDescriptorSets(Device::getDevice()(), &allocInfo, (*set_pool)()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }
}

void SetPool::reallocateDefaultDescriptorSets() {
    auto set_pool = MPool<VkDescriptorSet>::getPool();
    std::vector<VkDescriptorSetLayout> tmp_vec;
    tmp_vec.assign(m_count,SlbPool::getPool().getSL("Default"));

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_set_pool;
    allocInfo.descriptorSetCount = m_count;
    allocInfo.pSetLayouts = tmp_vec.data();

    if (vkAllocateDescriptorSets(Device::getDevice()(), &allocInfo, (*set_pool)[m_count]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }
    m_count *=2;
}



