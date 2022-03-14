//
// Created by maybe on 2021/6/6.
//
#include "precompiledhead.h"

#include "shaderpool.h"
#include "Engine/Renderer/shatter_render_include.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Object/device.h"
#include <mutex>
#include ConfigCatalog
#include <algorithm>

using namespace Shatter::render;

static bool if_created = false;
static std::mutex pool_mutex;

ShaderPool &ShaderPool::getPool() {
    static ShaderPool pool;
    std::lock_guard<std::mutex> guard_mutex(pool_mutex);
    if(!if_created){
        pool.init();
        if_created = true;
    }
    return pool;
}

void ShaderPool::createShaderStage(const Shader_id& _id,const std::string& _filename,ShaderType _type){
    checkMap(m_map);

    if(!file::fileExists(_filename)){
        throwFile(_filename);
    }
    auto shader_code = file::readFile(_filename);
    VkPipelineShaderStageCreateInfo shader_info;
    shader_info.pNext = VK_NULL_HANDLE;
    shader_info.flags = 0;
    shader_info.pSpecializationInfo = VK_NULL_HANDLE;
    auto m_shader_module = createShaderModule(shader_code, *ShatterRender::getRender().getDevice());

    shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    switch (_type) {
        case VertexShader:{
            shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        }
        case FragmentShader:{
            shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        }
        case ComputeShader:{
            shader_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        }
        case RayGen:{
            shader_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            break;
        }
        case RayMiss:{
            shader_info.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            break;
        }
        case RayClosest:{
            shader_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            break;
        }
        case TaskShader:{
            shader_info.stage = VK_SHADER_STAGE_TASK_BIT_NV;
            break;
        }
        case MeshShader:{
            shader_info.stage = VK_SHADER_STAGE_MESH_BIT_NV;
            break;
        }
        default: break;
    }
    shader_info.module = m_shader_module;
    shader_info.pName = "main";

    m_map[_id] = shader_info;
}

void ShaderPool::init() {
    createShaderStage("build_vs",tool::combineShader("g/gbuffer_vert.spv"),VertexShader);
    createShaderStage("build_fs",tool::combineShader("g/gbuffer_frag.spv"),FragmentShader);

    createShaderStage("agbasic_vs",tool::combineShader("agbasic/vert.spv"),VertexShader);
    createShaderStage("agbasic_fs",tool::combineShader("agbasic/frag.spv"),FragmentShader);

    createShaderStage("gplanet_vs",tool::combineShader("gplanet/vert.spv"),VertexShader);
    createShaderStage("gplanet_fs",tool::combineShader("gplanet/frag.spv"),FragmentShader);

    createShaderStage("transparent_vs",tool::combineShader("pass/transparent_vert.spv"),VertexShader);
    createShaderStage("transparent_fs",tool::combineShader("pass/transparent_frag.spv"),FragmentShader);

    createShaderStage("composition_vs",tool::combineShader("pass/composition_vert.spv"),VertexShader);
    createShaderStage("composition_fs",tool::combineShader("pass/composition_frag.spv"),FragmentShader);

    createShaderStage("shadowDepth_vs",tool::combineShader("cascade_shadow/depthpass_vert.spv"),VertexShader);
    createShaderStage("shadowDepth_fs",tool::combineShader("cascade_shadow/depthpass_frag.spv"),FragmentShader);

    createShaderStage("polyline_vs",tool::combineShader("polyline/pc_vert.spv"),VertexShader);
    createShaderStage("polyline_fs",tool::combineShader("polyline/pc_frag.spv"),FragmentShader);

    createShaderStage("simple_ms",tool::combineShader("TaskMesh/mesh.spv"),MeshShader);

    createShaderStage("particle_vs", tool::combineShader("point/vert.spv"),VertexShader);

    createShaderStage("planet_vs", tool::combineShader("planet/vert.spv"), VertexShader);
    createShaderStage("planet_fs", tool::combineShader("planet/frag.spv"), FragmentShader);

    createShaderStage("abasic_vs", tool::combineShader("abasic/vert.spv"), VertexShader);
    createShaderStage("abasic_fs", tool::combineShader("abasic/frag.spv"), FragmentShader);

    createShaderStage("aabb_vs", tool::combineShader("aabb/vert.spv"), VertexShader);
    createShaderStage("aabb_fs", tool::combineShader("aabb/frag.spv"), FragmentShader);
    /*
     * Skybox
     */
    {
        createShaderStage("skybox_vs",tool::combineShader("skybox/vert.spv"),VertexShader);
        createShaderStage("skybox_fs",tool::combineShader("skybox/frag.spv"),FragmentShader);
    }

    /*
     * imgui
     */
    {
        createShaderStage("gui_vs",tool::combineShader("gui/vert.spv"),VertexShader);
        createShaderStage("gui_fs",tool::combineShader("gui/frag.spv"),FragmentShader);
    }

    createShaderStage("quad_vs",tool::combineShader("quad/vert.spv"),VertexShader);
    createShaderStage("quad_fs",tool::combineShader("quad/frag.spv"),FragmentShader);

    createShaderStage("pbr_vs",tool::combineShader("pbrbasic/vert.spv"),VertexShader);
    createShaderStage("pbr_fs",tool::combineShader("pbrbasic/frag.spv"),FragmentShader);

    createShaderStage("animation_vs",tool::combineShader("animation/vert.spv"),VertexShader);
    createShaderStage("animation_fs",tool::combineShader("animation/frag.spv"),FragmentShader);

}

void ShaderPool::release() {
    for(auto & i: m_map){
        vkDestroyShaderModule(Device::getDevice()(), i.second.module, nullptr);
    }
}




