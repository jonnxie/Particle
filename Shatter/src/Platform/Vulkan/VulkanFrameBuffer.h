//
// Created by AnWell on 2022/2/20.
//

#ifndef MAIN_VULKANFRAMEBUFFER_H
#define MAIN_VULKANFRAMEBUFFER_H

#include <vulkan/vulkan.h>
#include "Engine/Render/FrameBuffer.h"

struct VkAttachment{
    VkImage                 image;
    VkImageView             imageView;
    VkDeviceMemory          memory;
    VkFormat                format;
    VkSampler               sampler;
    VkDescriptorImageInfo   descriptor;
};


class VulkanFrameBuffer : public FrameBuffer{
public:
    explicit VulkanFrameBuffer(FrameBufferSpecification _spec);
    ~VulkanFrameBuffer() override = default;
    void init();
    void resize(uint32_t _width,uint32_t _height) override;
    void release() override;
public:
    VkFramebuffer                   m_frame_buffer{};
    std::vector<VkAttachment>       m_attachments{};
};


#endif //MAIN_VULKANFRAMEBUFFER_H
