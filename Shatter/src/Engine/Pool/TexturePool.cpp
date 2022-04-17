//
// Created by jonnxie on 2022/4/16.
//

#include "TexturePool.h"
#include "Engine/Object/device.h"
#include <mutex>
#include "Engine/Buffer/shattertexture.h"
#include <sstream>
#include <iostream>
#include <cstdio>
#include <stb_image.h>
#include "Platform/Vulkan/Transfer.h"
#include "setpool.h"

TexturePool &TexturePool::getPool() {
    static TexturePool pool;
    static std::mutex lock;
    static bool inited = false;
    std::lock_guard<std::mutex> lockGuard(lock);
    if(!inited){
        inited = true;
        pool.init();
    }
    return pool;
}

void TexturePool::init() {
    addTexture("default", tool::combineTexture("default.png"), TextureType::Texture2DDefault);
}

void TexturePool::release() {
    for(auto& [key, tex]: m_map) {
        if(tex.sampler != VK_NULL_HANDLE) {
            vkDestroySampler(SingleDevice(), tex.sampler, nullptr);
        }
        if(tex.view != VK_NULL_HANDLE) {
            vkDestroyImageView(SingleDevice(), tex.view, nullptr);
        }
        if(tex.memory != VK_NULL_HANDLE) {
            vkFreeMemory(SingleDevice(), tex.memory, nullptr);
        }
        if(tex.image != VK_NULL_HANDLE) {
            vkDestroyImage(SingleDevice(), tex.image, nullptr);
        }
    }
}

void TexturePool::addTexture(const std::string& _key,
                             const std::string &_filename,
                             TextureType _type) {
    Texture texture;
    switch (_type) {
        case TextureType::Texture1D: {
            break;
        }
        case TextureType::Texture2DDefault: {
            create2DTexture( _key, _filename, texture);
            break;
        }
        case TextureType::Texture3D: {
            break;
        }
    }
    m_map[_key] = texture;
}

static void createBuffer(VkDeviceSize _size,
                  VkBufferUsageFlags _usage,
                  VkMemoryPropertyFlags _properties,
                  VkBuffer& _buffer,
                  VkDeviceMemory& _bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    std::array<uint32_t,2> queueFamilyIndices = {uint32_t(getIndices().computeFamily),uint32_t(getIndices().graphicsFamily)};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = VK_NULL_HANDLE;
    bufferInfo.size = _size;
    bufferInfo.usage = _usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(SingleDevice(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(SingleDevice(),
                                  _buffer,
                                  &memRequirements);

    auto memoryProperties = getVkPhysicalDeviceMemoryProperties();

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!memoryTypeFromProperties(memoryProperties,
                                 memRequirements.memoryTypeBits,
                                 _properties,
                                 &allocInfo.memoryTypeIndex)) {
        printf("Can not suitable memory type index");
    };

    if (vkAllocateMemory(SingleDevice(),
                         &allocInfo,
                         nullptr,
                         &_bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(SingleDevice(),
                       _buffer,
                       _bufferMemory,
                       0);
}

static void create2dImage(uint32_t _width,
                   uint32_t _height,
                   VkFormat _format,
                   VkImageTiling _tiling,
                   VkImageUsageFlags _usage,
                   VkMemoryPropertyFlags _properties,
                   VkImage& _image,
                   VkDeviceMemory& _imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = _width;
    imageInfo.extent.height = _height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = _format;
    imageInfo.tiling = _tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = _usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(SingleDevice(), &imageInfo, nullptr, &_image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(SingleDevice(), _image, &memRequirements);

    auto memoryProperties = getVkPhysicalDeviceMemoryProperties();

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!memoryTypeFromProperties(memoryProperties,
                                  memRequirements.memoryTypeBits,
                                  _properties,
                                  &allocInfo.memoryTypeIndex)) {
        printf("Can not suitable memory type index");
    };

    if (vkAllocateMemory(SingleDevice(), &allocInfo, nullptr, &_imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(SingleDevice(), _image, _imageMemory, 0);
}

static void setImageLayout(
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
    VkCommandBuffer commandBuffer = beginSingleCommandBuffer();

    VkImageMemoryBarrier imageMemoryBarrier {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            imageMemoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
            if (imageMemoryBarrier.srcAccessMask == 0)
            {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        }
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
            commandBuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

    endSingleCommandBuffer(commandBuffer);
}

static void setImageLayout(
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        uint32_t mipLevelCount = 1,
        uint32_t layerCount = 1) {
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevelCount;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = layerCount;
    setImageLayout(image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

static void transitionImageLayout(VkImage image,
                                           VkFormat format,
                                           VkImageLayout oldLayout,
                                           VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleCommandBuffer();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (::hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );

    endSingleCommandBuffer(commandBuffer);
}

static void copyBufferToImage(VkBuffer buffer,
                       VkImage image,
                       uint32_t width,
                       uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleCommandBuffer();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
            width,
            height,
            1
    };
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleCommandBuffer(commandBuffer);
}

static void create2dImageView(VkImage image, VkFormat format, VkImageView& view, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(SingleDevice(),
                          &viewInfo,
                          nullptr,
                          &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void createDefaultSampler(VkSampler& sampler){
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(SingleDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void TexturePool::create2DTexture(const std::string& _key,
                                  const std::string& _filename,
                                  Texture& _tex) {
    int texWidth, texHeight, texChannels;
    if (!file::fileExists(_filename)) {
        throwFile(_filename);
    }
    stbi_uc *pixels = stbi_load(_filename.c_str(),
                                &texWidth,
                                &texHeight,
                                &texChannels,
                                STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory);
    void *data;
    vkMapMemory(SingleDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(SingleDevice(), stagingBufferMemory);
    stbi_image_free(pixels);

    create2dImage(texWidth,
                  texHeight,
                  VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  _tex.image,
                  _tex.memory);

    setImageLayout(_tex.image,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(stagingBuffer,
                      _tex.image,
                      static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));

    setImageLayout(_tex.image,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    create2dImageView(_tex.image,
                      VK_FORMAT_R8G8B8A8_UNORM,
                      _tex.view,
                      VK_IMAGE_ASPECT_COLOR_BIT
                      );

    createDefaultSampler(_tex.sampler);

    vkDestroyBuffer(SingleDevice(), stagingBuffer, nullptr);
    vkFreeMemory(SingleDevice(), stagingBufferMemory, nullptr);

    SingleSetPool.AllocateDescriptorSets(_key,
                                         {"BaseTexture"},
                                         &_tex.set);
}


