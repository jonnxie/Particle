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
#include "spirv_reflect.h"
#include "Engine/Item/shatter_item.h"

class ShaderPool : public Pool<Shader_id,VkPipelineShaderStageCreateInfo> {
public:
    static ShaderPool& getPool();
    DefineUnCopy(ShaderPool);

    void init() override;

    void release() override;

    void createShaderStage(const Shader_id& _id,const std::string& _filename,ShaderType _type);

    std::unordered_map<Shader_id, std::vector<VKSL>>& getSLMap();

private:
    ShaderPool() = default;
    std::unordered_map<Shader_id, std::vector<VkDescriptorSetLayout>> reflectSlMap{};
    std::unordered_map<Shader_id, std::vector<VkPipelineVertexInputStateCreateInfo>> vertexInputMap{};
};

#define SingleShaderPool ShaderPool::getPool()


#endif //SHATTER_ENGINE_SHADERPOOL_H
