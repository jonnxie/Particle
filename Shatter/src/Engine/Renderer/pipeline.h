//
// Created by jonnxie on 2022/1/7.
//

#ifndef SHATTER_ENGINE_PIPELINE_H
#define SHATTER_ENGINE_PIPELINE_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include <vulkan/vulkan.h>

class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline();
    Pipeline(const Pipeline& _p) = delete;
    Pipeline(Pipeline&& _p) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;
    static Pipeline* createRayTracingPipeline(const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                                              const std::vector<VkRayTracingShaderGroupCreateInfoKHR>& _group,
                                              const std::vector<Sl_id>& _sl_id,
                                              uint32_t _recursion_depth);

    void initRayTracingPipeline(const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                                const std::vector<VkRayTracingShaderGroupCreateInfoKHR>& _group,
                                const std::vector<Sl_id>& _sl_id,
                                uint32_t _recursion_depth);
    static Pipeline* createCP(const Shader_id& _shader);
    void initCP(const Shader_id& _shader);
public:
    VkPipelineLayout getPipelineLayout(){return m_pipelineLayout;};
    VkPipeline getPipeline(){return m_pipeline;};
    VkPipeline operator()(){return m_pipeline;};
protected:
    VkPipelineLayout m_pipelineLayout {VK_NULL_HANDLE};
    VkPipeline m_pipeline {VK_NULL_HANDLE};
};

VkPipelineInputAssemblyStateCreateInfo getAssemblyState(AssemState _state);

VkPipelineRasterizationStateCreateInfo getRasterState(RasterState _state);

VkPipelineMultisampleStateCreateInfo getMultisampleState(MultisampleState _state);

VkPipelineDepthStencilStateCreateInfo getDepthStencilState(DepthStencilState _state);

class GP : public Pipeline{
public:
    GP();
    static GP* createGP(const std::vector<Input_Type>& _inputType,
                        const std::vector<Shader_id>& _shader,
                        AssemState _assemState,
                        RasterState _rasterState,
                        MultisampleState _multisampleState,
                        DepthStencilState _depthStencilState,
                        BlendState _blendState,
                        const std::vector<Sl_id>& _sl_id,
                        RenderPassType _passType = RenderPassType::Color,
                        uint32_t _pass = 0
    );


    void initGP(const std::vector<Input_Type>& _inputType,
                const std::vector<Shader_id>& _shader,
                AssemState _assemState,
                RasterState _rasterState,
                MultisampleState _multisampleState,
                DepthStencilState _depthStencilState,
                BlendState _blendState,
                const std::vector<Sl_id>& _sl_id,
                uint32_t _pass,
                RenderPassType _passType = RenderPassType::Color);
public:
    void setVertexInputInfo(VkPipelineVertexInputStateCreateInfo* createinfo);
    void setShaderStage(std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_vec);
    void setAssemblyState(VkPipelineInputAssemblyStateCreateInfo* assembly_state);
    void setTessellationState(VkPipelineTessellationStateCreateInfo* pTessellationState);
    void setViewportState(VkPipelineViewportStateCreateInfo* viewport_state);
    void setRasterizationState(VkPipelineRasterizationStateCreateInfo* rasterizer);
    void setMultisampleState(VkPipelineMultisampleStateCreateInfo* multisampling);
    void setDepthStencilState(VkPipelineDepthStencilStateCreateInfo* depthStencil);
    void setColorBlendState(BlendState _state);
    void setDynamicState(VkPipelineDynamicStateCreateInfo* pDynamicState);
    void setPipelineLayout(VkPipelineLayout pipelineLayout);
    void setRenderPass(VkRenderPass renderPass);
    void setSubpass(uint32_t subpass);
    void setBasePipelineHandle(VkPipeline basePipelineHandle);
    void setBasePipelineIndex(int32_t basePipelineIndex);
private:
    VkGraphicsPipelineCreateInfo m_createInfo{};
    VkPipelineColorBlendAttachmentState m_blendAttach{};
    std::vector<VkPipelineColorBlendAttachmentState> m_blendAttachs{};
    VkPipelineColorBlendStateCreateInfo m_blendState{};
    VkViewport m_defaultViewport{};
    VkRect2D m_defaultScissor{};
    VkPipelineViewportStateCreateInfo m_defaultViewportStateCreateInfo{};
};



#endif //SHATTER_ENGINE_PIPELINE_H
