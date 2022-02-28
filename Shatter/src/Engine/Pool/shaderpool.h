//
// Created by maybe on 2021/6/6.
//

#ifndef SHATTER_ENGINE_SHADERPOOL_H
#define SHATTER_ENGINE_SHADERPOOL_H

#include <map>
#include <vulkan/vulkan.h>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "pool.h"

class ShaderPool : public Pool<Shader_id,VkPipelineShaderStageCreateInfo> {
public:
    static ShaderPool& getPool();

    ShaderPool(const ShaderPool& _other) = delete;

    ShaderPool& operator = (const ShaderPool& _other) = delete;

    void init() override;

    void release() override;

    void createShaderStage(const Shader_id& _id,const std::string& _filename,ShaderType _type);

private:
    ShaderPool() = default;
};

#define SingleShaderPool ShaderPool::getPool()


#endif //SHATTER_ENGINE_SHADERPOOL_H
