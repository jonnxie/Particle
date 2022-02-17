//
// Created by maybe on 2021/6/7.
//

#ifndef SHATTER_ENGINE_SLBPOOL_H
#define SHATTER_ENGINE_SLBPOOL_H


#include <vulkan/vulkan.h>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include <map>
#include <vector>
#include "pool.h"
#include <unordered_map>

class SlbPool : public Pool<Slb_id,std::vector<VkDescriptorSetLayoutBinding>>{
public:
    static SlbPool& getPool();
    SlbPool(const SlbPool&) = delete;
    SlbPool& operator= (const SlbPool&) = delete;

    void init() override;

    void release() override;

    VkDescriptorSetLayout getSL(const Sl_id& _id);
    void setSL(const Sl_id& _id,VkDescriptorSetLayout _sl);
    void insertSL(const Sl_id& _id,std::vector<VkDescriptorSetLayoutBinding> _bindingVec);

    std::map<Sl_id,VkDescriptorSetLayout> m_sl_map;
    std::unordered_map<Sl_id,int> m_count;
private:
    SlbPool() = default;
};


#define SingleSLBPool SlbPool::getPool()

#endif //SHATTER_ENGINE_SLBPOOL_H
