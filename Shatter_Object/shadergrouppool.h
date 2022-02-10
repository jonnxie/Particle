//
// Created by jonnxie on 2021/10/5.
//

#ifndef SHATTER_ENGINE_SHADERGROUPPOOL_H
#define SHATTER_ENGINE_SHADERGROUPPOOL_H

#include <iostream>
#include <map>
#include "pool.h"
#include "../Shatter_Item/shatter_enum.h"
#include ItemCatalog

class ShaderGroupPool : public Pool<SGroup_id,std::vector<VkRayTracingShaderGroupCreateInfoKHR>> {
public:
    static ShaderGroupPool& getPool();

    ShaderGroupPool(const ShaderGroupPool& _other) = delete;

    ShaderGroupPool& operator = (const ShaderGroupPool& _other) = delete;

    void init() override;

    void release() override;

//    void createShaderBindingTable();

private:
    ShaderGroupPool() = default;
};

#define SingleShaderGroupPool ShaderGroupPool::getPool()


#endif //SHATTER_ENGINE_SHADERGROUPPOOL_H
