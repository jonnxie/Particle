//
// Created by jonnxie on 2022/1/7.
//
#include "precompiledhead.h"
#include "pipeline.h"
#include DeviceCatalog
#include SlbPoolCatalog
//#include RayCatalog
#include OffScreenCatalog
#include InputTypeCatalog
#include RenderCatalog
#include "renderer.h"

VkPipelineInputAssemblyStateCreateInfo getAssemblyState(AssemState _state){
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    switch (_state) {
        case AssemState::Point_List:{
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;
            break;
        }
        case AssemState::Line_Strip:{
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            inputAssembly.primitiveRestartEnable = VK_TRUE;
            break;
        }
        case AssemState::Line_List:{
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;
            break;
        }
        case AssemState::Triangle_List:{
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;
            break;
        }
        case AssemState::Triangle_Strip:{
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            inputAssembly.primitiveRestartEnable = VK_FALSE;
            break;
        }
        default:{
            throw std::runtime_error("get assembly_state error!");
        }
    }

    return inputAssembly;
}

VkPipelineRasterizationStateCreateInfo getRasterState(RasterState _state){
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.lineWidth = 1.0f;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    switch(_state){
        case RasterState::Point :{
            rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.depthBiasEnable = VK_FALSE;
            break;
        }
        case RasterState::Polyline :{
            rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.depthBiasEnable = VK_FALSE;
            break;
        }
        case RasterState::TriangleLine :{
            rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.depthBiasEnable = VK_FALSE;
            break;
        }
        case RasterState::TriangleFace :{
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.depthBiasEnable = VK_FALSE;
            break;
        }
        case RasterState::CPU_Particle :{
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.depthBiasEnable = VK_FALSE;
            break;
        }
        case RasterState::Marching_Cube :{
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.depthBiasEnable = VK_FALSE;
            break;
        }
        case RasterState::Back_Face :{
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
            rasterizer.depthBiasEnable = VK_FALSE;
            break;
        }
        default :{
            throw std::runtime_error("chose rasterization state error!");
        }
    }
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo getMultisampleState(MultisampleState _state){
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    switch(_state){
        case MultisampleState::Default : {
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            break;
        }
        default : {
            throw std::runtime_error("chose multisample state error!");
        }
    }
    return multisampling;
}

VkPipelineDepthStencilStateCreateInfo getDepthStencilState(DepthStencilState _state){
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    switch(_state){
        case DepthStencilState::Default:{
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
            break;
        }
        case DepthStencilState::UnderScreenLine:{
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
            break;
        }
        case DepthStencilState::DepthWriteDisable:{
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_FALSE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
            break;
        }
        case DepthStencilState::DepthDisable:{
            depthStencil.depthTestEnable = VK_FALSE;
            depthStencil.depthWriteEnable = VK_FALSE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
            break;
        }
        case DepthStencilState::MaxDepth:{
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
            break;
        }
        default:{
            throw std::runtime_error("chose depth_stencil_state error!");
        }
    }

    return depthStencil;
}

Pipeline::~Pipeline(){
    vkDestroyPipelineLayout(SingleDevice(), m_pipelineLayout, nullptr);
    vkDestroyPipeline(SingleDevice(), m_pipeline, nullptr);
}

Pipeline* Pipeline::createRayTracingPipeline(const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                                          const std::vector<VkRayTracingShaderGroupCreateInfoKHR>& _group,
                                          const std::vector<Sl_id>& _sl_id,
                                          uint32_t _recursion_depth)
{
    auto tracer = new Pipeline;
    tracer->initRayTracingPipeline(_shader,_group,_sl_id,_recursion_depth);
    return tracer;
}

void Pipeline::initRayTracingPipeline(const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                            const std::vector<VkRayTracingShaderGroupCreateInfoKHR>& _group,
                            const std::vector<Sl_id>& _sl_id,
                            uint32_t _recursion_depth)
{
    std::vector<VkDescriptorSetLayout> setLayout;
    setLayout.reserve(_sl_id.size());
    for(auto &i: _sl_id){
        setLayout.push_back(SlbPool::getPool().getSL(i));
    }

    /*PipelineLayoutCreateInfo*/
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            nullptr,
            0,
            uint32_t(setLayout.size()),
            setLayout.data(),
            0,
            nullptr};

    if (vkCreatePipelineLayout(SingleDevice(),
                               &pipelineLayoutInfo,
                               nullptr,
                               &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    /*
     * group
     */
    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI = tool::rayTracingPipelineCreateInfoKHR();
    rayTracingPipelineCI.stageCount = static_cast<uint32_t>(_shader.size());
    rayTracingPipelineCI.pStages = _shader.data();
    rayTracingPipelineCI.groupCount = static_cast<uint32_t>(_group.size());
    rayTracingPipelineCI.pGroups = _group.data();
    rayTracingPipelineCI.maxPipelineRayRecursionDepth = _recursion_depth;
    rayTracingPipelineCI.layout = m_pipelineLayout;

//    VK_CHECK_RESULT(Raytracing::getTracer()->vkCreateRayTracingPipelinesKHR(Device::getDevice().logicalDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &m_pipeline));
}

Pipeline* Pipeline::createCP(const VkPipelineShaderStageCreateInfo& _shader,
                   const std::vector<Sl_id>&_sl_id)
{
    auto cp = new Pipeline;
    cp->initCP(_shader,_sl_id);
    return cp;
}

void Pipeline::initCP(const VkPipelineShaderStageCreateInfo& _shader,
            const std::vector<Sl_id>& _sl_id){
    std::vector<VkDescriptorSetLayout> setLayout;
    setLayout.reserve(_sl_id.size());
    for(auto &i: _sl_id){
        setLayout.push_back(SlbPool::getPool().getSL(i));
    }
    /*PipelineLayoutCreateInfo*/
    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
            {
                    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    nullptr,
                    0,
                    uint32_t(setLayout.size()),
                    setLayout.data(),
                    0,
                    nullptr
            };

    if (vkCreatePipelineLayout(SingleDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    auto createInfo = tool::computePipelineCreateInfo(m_pipelineLayout);
    createInfo.stage = _shader;

    if (vkCreateComputePipelines(SingleDevice(),
                                 VK_NULL_HANDLE,
                                 1,
                                 &createInfo,
                                 nullptr, &m_pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }
}

void GP::setVertexInputInfo(VkPipelineVertexInputStateCreateInfo* createinfo){
    m_createInfo.pVertexInputState = createinfo;
}

void GP::setShaderStage(std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_vec){
    m_createInfo.stageCount = shader_stage_vec.size();
    m_createInfo.pStages = shader_stage_vec.data();
}

void GP::setAssemblyState(VkPipelineInputAssemblyStateCreateInfo* assembly_state){
    m_createInfo.pInputAssemblyState = assembly_state;
}

void GP::setTessellationState(VkPipelineTessellationStateCreateInfo* pTessellationState){
    m_createInfo.pTessellationState = pTessellationState;
}

void GP::setViewportState(VkPipelineViewportStateCreateInfo* viewport_state){
    m_createInfo.pViewportState = viewport_state;
}

void GP::setRasterizationState(VkPipelineRasterizationStateCreateInfo* rasterizer){
    m_createInfo.pRasterizationState = rasterizer;
}

void GP::setMultisampleState(VkPipelineMultisampleStateCreateInfo* multisampling){
    m_createInfo.pMultisampleState = multisampling;
}

void GP::setDepthStencilState(VkPipelineDepthStencilStateCreateInfo* depthStencil){
    m_createInfo.pDepthStencilState = depthStencil;
}

void GP::setColorBlendState(BlendState _state){

    m_blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_blendState.logicOpEnable = VK_FALSE;
    m_blendState.logicOp = VK_LOGIC_OP_COPY;
    m_blendState.attachmentCount = 1;
    m_blendState.blendConstants[0] = 0.0f;
    m_blendState.blendConstants[1] = 0.0f;
    m_blendState.blendConstants[2] = 0.0f;
    m_blendState.blendConstants[3] = 0.0f;
    switch (_state) {
        case BlendState::Default :{
            m_blendAttachs =
                    {
                            {
                                    VK_FALSE,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ZERO,
                                    VK_BLEND_OP_ADD,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ZERO,
                                    VK_BLEND_OP_ADD,
                                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                            }
                    };
            break;
        }
        case BlendState::CPU_Particle :{
            m_blendAttachs =
                    {
                            {
                                    VK_TRUE,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                    VK_BLEND_OP_ADD,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ZERO,
                                    VK_BLEND_OP_ADD,
                                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                            }
                     };
            break;
        }
        case BlendState::Particle :{
            m_blendAttachs =
                    {
                            {
                                    VK_TRUE,
                                    VK_BLEND_FACTOR_SRC_ALPHA,
                                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                    VK_BLEND_OP_ADD,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ZERO,
                                    VK_BLEND_OP_ADD,
                                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                            }
                     };
            break;
        }
        case BlendState::Capture:{
            m_blendAttachs =
                    {
                            {
                                    VK_FALSE,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ZERO,
                                    VK_BLEND_OP_ADD,
                                    VK_BLEND_FACTOR_ONE,
                                    VK_BLEND_FACTOR_ZERO,
                                    VK_BLEND_OP_ADD,
                                    VK_COLOR_COMPONENT_R_BIT
                            }
                    };
            break;
        }
        case BlendState::GPass:{
            m_blendAttachs = {
                    tool::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
                    tool::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
                    tool::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
                    tool::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
            };
            m_blendState.attachmentCount = m_blendAttachs.size();
            break;
        }
        default :{
            throw std::runtime_error("chose blend_state error!");
        }
    }
    m_blendState.pAttachments = m_blendAttachs.data();
    m_createInfo.pColorBlendState = &m_blendState;
}

void GP::setDynamicState(VkPipelineDynamicStateCreateInfo* pDynamicState){
    m_createInfo.pDynamicState = pDynamicState;
}
/*
* 配置管线布局
*/
void GP::setPipelineLayout(VkPipelineLayout pipelineLayout){
    m_createInfo.layout = pipelineLayout;
}

void GP::setRenderPass(VkRenderPass renderPass){
    m_createInfo.renderPass = renderPass;
}

void GP::setSubpass(uint32_t subpass){
    m_createInfo.subpass = subpass;
}

void GP::setBasePipelineHandle(VkPipeline basePipelineHandle){
    m_createInfo.basePipelineHandle = basePipelineHandle;
}

void GP::setBasePipelineIndex(int32_t basePipelineIndex){
    m_createInfo.basePipelineIndex = basePipelineIndex;
}

GP::GP() {
    m_createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    m_defaultViewport.x = 0.0f;
    m_defaultViewport.y = 0.0f;
    m_defaultViewport.width = (float)getViewPort().view.width;
    m_defaultViewport.height = (float)getViewPort().view.height;

//    m_defaultViewport.width = (float)SingleRender.getExtent2D().width;
//    m_defaultViewport.height = (float)SingleRender.getExtent2D().height;
    m_defaultViewport.minDepth = 0.0f;
    m_defaultViewport.maxDepth = 1.0f;

    m_defaultScissor.offset = getScissor().offset;
    m_defaultScissor.extent = getScissor().extent;
//    m_defaultScissor.offset = {0, 0};
//    m_defaultScissor.extent = SingleRender.getExtent2D();

    m_defaultViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_defaultViewportStateCreateInfo.viewportCount = 1;
    m_defaultViewportStateCreateInfo.pViewports = &m_defaultViewport;
    m_defaultViewportStateCreateInfo.scissorCount = 1;
    m_defaultViewportStateCreateInfo.pScissors = &m_defaultScissor;
}

//GP* GP::createGP(const std::vector<Input_Type>& _inputType,
//                 const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
//                 AssemState _assemState,
//                 RasterState _rasterState,
//                 MultisampleState _multisampleState,
//                 DepthStencilState _depthStencilState,
//                 BlendState _blendState,
//                 const std::vector<Sl_id>& _sl_id,
//                 uint32_t _pass,
//                 bool _off
//){
//    auto gp = new GP;
//    gp->initGP(_inputType,
//               _shader,
//               _assemState,
//               _rasterState,
//               _multisampleState,
//               _depthStencilState,
//               _blendState,
//               _sl_id,
//               _pass,
//               _off);
//    return gp;
//}

GP* GP::createGP(const std::vector<Input_Type>& _inputType,
                    const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                    AssemState _assemState,
                    RasterState _rasterState,
                    MultisampleState _multisampleState,
                    DepthStencilState _depthStencilState,
                    BlendState _blendState,
                    const std::vector<Sl_id>& _sl_id,
                    RenderPassType _passType,
                    uint32_t _pass
){
    auto gp = new GP;
    gp->initGP(_inputType,
               _shader,
               _assemState,
               _rasterState,
               _multisampleState,
               _depthStencilState,
               _blendState,
               _sl_id,
               _pass,
               _passType);
    return gp;
}

void GP::initGP(const std::vector<Input_Type>& _inputType,
                const std::vector<VkPipelineShaderStageCreateInfo>& _shader,
                AssemState _assemState,
                RasterState _rasterState,
                MultisampleState _multisampleState,
                DepthStencilState _depthStencilState,
                BlendState _blendState,
                const std::vector<Sl_id>& _sl_id,
                uint32_t _pass,
                RenderPassType _passType)
{
    /* ShaderStageCreateInfo*/
    m_createInfo.stageCount = _shader.size();
    m_createInfo.pStages = _shader.data();

    /*VertexInputStateCreateInfo*/
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;

    bindingDescriptions.reserve(_inputType.size());
    for(auto& input : _inputType)
    {
        bindingDescriptions.push_back(getBindingDescription(input));
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = getAttributeDescriptions(_inputType[0]);

    vertexInputInfo.vertexBindingDescriptionCount = _inputType[0] == Input_Type::NONE? 0:_inputType.size();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    setVertexInputInfo(&vertexInputInfo);

    /*InputAssemblyStateCreateInfo*/
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = getAssemblyState(_assemState);
    setAssemblyState(&inputAssembly);

    /*ViewportStateCreateInfo*/
    setViewportState(&m_defaultViewportStateCreateInfo);

    /*RasterizationStateCreateInfo*/
    VkPipelineRasterizationStateCreateInfo rasterizer = getRasterState(_rasterState);
    setRasterizationState(&rasterizer);

    /*MultisampleStateCreateInfo*/
    VkPipelineMultisampleStateCreateInfo multisampling = getMultisampleState(_multisampleState);
    setMultisampleState(&multisampling);

    /*DepthStencilStateCreateInfo*/
    VkPipelineDepthStencilStateCreateInfo depthStencil = getDepthStencilState(_depthStencilState);
    setDepthStencilState(&depthStencil);

    /*ColorBlendStateCreateInfo*/
    setColorBlendState(_blendState);


    std::vector<VkDescriptorSetLayout> setLayout;
    setLayout.reserve(_sl_id.size());
    for(auto &i: _sl_id){
        setLayout.push_back(SlbPool::getPool().getSL(i));
    }

    /*PipelineLayoutCreateInfo*/
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            nullptr,
            0,
            uint32_t(setLayout.size()),
            setLayout.data(),
            0,
            nullptr};

    if (vkCreatePipelineLayout(SingleDevice(),
                               &pipelineLayoutInfo,
                               nullptr,
                               &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    setPipelineLayout(m_pipelineLayout);

    std::vector<VkDynamicState> dynamicStateEnables = {
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_VIEWPORT
    };
    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = VK_NULL_HANDLE;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStateEnables.data();

    setDynamicState(&dynamicState);

    switch (_passType) {
        case RenderPassType::Default:{
            setRenderPass(Shatter::render::ShatterRender::getRender().getDefaultRenderPass());
            break;
        }
        case RenderPassType::OffScreen:{
            setRenderPass(SingleOffScreen.m_pass);
            break;
        }
        case RenderPassType::CascadeShadow:{
            setRenderPass(SingleCascade.depthPass());
            break;
        }
        case RenderPassType::Capture:{
            setRenderPass(Shatter::render::ShatterRender::getRender().getCaptureRenderPass());
            break;
        }
        case RenderPassType::Present:{
            setRenderPass(Shatter::render::ShatterRender::getRender().getPresentRenderPass());
            break;
        }
        case RenderPassType::Color:{
            setRenderPass(Shatter::render::ShatterRender::getRender().getColorRenderPass());
            break;
        }
        default:
        {
            WARNING(RenderPass Type error!)
            break;
        }
    }
    setSubpass(_pass);
    setBasePipelineHandle(VK_NULL_HANDLE);
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(SingleDevice(),
                                              VK_NULL_HANDLE,
                                              1,
                                              &m_createInfo,
                                              nullptr,
                                              &m_pipeline));
}








