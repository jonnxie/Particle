//
// Created by maybe on 2021/6/6.
//

#ifndef SHATTER_ENGINE_SETPOOL_H
#define SHATTER_ENGINE_SETPOOL_H


#include <vulkan/vulkan.h>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "pool.h"
#include <map>
#include <vector>

class SetPool : public Pool<Set_id,VkDescriptorSet>{
public:
    static SetPool& getPool();
    DefineUnCopy(SetPool);
    void init() override;

    void release() override;

    void setPool(VkDescriptorPool _pool);

    void AllocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& _des_set_layout,
                                VkDescriptorSet* _set) const;

    void AllocateDescriptorSets(const std::vector<Set_id>& _ids,VkDescriptorSet* _set);

    void AllocateDescriptorSets(const std::string& _setId, const Slb_id& _id, VkDescriptorSet* _set);

    void addRelease(VkDescriptorSet _set);

    void allocateDefaultDescriptorSets() const;

    void reallocateDefaultDescriptorSets() ;

    void refine();

    VkDescriptorPool m_set_pool;
    int m_count;
private:
    SetPool():m_set_pool(VK_NULL_HANDLE){};
};

#define SingleSetPool SetPool::getPool()

#endif //SHATTER_ENGINE_SETPOOL_H
