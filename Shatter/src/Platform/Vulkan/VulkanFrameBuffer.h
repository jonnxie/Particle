//
// Created by AnWell on 2022/2/20.
//

#ifndef MAIN_VULKANFRAMEBUFFER_H
#define MAIN_VULKANFRAMEBUFFER_H

#include <vulkan/vulkan.h>
#include "Engine/Renderer/FrameBuffer.h"
#include "Engine/Object/device.h"
#include "Transfer.h"
#include "Engine/Renderer/FormatFilter.hpp"

struct VkAttachment{
    VkImage                 image;
    VkImageView             imageView;
    VkDeviceMemory          memory;
    VkFormat                format;
    VkSampler               sampler;
    VkDescriptorImageInfo   descriptor;
};

VkImageAspectFlags getAspect(VkImageUsageFlags _usage);

class VulkanFrameBuffer : public FrameBuffer{
public:
    explicit VulkanFrameBuffer(FrameBufferSpecification _spec);
    ~VulkanFrameBuffer() override = default;
    void init();
    void resize(uint32_t _width,uint32_t _height) override;
    void release() override;
    void releaseCaptureVals() override;
    auto capture(uint32_t _xCoordinate, uint32_t _yCoordinate, int _attachmentIndex) {
        auto format = m_attachments[_attachmentIndex].format;
        void* mapped;
        switch (format) {
            case VK_FORMAT_R32_UINT:{
                VkBuffer buffer;
                VkDeviceMemory memory;
                if (m_captureVals.count(VK_FORMAT_R32_UINT) == 0) {
                    VkDeviceSize size = 4;
                    auto bufferInfo = tool::bufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);

                    vkCreateBuffer(SingleDevice(), &bufferInfo, VK_NULL_HANDLE, &buffer);

                    VkMemoryAllocateInfo memAlloc = tool::memoryAllocateInfo();
                    VkMemoryRequirements memReqs;

                    vkGetBufferMemoryRequirements(SingleDevice(), buffer, &memReqs);
                    memAlloc.allocationSize = memReqs.size;
                    memAlloc.memoryTypeIndex = SingleDevice.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
                    VK_CHECK_RESULT(vkAllocateMemory(SingleDevice(), &memAlloc, nullptr, &memory));
                    VK_CHECK_RESULT(vkBindBufferMemory(SingleDevice(), buffer, memory, 0))
                    m_captureVals[VK_FORMAT_R32_UINT] = std::make_pair(buffer, memory);
                }

                auto cmd = beginSingleTransCommandBuffer();
                VkBufferImageCopy bufferCopyRegion = {};
                bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                bufferCopyRegion.imageSubresource.layerCount = 1;
                bufferCopyRegion.imageOffset.x = _xCoordinate;
                bufferCopyRegion.imageOffset.y = _yCoordinate;
                bufferCopyRegion.imageOffset.z = 0;
                bufferCopyRegion.imageExtent.width = 1;
                bufferCopyRegion.imageExtent.height = 1;
                bufferCopyRegion.imageExtent.depth = 1;

                tool::setImageLayout(
                        cmd,
                        m_attachments[_attachmentIndex].image,
                        getAspect(m_spec.formats[_attachmentIndex].usage),
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                vkCmdCopyImageToBuffer(cmd,
                                       m_attachments[_attachmentIndex].image,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       m_captureVals[VK_FORMAT_R32_UINT].first,
                                       1,
                                       &bufferCopyRegion);

                tool::setImageLayout(
                        cmd,
                        m_attachments[_attachmentIndex].image,
                        getAspect(m_spec.formats[_attachmentIndex].usage),
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

                endSingleTransCommandBuffer(cmd);

                vkMapMemory(SingleDevice(), m_captureVals[VK_FORMAT_R32_UINT].second, 0, 4, 0, &mapped);
                uint32_t result = *((uint32_t*)mapped);
                vkUnmapMemory(SingleDevice(), m_captureVals[VK_FORMAT_R32_UINT].second);
                return result;
            }
            default:{
                std::cout << "No Such Attachment Type: [1]" << format << std::endl;
                break;
            }
        }
    };
    [[nodiscard]] VkFramebuffer get() const {
        return m_frame_buffer;
    }
public:
    std::unordered_map<VkFormat, std::pair<VkBuffer, VkDeviceMemory>> m_captureVals;
    VkFramebuffer                   m_frame_buffer{};
    std::vector<VkAttachment>       m_attachments{};
    bool                            m_released = false;
};


#endif //MAIN_VULKANFRAMEBUFFER_H
