//
// Created by maybe on 2021/6/5.
//

#ifndef SHATTER_ENGINE_PPOOL_H
#define SHATTER_ENGINE_PPOOL_H


#include <map>
#include <mutex>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "pool.h"
#include <vulkan/vulkan.h>
#include <vector>

class Pipeline;

class PPool : public Pool<P_id,Pipeline*>{
public:
    static PPool& getPool();
    PPool(const PPool&) = delete;
    PPool(PPool&&) = delete;
    PPool& operator=(const PPool&) = delete;
    PPool& operator=(PPool&&) = delete;

    void init() override;

    void release() override;

    void createGP(const P_id & _id,
                  const std::vector<Input_Type>& _inputType,
                  const std::vector<Shader_id>& _shader,
                  AssemState _assemState,
                  RasterState _rasterState,
                  MultisampleState _multisampleState,
                  DepthStencilState _depthStencilState,
                  BlendState _blendState,
                  RenderPassType _passType= RenderPassType::Color,
                  uint32_t _pass = SubpassG
                  );

    void createCP(const P_id & _id,
                  const Shader_id& _shader);

    void createRP(const P_id& _id,
                  const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                  const std::vector<VkRayTracingShaderGroupCreateInfoKHR>& _group,
                  const std::vector<Sl_id>& _sl_id,
                  uint32_t _recursion_depth = 2);

private:
    PPool() = default;
};

#define SinglePPool PPool::getPool()

#endif //SHATTER_ENGINE_PPOOL_H
