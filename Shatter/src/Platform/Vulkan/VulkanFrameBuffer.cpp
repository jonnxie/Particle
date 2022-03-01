//
// Created by AnWell on 2022/2/20.
//
#include "precompiledhead.h"
#include "VulkanFrameBuffer.h"
#include "VulkanFormatFilter.h"
#include <utility>
#include "Engine/Object/device.h"

VkImageAspectFlags getAspect(VkImageUsageFlags _usage)
{
    VkFlags flags = 0x00000000;
    if(_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) flags |= VK_IMAGE_ASPECT_COLOR_BIT;

    if(_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) flags |= VK_IMAGE_ASPECT_DEPTH_BIT;

    return flags;
}

void VulkanFrameBuffer::init() {
    auto device = SingleDevice();
    m_attachments.resize(m_spec.formats.size());
    for(size_t index = 0; index < m_spec.formats.size(); index++)
    {
        m_attachments[index].format = m_spec.formats[index].format;
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
        image.usage = m_spec.formats[index].usage | VK_IMAGE_USAGE_SAMPLED_BIT;

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
        colorImageView.subresourceRange.aspectMask = getAspect(m_spec.formats[index].usage);
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

    VkFramebufferCreateInfo info = tool::framebufferCreateInfo();
    info.renderPass = m_spec.RenderPass;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();
    info.width = m_spec.Width;
    info.height = m_spec.Height;
    info.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(device, &info, nullptr, &m_frame_buffer));
}

void VulkanFrameBuffer::resize(uint32_t _width, uint32_t _height) {
    m_spec.Width = _width;
    m_spec.Height = _height;
    release();
    init();
}

void VulkanFrameBuffer::release() {
    auto device = SingleDevice();
    for(auto& a : m_attachments)
    {
        vkDestroyImageView(device, a.imageView, nullptr);
        vkDestroyImage(device, a.image,nullptr);
        vkFreeMemory(device, a.memory, nullptr);
        vkDestroySampler(device, a.sampler, nullptr);
    }

    vkDestroyFramebuffer(device, m_frame_buffer, nullptr);
}

VulkanFrameBuffer::VulkanFrameBuffer(FrameBufferSpecification _spec)
        : FrameBuffer(std::move(_spec)){
    init();
}

