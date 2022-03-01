//
// Created by AnWell on 2022/2/20.
//
#include "precompiledhead.h"
#include "VulkanFrameBuffer.h"
#include "VulkanFormatFilter.h"
#include <utility>
#include "Engine/Object/device.h"

void VulkanFrameBuffer::init() {
    auto device = SingleDevice();
    for(size_t index = 0; index < m_spec.formats.size(); index++)
    {
        m_attachments[index].format = getFormat(m_spec.formats[index].format);
        VkImageCreateInfo image = tool::imageCreateInfo();
        image.imageType = VK_IMAGE_TYPE_2D;
        image.format = m_attachments[index].format;
        image.extent.width = m_spec.Width;
        image.extent.height = m_spec.Height;
        image.extent.depth = 1;
        image.mipLevels = 1;
        image.arrayLayers = 1;
        image.samples = VkSampleCountFlagBits(1 << m_spec.Samples);
        image.tiling = VK_IMAGE_TILING_OPTIMAL;
        image.usage = getUsage(m_spec.formats[index].usage) | VK_IMAGE_USAGE_SAMPLED_BIT;

        VkMemoryAllocateInfo memAlloc = tool::memoryAllocateInfo();
        VkMemoryRequirements memReqs;

        VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &m_attachments[index].image));
        vkGetImageMemoryRequirements(device, m_attachments[index].image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = SingleDevice.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &m_attachments[index].memory));
        VK_CHECK_RESULT(vkBindImageMemory(device, m_attachments[index].image, m_attachments[index].memory, 0));

        VkImageViewCreateInfo colorImageView = tool::imageViewCreateInfo();
        colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorImageView.format = m_attachments[index].format;
        colorImageView.subresourceRange = {};
        colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageView.subresourceRange.baseMipLevel = 0;
        colorImageView.subresourceRange.levelCount = 1;
        colorImageView.subresourceRange.baseArrayLayer = 0;
        colorImageView.subresourceRange.layerCount = 1;
        colorImageView.image = m_attachments[index].image;
        VK_CHECK_RESULT(vkCreateImageView(device, &colorImageView, nullptr, &m_attachments[index].imageView));

        // Create sampler to sample from the attachment in the fragment shader
        VkSamplerCreateInfo samplerInfo = tool::samplerCreateInfo();
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &m_attachments[index].sampler));

        m_attachments[index].descriptor = {m_attachments[index].sampler, m_attachments[index].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    std::vector<VkImageView> attachments{};
    for(auto & a: m_attachments)
    {
        attachments.emplace_back(a.imageView);
    }

    VkFramebufferCreateInfo fbufCreateInfo = tool::framebufferCreateInfo();
    fbufCreateInfo.renderPass = m_renderPass;
    fbufCreateInfo.attachmentCount = attachments.size();
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.width = m_spec.Width;
    fbufCreateInfo.height = m_spec.Height;
    fbufCreateInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &m_frame_buffer));

}

void VulkanFrameBuffer::resize(uint32_t _width, uint32_t _height) {
}

void VulkanFrameBuffer::release() {
}

VulkanFrameBuffer::VulkanFrameBuffer(FrameBufferSpecification _spec,VkRenderPass _renderPass)
        : FrameBuffer(std::move(_spec)),m_renderPass(_renderPass) {
    init();
}

