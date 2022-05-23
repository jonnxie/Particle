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
                 std::vector<Shader_id>{"skybox_vs",
                                                              "skybox_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::DepthWriteDisable,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency
        );
//
//        createGP("Quad",
//                 std::vector<Input_Type>{Input_Type::NONE},
//                 std::vector<Shader_id>{"quad_vs"),//                                                              "quad_fs")},//                 AssemState::Triangle_Strip,
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
//                 std::vector<Shader_id>{"pbr_vs"),//                                                              "pbr_fs")},//                 AssemState::Triangle_List,
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
                 std::vector<Shader_id>{"animation_vs",
                                                              "animation_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency);
    }

    /*
     * passes
     */
    {
        createGP("Build",
                 std::vector<Input_Type>{Input_Type::Building},
                 std::vector<Shader_id>{"build_vs",
                                                              "build_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("AGBasic",
                 std::vector<Input_Type>{Input_Type::GLTF},
                 std::vector<Shader_id>{"agbasic_vs",
                                                              "agbasic_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("I3DMInstanceBasic",
                 std::vector<Input_Type>{Input_Type::GLTFInstance, Input_Type::GLTF},
                 std::vector<Shader_id>{"aginstance_vs",
                                                              "aginstance_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("I3DMInstanceTexture",
                 std::vector<Input_Type>{Input_Type::GLTFInstance, Input_Type::GLTF},
                 std::vector<Shader_id>{"aginstance_vs",
                                                              "aginstanceTex_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("GPlanet",
                 std::vector<Input_Type>{Input_Type::Point3d},
                 std::vector<Shader_id>{"gplanet_vs",
                                                              "gplanet_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("GPlane",
                 std::vector<Input_Type>{Input_Type::Point3dUV},
                 std::vector<Shader_id>{"plane_vs",
                                                              "plane_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("GPlaneTex",
                 std::vector<Input_Type>{Input_Type::Point3dUV},
                 std::vector<Shader_id>{"plane_vs",
                                                              "planeTex_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("GCube",
                 std::vector<Input_Type>{Input_Type::Point3dNormalUV},
                 std::vector<Shader_id>{"cube_vs",
                                                              "cube_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("GCubeTex",
                 std::vector<Input_Type>{Input_Type::Point3dNormalUV},
                 std::vector<Shader_id>{"cube_vs",
                                                              "cubeTex_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("GSkin",
                 std::vector<Input_Type>{Input_Type::GLTF},
                 std::vector<Shader_id>{"gskin_vs",
                                                              "gskin_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG
        );

        createGP("GSkinInstance",
                 std::vector<Input_Type>{Input_Type::GLTFInstance, Input_Type::GLTF},
                 std::vector<Shader_id>{"gskinInstance_vs",
                                                              "gskinInstance_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::GPass,
                 RenderPassType::Color,
                 SubpassG);

        createGP("Composition",
                 std::vector<Input_Type>{Input_Type::NONE},
                 std::vector<Shader_id>{"composition_vs",
                                                              "composition_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::DepthWriteDisable,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassLight);

        createGP("Present",
                 std::vector<Input_Type>{Input_Type::NONE},
                 std::vector<Shader_id>{"present_vs",
                                                              "present_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Present,
                 SubpassG);

        createGP("Transparent",
                 std::vector<Input_Type>{Input_Type::TransparentGlass},
                 std::vector<Shader_id>{"transparent_vs",
                                                              "transparent_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency);

        createGP("EarthTex",
                 std::vector<Input_Type>{Input_Type::Point2dDouble},
                 std::vector<Shader_id>{"earth_vs",
                                        "earth_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency);

        createGP("Point",
                 std::vector<Input_Type>{Input_Type::Point3dColorSize},
                 std::vector<Shader_id>{"particle_vs",
                                                              "particle_fs"},
                 AssemState::Point_List,
                 RasterState::Point,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency
        );

        createGP("PointTex",
                 std::vector<Input_Type>{Input_Type::Point3dColorSize},
                 std::vector<Shader_id>{"particle_vs",
                                                              "particleTex_fs"},
                 AssemState::Point_List,
                 RasterState::Point,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency
        );

        createGP("Planet_Face",
                 std::vector<Input_Type>{Input_Type::Point3d},
                 std::vector<Shader_id>{"planet_vs",
                                                              "planet_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency
        );

        createGP("Planet_Line",
                 std::vector<Input_Type>{Input_Type::Point3d},
                 std::vector<Shader_id>{"planet_vs",
                                                              "planet_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleLine,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency
        );

        createGP("ABasic",
                 std::vector<Input_Type>{Input_Type::GLTF},
                 std::vector<Shader_id>{"abasic_vs",
                                                              "abasic_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency
        );

#ifdef SHATTER_GPU_CAPTURE
        createGP("AABBCapture",
                 std::vector<Input_Type>{Input_Type::Point3d},
                 std::vector<Shader_id>{"aabb_vs",
                                                              "aabb_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Capture,
                 RenderPassType::Capture,
                 0
        );
#endif

        createGP("Polyline",
                 std::vector<Input_Type>{Input_Type::Point3dColor},
                 std::vector<Shader_id>{"polyline_vs",
                                                              "polyline_fs"},
                 AssemState::Line_List,
                 RasterState::Polyline,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency
        );

        createGP("TriangleLine",
                 std::vector<Input_Type>{Input_Type::Point3dColor},
                 std::vector<Shader_id>{"polyline_vs",
                                                              "polyline_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleLine,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency);

        createGP("TriangleFace",
                 std::vector<Input_Type>{Input_Type::Point3dColor},
                 std::vector<Shader_id>{"polyline_vs",
                                                              "polyline_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
                 RenderPassType::Color,
                 SubpassTransparency);

        createGP("ShadowDepth",
                 std::vector<Input_Type>{Input_Type::Point3dUV},
                 std::vector<Shader_id>{"shadowDepth_vs",
                                                              "shadowDepth_fs"},
                 AssemState::Triangle_List,
                 RasterState::TriangleFace,
                 MultisampleState::Default,
                 DepthStencilState::Default,
                 BlendState::Default,
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
              const std::vector<Shader_id>& _shader,
              AssemState _assemState,
              RasterState _rasterState,
              MultisampleState _multisampleState,
              DepthStencilState _depthStencilState,
              BlendState _blendState,
              RenderPassType _passType,
              uint32_t _pass
){
    std::lock_guard<std::mutex> g_mutex(m_mutex);
    checkMap(m_map);
    m_map[_id] = GP::createGP(_inputType,_shader,_assemState,_rasterState,
                 _multisampleState,_depthStencilState,_blendState,_passType,_pass);
}

void PPool::createCP(const P_id &_id, const Shader_id &_shader) {
    std::lock_guard<std::mutex> g_mutex(m_mutex);
    checkMap(m_map);
    m_map[_id] = Pipeline::createCP(_shader);
}

void PPool::createRP(const P_id &_id, const std::vector<VkPipelineShaderStageCreateInfo> &_shader,
                     const std::vector<VkRayTracingShaderGroupCreateInfoKHR> &_group, const std::vector<Sl_id> &_sl_id,
                     uint32_t _recursion_depth) {
    std::lock_guard<std::mutex> g_mutex(m_mutex);
    checkMap(m_map);
    m_map[_id] = Pipeline::createRayTracingPipeline(_shader,_group,_sl_id,_recursion_depth);
}
