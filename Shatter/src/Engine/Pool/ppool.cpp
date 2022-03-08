//
// Created by maybe on 2021/6/5.
//
#include "precompiledhead.h"

#include "ppool.h"
#include "shaderpool.h"
#include "slbpool.h"
#include <mutex>
#include "shadergrouppool.h"
#include PCatalog

PPool &PPool::getPool() {
    static PPool pool;
    static std::mutex npLock;
    static bool npReady = false;
    std::lock_guard<std::mutex> guard_mutex(npLock);
    if(!npReady){
        pool.init();
        npReady = true;
    }
    return pool;
}

void PPool::init() {

        createGP("Skybox",
                 std::vector<Input_Type>{Input_Type::Point3dNormalUV},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("skybox_vs"),
                                                              ShaderPool::getPool().Get("skybox_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::DepthWriteDisable,
                 BlendState::Default,
                 std::vector<Sl_id>{"Default", "Camera", "Texture"},
                 RenderPassType::Default,
                 SubpassTransparency
        );
//
//        createGP("Quad",
//                 std::vector<Input_Type>{Input_Type::NONE},
//                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("quad_vs"),
//                                                              ShaderPool::getPool().Get("quad_fs")},
//                 AssemState::Triangle_Strip,
//                 RasterState::TriangleFace,
//                 MultisampleState::Default,
//                 DepthStencilState::Default,
//                 BlendState::Default,
//                 std::vector<Sl_id>{"OffScreen"}
//        );

    /*
     * physical render
     */
//    {
//        createGP("PhysicalBasicRender",
//                 std::vector<Input_Type>{Input_Type::Point3dNormal},
//                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("pbr_vs"),
//                                                              ShaderPool::getPool().Get("pbr_fs")},
//                 AssemState::Triangle_List,
//                 RasterState::TriangleFace,
//                 MultisampleState::Default,
//                 DepthStencilState::Default,
//                 BlendState::Default,
//                 std::vector<Sl_id>{"Default", "Camera", "PhysicalBasicRender"});
//    }

    /*
     *  Animation Rendering
     */
    {
        createGP("Animation",
                 std::vector<Input_Type>{Input_Type::Point3dUV},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("animation_vs"),
                                                              ShaderPool::getPool().Get("animation_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id>{"Default", "Camera", "AnimationUniform", "AnimationTexture"},
                 RenderPassType::Default,
                 SubpassTransparency);
    }

    /*
     * passes
     */
    {
        createGP("Build",
                 std::vector<Input_Type>{Input_Type::Building},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("build_vs"),
                                                              ShaderPool::getPool().Get("build_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 std::vector<Sl_id>{"Default", "Camera"},
                 RenderPassType::Default,
                 SubpassG);

        createGP("Composition",
                 std::vector<Input_Type>{Input_Type::NONE},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("composition_vs"),
                                                              ShaderPool::getPool().Get("composition_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::DepthWriteDisable,
                 BlendState::Default,
                 std::vector<Sl_id>{"gBuffer", "MultiLight"},
                 RenderPassType::Default,
                 SubpassLight);

        createGP("Transparent",
                 std::vector<Input_Type>{Input_Type::TransparentGlass},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("transparent_vs"),
                                                              ShaderPool::getPool().Get("transparent_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id>{"Default", "Camera", "TransparentInput"},
                 RenderPassType::Default,
                 SubpassTransparency);

        createGP("Point",
                 std::vector<Input_Type>{Input_Type::Point3dColorSize},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("particle_vs"),
                                                              ShaderPool::getPool().Get("polyline_fs")},
                 AssemState::Point_List,
                 RasterState::Point,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id> {"Default","Camera"},
                 RenderPassType::Default,
                 SubpassTransparency
        );

        createGP("Planet_Face",
                 std::vector<Input_Type>{Input_Type::Point3d},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("planet_vs"),
                                                              ShaderPool::getPool().Get("planet_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id> {"Default","Camera", "Planet"},
                 RenderPassType::Default,
                 SubpassTransparency
        );

        createGP("Planet_Line",
                 std::vector<Input_Type>{Input_Type::Point3d},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("planet_vs"),
                                                              ShaderPool::getPool().Get("planet_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleLine,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id> {"Default","Camera", "Planet"},
                 RenderPassType::Default,
                 SubpassTransparency
        );

        createGP("ABasic",
                 std::vector<Input_Type>{Input_Type::GLTF},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("abasic_vs"),
                                                              ShaderPool::getPool().Get("abasic_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id> {"Default","Camera", "Planet"},
                 RenderPassType::Default,
                 SubpassTransparency
        );

        createGP("AABBCapture",
                 std::vector<Input_Type>{Input_Type::Point3d},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("aabb_vs"),
                                                              ShaderPool::getPool().Get("aabb_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Capture,
                 std::vector<Sl_id> {"Default","Camera", "CaptureVal"},
                 RenderPassType::Capture,
                 0
        );

        createGP("Polyline",
                 std::vector<Input_Type>{Input_Type::Point3dColor},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("polyline_vs"),
                                                              ShaderPool::getPool().Get("polyline_fs")},
                 AssemState::Line_List,
                 RasterState::Polyline,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id> {"Default","Camera"},
                 RenderPassType::Default,
                 SubpassTransparency
        );

        createGP("TriangleLine",
                 std::vector<Input_Type>{Input_Type::Point3dColor},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("polyline_vs"),
                                                              ShaderPool::getPool().Get("polyline_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleLine,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id> {"Default","Camera"},
                 RenderPassType::Default,
                 SubpassTransparency);

        createGP("TriangleFace",
                 std::vector<Input_Type>{Input_Type::Point3dColor},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("polyline_vs"),
                                                              ShaderPool::getPool().Get("polyline_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id> {"Default","Camera"},
                 RenderPassType::Default,
                 SubpassTransparency);

        createGP("ShadowDepth",
                 std::vector<Input_Type>{Input_Type::Point3dUV},
                 std::vector<VkPipelineShaderStageCreateInfo>{ShaderPool::getPool().Get("shadowDepth_vs"),
                                                              ShaderPool::getPool().Get("shadowDepth_fs")},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 std::vector<Sl_id>{"Default", "Cascade"},
                 RenderPassType::CascadeShadow,
                 0);

    }
}

void PPool::release() {
    for(auto& i : m_map){
        delete i.second;
    }
//    m_map.clear();
}

void PPool::createGP(const P_id & _id,
              const std::vector<Input_Type>& _inputType,
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
    std::lock_guard<std::mutex> g_mutex(m_mutex);
    checkMap(m_map);
    m_map[_id] = GP::createGP(_inputType,_shader,_assemState,_rasterState,
                 _multisampleState,_depthStencilState,_blendState,_sl_id,_passType,_pass);
}

void PPool::createCP(const P_id &_id, const VkPipelineShaderStageCreateInfo &_shader,
                     const std::vector<Sl_id> &_descriptorSetLayoutBinding) {
    std::lock_guard<std::mutex> g_mutex(m_mutex);
    checkMap(m_map);
    m_map[_id] = Pipeline::createCP(_shader,_descriptorSetLayoutBinding);
}

void PPool::createRP(const P_id &_id, const std::vector<VkPipelineShaderStageCreateInfo> &_shader,
                     const std::vector<VkRayTracingShaderGroupCreateInfoKHR> &_group, const std::vector<Sl_id> &_sl_id,
                     uint32_t _recursion_depth) {
    std::lock_guard<std::mutex> g_mutex(m_mutex);
    checkMap(m_map);
    m_map[_id] = Pipeline::createRayTracingPipeline(_shader,_group,_sl_id,_recursion_depth);
}
