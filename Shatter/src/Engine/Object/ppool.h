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

//    void createGP(const P_id & _id,
//                  const std::vector<Input_Type>& _inputType,
//                  const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
//                  AssemState _assemState,
//                  RasterState _rasterState,
//                  MultisampleState _multisampleState,
//                  DepthStencilState _depthStencilState,
//                  BlendState _blendState,
//                  const std::vector<Sl_id>& _sl_id,
//                  uint32_t _pass = SubpassG,
//                  bool _off = false);

    void createGP(const P_id & _id,
                  const std::vector<Input_Type>& _inputType,
                  const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                  AssemState _assemState,
                  RasterState _rasterState,
                  MultisampleState _multisampleState,
                  DepthStencilState _depthStencilState,
                  BlendState _blendState,
                  const std::vector<Sl_id>& _sl_id,
                  RenderPassType _passType= RenderPassType::Default,
                  uint32_t _pass = SubpassG
                  );

    void createCP(const P_id & _id,
                  const VkPipelineShaderStageCreateInfo& _shader,
                  const std::vector<Sl_id>& _descriptorSetLayoutBinding);

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
