//
// Created by maybe on 2021/6/7.
//
#include "precompiledhead.h"

#include "slbpool.h"
#include "Engine/Render/shatter_render_include.h"
#include "device.h"
#include <mutex>
using namespace Shatter::render;

static bool created = false;
static std::mutex pool_mutex;


SlbPool &SlbPool::getPool() {
    static SlbPool pool;
    std::lock_guard<std::mutex> guard_mutex(pool_mutex);
    if(!created){
        pool.init();
        created =true;
    }
    return pool;
}


void SlbPool::init(){

    m_map["Texture"] = std::vector<VkDescriptorSetLayoutBinding>{
            tool::descriptorSetLayoutBinding(
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                    0),
            VkDescriptorSetLayoutBinding{
                    1,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    1,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    nullptr
            }
    };

    /*
     * model matrix
     */
    m_map["Default"] = std::vector<VkDescriptorSetLayoutBinding>{
        VkDescriptorSetLayoutBinding{
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr}
    };

    /*
     * camera: view matrix and proj matrix
     */
    m_map["Camera"] = std::vector<VkDescriptorSetLayoutBinding>{
        VkDescriptorSetLayoutBinding{
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr}
    };

    /*
     * basic
     */
    {
        /*
         * offscreen rendering target used as shader resources
         */
        m_map["OffScreen"] =  std::vector<VkDescriptorSetLayoutBinding>{
                // Binding 0: Instance input data buffer
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0)
        };

        m_map["PhysicalBasicRender"] = std::vector<VkDescriptorSetLayoutBinding>{
                // Binding 0: Instance input data buffer
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        1),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        2),
        };
    }

    /*
     * animation
     */
    {
        m_map["AnimationUniform"] =  std::vector<VkDescriptorSetLayoutBinding>{
                // Binding 0: Instance input data buffer
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0)
        };

        m_map["AnimationTexture"] =  std::vector<VkDescriptorSetLayoutBinding>{
                // Binding 0: Instance input data buffer
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0)
        };
    }

    /*
     * multiple pass
     */
    {
        m_map["TransparentInput"] = std::vector<VkDescriptorSetLayoutBinding>{
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        1),
        };

        m_map["gBuffer"] = std::vector<VkDescriptorSetLayoutBinding>{
                // Binding 0: Instance input data buffer
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        1),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        2),
        };

        m_map["Cascade"] = std::vector<VkDescriptorSetLayoutBinding>{
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        1),
        };

        m_map["CompositeAttachment"] = std::vector<VkDescriptorSetLayoutBinding>{
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        1),
        };

        m_map["MultiLight"] = std::vector<VkDescriptorSetLayoutBinding>{
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        1),
                tool::descriptorSetLayoutBinding(
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        2),
        };
    }

    for(auto &i:m_map){
        std::vector<VkDescriptorSetLayoutBinding> bindings = i.second;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout sl;

        if (vkCreateDescriptorSetLayout(*(ShatterRender::getRender().getDevice()),
                                        &layoutInfo,
                                        nullptr,
                                        &sl) != VK_SUCCESS) {
            throw std::runtime_error("failed to create" + i.first + " descriptor set layout!");
        }
        m_sl_map[i.first] = sl;
    }
}

void SlbPool::insertSL(const Sl_id &_id, std::vector<VkDescriptorSetLayoutBinding> _bindingVec) {
    m_map[_id] = std::move(_bindingVec);
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = _bindingVec.size();
    layoutInfo.pBindings = _bindingVec.data();

    VkDescriptorSetLayout sl;

    if (vkCreateDescriptorSetLayout(*(ShatterRender::getRender().getDevice()),
                                    &layoutInfo,
                                    nullptr,
                                    &sl) != VK_SUCCESS) {
        throw std::runtime_error("failed to create" + _id + " descriptor set layout!");
    }
    m_sl_map[_id] = sl;
}

VkDescriptorSetLayout SlbPool::getSL(const Sl_id& _id) {
    return m_sl_map[_id];
}

void SlbPool::setSL(const Sl_id &_id, VkDescriptorSetLayout _sl) {
    checkMap(m_sl_map);
    m_sl_map[_id] = _sl;
}

void SlbPool::release() {
    for(auto &i:m_sl_map){
        vkDestroyDescriptorSetLayout(Device::getDevice()(), i.second, nullptr);
    }
}
/*
 *         BPool::getPool().createStorageBuffer("marching_cube_density",sizeof(int)*m_cube_vec.size(),m_cube_vec.data());
 */

