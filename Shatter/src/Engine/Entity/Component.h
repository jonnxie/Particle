//
// Created by jonnxie on 2022/3/15.
//

#ifndef MAIN_COMPONENT_H
#define MAIN_COMPONENT_H

#include <glm.hpp>
#include <vulkan/vulkan.h>
#include "Engine/Core/BuildMacro.h"

class Component {

};

struct GPUCaptureComponent {
    glm::vec3       m_min{};
    glm::vec3       m_max{};
    uint32_t        m_model_index{};
#ifdef SHATTER_GRAPHICS_VULKAN
    VkDescriptorSet m_capture_set{};
#endif
    GPUCaptureComponent() = default;
    GPUCaptureComponent(const GPUCaptureComponent&) = default;
    GPUCaptureComponent(const glm::vec3& _min,const glm::vec3& _max)
            : m_min(_min), m_max(_max) {}
};

struct Transform{
    glm::mat4 transition;
    glm::mat4 rotate;
    glm::mat4 scale;
};

#endif //MAIN_COMPONENT_H
