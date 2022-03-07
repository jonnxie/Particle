//
// Created by AnWell on 2022/2/20.
//

#ifndef MAIN_VULKANFRAMEBUFFER_H
#define MAIN_VULKANFRAMEBUFFER_H

#include <vulkan/vulkan.h>
#include "Engine/Renderer/FrameBuffer.h"
#include "Engine/Object/device.h"

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
    auto capture(uint32_t _xCoordinate, uint32_t _yCoordinate, int _attachmentIndex){
        auto format = m_attachments[_attachmentIndex].format;
        uint32_t index = _yCoordinate * m_spec.Width + _xCoordinate;
        void* mapped;
        switch (format) {
            case VK_FORMAT_R32_UINT:{
                index *= 4;
                vkMapMemory(SingleDevice(), m_attachments[_attachmentIndex].memory, index, 4, 0, &mapped);
                uint32_t result = *(uint32_t*)mapped;
                vkUnmapMemory(SingleDevice(), m_attachments[_attachmentIndex].memory);
                return result;
            }
            default:{
                std::cout << "No Such Attachment Type: [1]" << format << std::endl;
                break;
            }
        }
    }
public:
    VkFramebuffer                   m_frame_buffer{};
    std::vector<VkAttachment>       m_attachments{};
    bool                            m_released = false;
};


#endif //MAIN_VULKANFRAMEBUFFER_H
