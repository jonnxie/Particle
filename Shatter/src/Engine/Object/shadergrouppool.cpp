//
// Created by jonnxie on 2021/10/5.
//
#include "precompiledhead.h"

#include "shadergrouppool.h"


//
// Created by maybe on 2021/6/6.
//

#include "shaderpool.h"
#include "Engine/Render/shatter_render_include.h"
#include "Engine/Item/shatter_item.h"
#include "device.h"
#include <mutex>


using namespace Shatter::render;

static bool if_created = false;
static std::mutex pool_mutex;

ShaderGroupPool &ShaderGroupPool::getPool() {
    static ShaderGroupPool pool;
    std::lock_guard<std::mutex> guard_mutex(pool_mutex);
    if(!if_created){
        pool.init();
        if_created = true;
    }
    return pool;
}

void ShaderGroupPool::init() {

    {
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

        {
            VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
            shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroup.generalShader = 0;
            shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
            shaderGroups.push_back(shaderGroup);
        }

        {
            VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
            shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroup.generalShader = 1;
            shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
            shaderGroups.push_back(shaderGroup);
            // Second shader for shadows
            shaderGroup.generalShader = 2;
            shaderGroups.push_back(shaderGroup);
        }

        // Closest hit group
        {
            VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
            shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.closestHitShader = 3;
            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
            shaderGroups.push_back(shaderGroup);
        }

        m_map["RayTracingShadows"] = shaderGroups;
    }

}

void ShaderGroupPool::release() {
    for(auto & i: m_map){
    }
}





