
/*
* Vulkan glTF model and texture loading class based on tinyglTF (https://github.com/syoyo/tinygltf)
*
* Copyright (C) 2018 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include "precompiledhead.h"


#ifndef TINYGLTF_IMPLEMENTATION
    #define TINYGLTF_IMPLEMENTATION
#endif

#ifndef TINYGLTF_NO_STB_IMAGE_WRITE
    #define TINYGLTF_NO_STB_IMAGE_WRITE
#endif

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "VulkanglTFModels.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Renderer/shatter_render_include.h"
#include <execution>

VkDescriptorSetLayout vkglTF::descriptorSetLayoutImage = VK_NULL_HANDLE;
VkDescriptorSetLayout vkglTF::descriptorSetLayoutUbo = VK_NULL_HANDLE;
VkMemoryPropertyFlags vkglTF::memoryPropertyFlags = 0;
uint32_t vkglTF::descriptorBindingFlags = vkglTF::DescriptorBindingFlags::ImageBaseColor;

/*
	We use a custom image loading function with tinyglTF, so we can do custom stuff loading ktx textures
*/
bool loadImageDataFunc(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData)
{
	// KTX files will be handled by our own code
	if (image->uri.find_last_of(".") != std::string::npos) {
		if (image->uri.substr(image->uri.find_last_of(".") + 1) == "ktx") {
			return true;
		}
	}

	return tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
}

bool loadImageDataFuncEmpty(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData) 
{
	// This function will be used for samples that don't require images to be loaded
	return true;
}


/*
	glTF texture loading class
*/

void vkglTF::Texture::updateDescriptor()
{
	descriptor.sampler = sampler;
	descriptor.imageView = view;
	descriptor.imageLayout = imageLayout;
}

void vkglTF::Texture::destroy()
{
	if (device)
	{
		vkDestroyImageView(device->logicalDevice, view, nullptr);
		vkDestroyImage(device->logicalDevice, image, nullptr);
		vkFreeMemory(device->logicalDevice, deviceMemory, nullptr);
		vkDestroySampler(device->logicalDevice, sampler, nullptr);
	}
}

void vkglTF::Texture::fromGlTfImage(tinygltf::Image &gltfImage, std::string path, Device *device, VkQueue copyQueue) {
    this->device = device;

    bool isKtx = false;
    // Image points to an external ktx file
    if (gltfImage.uri.find_last_of(".") != std::string::npos) {
        if (gltfImage.uri.substr(gltfImage.uri.find_last_of(".") + 1) == "ktx") {
            isKtx = true;
        }
    }

    VkFormat format;

    if (!isKtx) {
        // Texture was loaded using STB_Image

        unsigned char* buffer = nullptr;
        VkDeviceSize bufferSize = 0;
        bool deleteBuffer = false;
        if (gltfImage.component == 3) {
            // Most devices don't support RGB only on Vulkan so convert if necessary
            // TODO: Check actual format support and transform only if required
            bufferSize = gltfImage.width * gltfImage.height * 4;
            buffer = new unsigned char[bufferSize];
            unsigned char* rgba = buffer;
            unsigned char* rgb = &gltfImage.image[0];
            for (size_t i = 0; i < gltfImage.width * gltfImage.height; ++i) {
                for (int32_t j = 0; j < 3; ++j) {
                    rgba[j] = rgb[j];
                }
                rgba += 4;
                rgb += 3;
            }
            deleteBuffer = true;
        }
        else {
            buffer = &gltfImage.image[0];
            bufferSize = gltfImage.image.size();
        }

        format = VK_FORMAT_R8G8B8A8_UNORM;

        VkFormatProperties formatProperties;

        width = gltfImage.width;
        height = gltfImage.height;
        mipLevels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);

        vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProperties);
        assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
        assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs{};

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));
        vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
        VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

        uint8_t* data;
        VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
        memcpy(data, buffer, bufferSize);
        vkUnmapMemory(device->logicalDevice, stagingMemory);

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { width, height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &image));
        vkGetImageMemoryRequirements(device->logicalDevice, image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &deviceMemory));
        VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));

//        VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        VkCommandBuffer copyCmd = Shatter::render::ShatterRender::getRender().beginSingleTimeCommands();

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }

        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = 0;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = width;
        bufferCopyRegion.imageExtent.height = height;
        bufferCopyRegion.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }

        Shatter::render::ShatterRender::getRender().endSingleTimeCommands(copyCmd);
//        device->flushCommandBuffer(copyCmd, copyQueue, true);

        vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
        vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);

        // Generate the mip chain (glTF uses jpg and png, so we need to create this manually)
        VkCommandBuffer blitCmd = Shatter::render::ShatterRender::getRender().beginSingleTimeCommands();
        for (uint32_t i = 1; i < mipLevels; i++) {
            VkImageBlit imageBlit{};

            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.srcSubresource.mipLevel = i - 1;
            imageBlit.srcOffsets[1].x = int32_t(width >> (i - 1));
            imageBlit.srcOffsets[1].y = int32_t(height >> (i - 1));
            imageBlit.srcOffsets[1].z = 1;

            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.layerCount = 1;
            imageBlit.dstSubresource.mipLevel = i;
            imageBlit.dstOffsets[1].x = int32_t(width >> i);
            imageBlit.dstOffsets[1].y = int32_t(height >> i);
            imageBlit.dstOffsets[1].z = 1;

            VkImageSubresourceRange mipSubRange = {};
            mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            mipSubRange.baseMipLevel = i;
            mipSubRange.levelCount = 1;
            mipSubRange.layerCount = 1;

            {
                VkImageMemoryBarrier imageMemoryBarrier{};
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = 0;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.image = image;
                imageMemoryBarrier.subresourceRange = mipSubRange;
                vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }

            vkCmdBlitImage(blitCmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

            {
                VkImageMemoryBarrier imageMemoryBarrier{};
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageMemoryBarrier.image = image;
                imageMemoryBarrier.subresourceRange = mipSubRange;
                vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
        }

        subresourceRange.levelCount = mipLevels;
        imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }

        if (deleteBuffer) {
            delete[] buffer;
        }

        Shatter::render::ShatterRender::getRender().endSingleTimeCommands(blitCmd);
    }
    else {
        // Texture is stored in an external ktx file
        std::string filename = path + "/" + gltfImage.uri;

        ktxTexture* ktxTexture;

        ktxResult result = KTX_SUCCESS;
#if defined(__ANDROID__)
        AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
		if (!asset) {
			vks::tools::exitFatal("Could not load texture from " + filename + "\n\nThe file may be part of the additional asset pack.\n\nRun \"download_assets.py\" in the repository root to download the latest version.", -1);
		}
		size_t size = AAsset_getLength(asset);
		assert(size > 0);
		ktx_uint8_t* textureData = new ktx_uint8_t[size];
		AAsset_read(asset, textureData, size);
		AAsset_close(asset);
		result = ktxTexture_CreateFromMemory(textureData, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
		delete[] textureData;
#else
        if (!file::fileExists(filename)) {
            WARNING("no such file!");
        }
        result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
#endif
        assert(result == KTX_SUCCESS);

        this->device = device;
        width = ktxTexture->baseWidth;
        height = ktxTexture->baseHeight;
        mipLevels = ktxTexture->numLevels;

        ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);
        // @todo: Use ktxTexture_GetVkFormat(ktxTexture)
        format = VK_FORMAT_R8G8B8A8_UNORM;

        // Get device properties for the requested texture format
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProperties);

        VkCommandBuffer copyCmd = Shatter::render::ShatterRender::getRender().beginSingleTimeCommands();;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        VkBufferCreateInfo bufferCreateInfo = tool::bufferCreateInfo();
        bufferCreateInfo.size = ktxTextureSize;
        // This buffer is used as a transfer source for the buffer copy
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

        VkMemoryAllocateInfo memAllocInfo = tool::memoryAllocateInfo();
        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
        VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

        uint8_t* data;
        VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
        memcpy(data, ktxTextureData, ktxTextureSize);
        vkUnmapMemory(device->logicalDevice, stagingMemory);

        std::vector<VkBufferImageCopy> bufferCopyRegions;
        for (uint32_t i = 0; i < mipLevels; i++)
        {
            ktx_size_t offset;
            KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
            assert(result == KTX_SUCCESS);
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = i;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
            bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = offset;
            bufferCopyRegions.push_back(bufferCopyRegion);
        }

        // Create optimal tiled target image
        VkImageCreateInfo imageCreateInfo = tool::imageCreateInfo();
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { width, height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &image));

        vkGetImageMemoryRequirements(device->logicalDevice, image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &deviceMemory));
        VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = mipLevels;
        subresourceRange.layerCount = 1;

        tool::setLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        subresourceRange);
        vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());
        tool::setLayout(copyCmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        subresourceRange);
        Shatter::render::ShatterRender::getRender().endSingleTimeCommands(copyCmd);
//        device->flushCommandBuffer(copyCmd, copyQueue);
        this->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
        vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);

        ktxTexture_Destroy(ktxTexture);
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.maxAnisotropy = 1.0;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxLod = (float)mipLevels;
    samplerInfo.maxAnisotropy = 8.0f;
    samplerInfo.anisotropyEnable = VK_TRUE;
    VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerInfo, nullptr, &sampler));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = mipLevels;
    VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewInfo, nullptr, &view));

    descriptor.sampler = sampler;
    descriptor.imageView = view;
    descriptor.imageLayout = imageLayout;
}


/*
	glTF material
*/
void vkglTF::Material::createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device->logicalDevice, &descriptorSetAllocInfo, &descriptorSet));
	std::vector<VkDescriptorImageInfo> imageDescriptors{};
	std::vector<VkWriteDescriptorSet> writeDescriptorSets{};
	if (descriptorBindingFlags & DescriptorBindingFlags::ImageBaseColor) {
		imageDescriptors.push_back(baseColorTexture->descriptor);
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = static_cast<uint32_t>(writeDescriptorSets.size());
		writeDescriptorSet.pImageInfo = &baseColorTexture->descriptor;
		writeDescriptorSets.push_back(writeDescriptorSet);
	}
	if (normalTexture && descriptorBindingFlags & DescriptorBindingFlags::ImageNormalMap) {
		imageDescriptors.push_back(normalTexture->descriptor);
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = static_cast<uint32_t>(writeDescriptorSets.size());
		writeDescriptorSet.pImageInfo = &normalTexture->descriptor;
		writeDescriptorSets.push_back(writeDescriptorSet);
	}
	vkUpdateDescriptorSets(device->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}


/*
	glTF primitive
*/
void vkglTF::Primitive::setDimensions(glm::vec3 min, glm::vec3 max) {
	dimensions.min = min;
	dimensions.max = max;
	dimensions.size = max - min;
	dimensions.center = (min + max) / 2.0f;
	dimensions.radius = glm::distance(min, max) / 2.0f;
}

/*
	glTF mesh
*/
vkglTF::Mesh::Mesh(Device *device, glm::mat4 matrix) {
	this->device = device;
	this->uniformBlock.matrix = matrix;
	VK_CHECK_RESULT(device->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(uniformBlock),
		&uniformBuffer.buffer,
		&uniformBuffer.memory,
		&uniformBlock));
	VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, uniformBuffer.memory, 0, sizeof(uniformBlock), 0, &uniformBuffer.mapped));
	uniformBuffer.descriptor = { uniformBuffer.buffer, 0, sizeof(uniformBlock) };
};

vkglTF::Mesh::~Mesh() {
    if(uniformBuffer.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device->logicalDevice, uniformBuffer.buffer, nullptr);
    }
    if(uniformBuffer.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device->logicalDevice, uniformBuffer.memory, nullptr);
    }
    for(auto& p : primitives)
    {
        delete p;
    }
}

vkglTF::Skin::~Skin() {
    if(skinBuffer.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device->logicalDevice, skinBuffer.buffer, nullptr);
    }
    if(skinBuffer.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device->logicalDevice, skinBuffer.memory, nullptr);
    }
}

/*
	glTF node
*/
void vkglTF::Node::resetMatrix(){
    translation = initialTranslation;
    rotation = initialRotation;
    scale = initialScale;
}

glm::mat4 vkglTF::Node::localMatrix() {
	return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
}

glm::mat4 vkglTF::Node::getMatrix() {
	glm::mat4 m = localMatrix();
	vkglTF::Node *p = parent;
	while (p) {
		m = p->localMatrix() * m;
		p = p->parent;
	}
	return m;
}

glm::mat4 vkglTF::Node::getSkinMatrix() {
    glm::mat4 m = localMatrix();
    vkglTF::Node *p = parent;
    while (p) {
        m = p->localMatrix() * m;
        p = p->parent;
    }
    return m;
}


void vkglTF::Node::update(const glm::mat4& world_matrix) {
	if (mesh) {
        glm::mat4 m = getMatrix();
        m = world_matrix * m;
        memcpy(mesh->uniformBuffer.mapped, &m, sizeof(glm::mat4));
	}

	for (auto& child : children) {
		child->update(world_matrix);
	}
}

void vkglTF::Node::updateSkin(const glm::mat4& world_matrix) {
    if (mesh) {
        glm::mat4 nodeMatrix = matrix;
        Node *currentParent = parent;
        while (currentParent) {
            nodeMatrix = currentParent->matrix * nodeMatrix;
            currentParent = currentParent->parent;
        }
        nodeMatrix = world_matrix * nodeMatrix;
        memcpy(mesh->uniformBuffer.mapped, &nodeMatrix, sizeof(glm::mat4));
    }
    if (-1 != skinIndex) {
        glm::mat4 inverseTransform = glm::inverse(getMatrix());
        int numJoints = int(skin->joints.size());
        for (size_t i = 0; i < numJoints; i++) {
            vkglTF::Node *jointNode = skin->joints[i];
            skin->jointMatrices[i] = jointNode->getMatrix() * skin->inverseBindMatrices[i];
//            skin->jointMatrices[i] = inverseTransform * skin->jointMatrices[i];
        }
        memcpy(skin->skinBuffer.mapped, skin->jointMatrices.data(), numJoints * one_matrix);
    }

    for (auto& child : children) {
        child->updateSkin(world_matrix);
    }
}


vkglTF::Node::~Node() {
    delete mesh;
	for (auto& child : children) {
		delete child;
	}
}

/*
	glTF default vertex layout with easy Vulkan mapping functions
*/

VkVertexInputBindingDescription vkglTF::Vertex::vertexInputBindingDescription;
std::vector<VkVertexInputAttributeDescription> vkglTF::Vertex::vertexInputAttributeDescriptions;
VkPipelineVertexInputStateCreateInfo vkglTF::Vertex::pipelineVertexInputStateCreateInfo;

VkVertexInputBindingDescription vkglTF::Vertex::inputBindingDescription(uint32_t binding) {
	return VkVertexInputBindingDescription({ binding, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX });
}

VkVertexInputAttributeDescription vkglTF::Vertex::inputAttributeDescription(uint32_t binding, uint32_t location, VertexComponent component) {
	switch (component) {
		case VertexComponent::Position: 
			return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) });
		case VertexComponent::Normal:
			return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		case VertexComponent::UV:
			return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
		case VertexComponent::Color:
			return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color) });
		case VertexComponent::Joint0:
			return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, joint0) });
		case VertexComponent::Weight0:
			return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight0) });
        case VertexComponent::Tangent:
            return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent)} );
		default:
			return VkVertexInputAttributeDescription({});
	}
}

std::vector<VkVertexInputAttributeDescription> vkglTF::Vertex::inputAttributeDescriptions(uint32_t binding, const std::vector<VertexComponent> components) {
	std::vector<VkVertexInputAttributeDescription> result;
	uint32_t location = 0;
	for (VertexComponent component : components) {
		result.push_back(Vertex::inputAttributeDescription(binding, location, component));
		location++;
	}
	return result;
}

/** @brief Returns the default pipeline vertex input state create info structure for the requested vertex components */
VkPipelineVertexInputStateCreateInfo* vkglTF::Vertex::getPipelineVertexInputState(const std::vector<VertexComponent> components) {
	vertexInputBindingDescription = Vertex::inputBindingDescription(0);
	Vertex::vertexInputAttributeDescriptions = Vertex::inputAttributeDescriptions(0, components);
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &Vertex::vertexInputBindingDescription;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::vertexInputAttributeDescriptions.size());
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = Vertex::vertexInputAttributeDescriptions.data();
	return &pipelineVertexInputStateCreateInfo;
}

vkglTF::Texture* vkglTF::Model::getTexture(uint32_t index)
{

	if (index < textures.size()) {
		return &textures[index];
	}
	return nullptr;
}

void vkglTF::Model::createEmptyTexture(VkQueue transferQueue)
{
	emptyTexture.device = device;
	emptyTexture.width = 1;
	emptyTexture.height = 1;
	emptyTexture.layerCount = 1;
	emptyTexture.mipLevels = 1;

	size_t bufferSize = emptyTexture.width * emptyTexture.height * 4;
	unsigned char* buffer = new unsigned char[bufferSize];
	memset(buffer, 0, bufferSize);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	VkBufferCreateInfo bufferCreateInfo = tool::getBufferCreateInfo();
	bufferCreateInfo.size = bufferSize;
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

	VkMemoryAllocateInfo memAllocInfo = tool::memoryAllocateInfo();
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, buffer, bufferSize);
	vkUnmapMemory(device->logicalDevice, stagingMemory);

	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = emptyTexture.width;
	bufferCopyRegion.imageExtent.height = emptyTexture.height;
	bufferCopyRegion.imageExtent.depth = 1;

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = tool::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { emptyTexture.width, emptyTexture.height, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &emptyTexture.image));

	vkGetImageMemoryRequirements(device->logicalDevice, emptyTexture.image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &emptyTexture.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, emptyTexture.image, emptyTexture.deviceMemory, 0));

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;

//	VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkCommandBuffer copyCmd = Shatter::render::ShatterRender::getRender().beginSingleTimeCommands();
    tool::setImageLayout(copyCmd, emptyTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    subresourceRange);
	vkCmdCopyBufferToImage(copyCmd, stagingBuffer, emptyTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
    tool::setImageLayout(copyCmd, emptyTexture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
//	device->flushCommandBuffer(copyCmd, transferQueue);
    Shatter::render::ShatterRender::getRender().endSingleTimeCommands(copyCmd);

    emptyTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Clean up staging resources
	vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
	vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);

	VkSamplerCreateInfo samplerCreateInfo = tool::samplerCreateInfo();
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &emptyTexture.sampler));

	VkImageViewCreateInfo viewCreateInfo = tool::imageViewCreateInfo();
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.image = emptyTexture.image;
	VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &emptyTexture.view));

	emptyTexture.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	emptyTexture.descriptor.imageView = emptyTexture.view;
	emptyTexture.descriptor.sampler = emptyTexture.sampler;
}

void vkglTF::Model::release()
{
    for (auto node : nodes) {
        delete node;
    }
}

/*
	glTF model loading and rendering class
*/
vkglTF::Model::~Model()
{
	vkDestroyBuffer(device->logicalDevice, vertices.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, vertices.memory, nullptr);
	vkDestroyBuffer(device->logicalDevice, indices.buffer, nullptr);
	vkFreeMemory(device->logicalDevice, indices.memory, nullptr);
	for (auto texture : textures) {
		texture.destroy();
	}
	for (auto node : nodes) {
		delete node;
	}
    for (auto skin : skins) {
        delete skin;
    }
	if (descriptorSetLayoutUbo != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(device->logicalDevice, descriptorSetLayoutUbo, nullptr);
		descriptorSetLayoutUbo = VK_NULL_HANDLE;
	}
	if (descriptorSetLayoutImage != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(device->logicalDevice, descriptorSetLayoutImage, nullptr);
		descriptorSetLayoutImage = VK_NULL_HANDLE;
	}
	vkDestroyDescriptorPool(device->logicalDevice, descriptorPool, nullptr);
	emptyTexture.destroy();
}

void vkglTF::Model::loadNode(vkglTF::Node *parent, const tinygltf::Node &node, uint32_t nodeIndex, const tinygltf::Model &model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale)
{
	vkglTF::Node *newNode = new Node{};
	newNode->index = nodeIndex;
	newNode->parent = parent;
	newNode->name = node.name;
	newNode->skinIndex = node.skin;
    new ((glm::mat4*)&newNode->matrix) glm::mat4(1.0f);

	// Generate local node matrix
	if (node.translation.size() == 3) {
        new ((glm::vec3*)&newNode->translation) glm::vec3(glm::make_vec3(node.translation.data()));
        new ((glm::vec3*)&newNode->initialTranslation) glm::vec3(glm::make_vec3(node.translation.data()));
	}
	if (node.rotation.size() == 4) {
        new ((glm::quat*)&newNode->rotation) glm::quat(glm::make_quat(node.rotation.data()));
        new ((glm::quat*)&newNode->initialRotation) glm::quat(glm::make_quat(node.rotation.data()));
	}
	if (node.scale.size() == 3) {
        new ((glm::vec3*)&newNode->scale) glm::vec3(glm::make_vec3(node.scale.data()));
        new ((glm::vec3*)&newNode->initialScale) glm::vec3(glm::make_vec3(node.scale.data()));
	}
	if (node.matrix.size() == 16) {
        new ((glm::mat4*)&newNode->matrix) glm::mat4(glm::make_mat4x4(node.matrix.data()));
		if (globalscale != 1.0f) {
			newNode->matrix = glm::scale(newNode->matrix, glm::vec3(globalscale));
		}
	};

    // Node with children
    if (!node.children.empty()) {
        for (int i : node.children) {
            loadNode(newNode, model.nodes[i], i, model, indexBuffer, vertexBuffer, globalscale);
        }
    }

    // Node contains mesh data
	if (node.mesh > -1) {
		const tinygltf::Mesh mesh = model.meshes[node.mesh];
		Mesh *newMesh = new Mesh(device, newNode->matrix);
		newMesh->name = mesh.name;
		for (size_t j = 0; j < mesh.primitives.size(); j++) {
			const tinygltf::Primitive &primitive = mesh.primitives[j];
			if (primitive.indices < 0) {
				continue;
			}
			uint32_t indexStart = static_cast<uint32_t>(indexBuffer.size());
			uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			glm::vec3 posMin{};
			glm::vec3 posMax{};
			bool hasSkin = false;
			// Vertices
			{
				const float *bufferPos = nullptr;
				const float *bufferNormals = nullptr;
				const float *bufferTexCoords = nullptr;
				const float* bufferColors = nullptr;
				const float *bufferTangents = nullptr;
				uint32_t numColorComponents;
				const uint16_t *bufferJoints = nullptr;
				const float *bufferWeights = nullptr;

				// Position attribute is required
				assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

				const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
				bufferPos = reinterpret_cast<const float *>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

				if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
					const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
					bufferNormals = reinterpret_cast<const float *>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
				}

				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
					const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
					bufferTexCoords = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
				}

				if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& colorAccessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& colorView = model.bufferViews[colorAccessor.bufferView];
					// Color buffer are either of type vec3 or vec4
					numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
					bufferColors = reinterpret_cast<const float*>(&(model.buffers[colorView.buffer].data[colorAccessor.byteOffset + colorView.byteOffset]));
				}

				if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
				{
					const tinygltf::Accessor &tangentAccessor = model.accessors[primitive.attributes.find("TANGENT")->second];
					const tinygltf::BufferView &tangentView = model.bufferViews[tangentAccessor.bufferView];
					bufferTangents = reinterpret_cast<const float *>(&(model.buffers[tangentView.buffer].data[tangentAccessor.byteOffset + tangentView.byteOffset]));
				}

				// Skinning
				// Joints
				if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
					const tinygltf::Accessor &jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView &jointView = model.bufferViews[jointAccessor.bufferView];
					bufferJoints = reinterpret_cast<const uint16_t *>(&(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));
				}

				if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
					const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
					bufferWeights = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
				}

				hasSkin = (bufferJoints && bufferWeights);

				vertexCount = static_cast<uint32_t>(posAccessor.count);

				for (size_t v = 0; v < posAccessor.count; v++) {
					Vertex vert{};
					vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * 3]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
					vert.uv = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
					if (bufferColors) {
						switch (numColorComponents) {
							case 3: 
								vert.color = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
							case 4:
								vert.color = glm::make_vec4(&bufferColors[v * 4]);
						}
					}
					else {
						vert.color = glm::vec4(1.0f);
					}
					vert.tangent = bufferTangents ? glm::vec4(glm::make_vec4(&bufferTangents[v * 4])) : glm::vec4(0.0f);
					vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * 4])) : glm::vec4(0.0f);
					vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * 4]) : glm::vec4(0.0f);
					vertexBuffer.push_back(vert);
				}
			}
			// Indices
			{
				const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
				const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

				indexCount = static_cast<uint32_t>(accessor.count);
                const void *dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

				switch (accessor.componentType) {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					const auto *buf = static_cast<const uint32_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						indexBuffer.push_back(buf[index] + vertexStart);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const auto *buf = static_cast<const uint16_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						indexBuffer.push_back(buf[index] + vertexStart);
					}
                    break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					const auto *buf = static_cast<const uint8_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						indexBuffer.push_back(buf[index] + vertexStart);
					}
                    break;
				}
				default:
					std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
					return;
				}
			}
			Primitive *newPrimitive = new Primitive(indexStart, indexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
			newPrimitive->firstVertex = vertexStart;
			newPrimitive->vertexCount = vertexCount;
			newPrimitive->setDimensions(posMin, posMax);
			newMesh->primitives.push_back(newPrimitive);
		}
		newNode->mesh = newMesh;
	}

    if (parent) {
		parent->children.push_back(newNode);
	} else {
		nodes.push_back(newNode);
	}

	linearNodes.push_back(newNode);
}

void vkglTF::Model::loadNodeDefault(vkglTF::Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, std::vector<uint32_t>& indexBuffer, std::vector<GltfPoint>& vertexBuffer, float globalscale){
    vkglTF::Node *newNode = new Node{};
    newNode->index = nodeIndex;
    newNode->parent = parent;
    newNode->name = node.name;
    newNode->skinIndex = node.skin;
    newNode->matrix = glm::mat4(1.0f);

    // Generate local node matrix
    glm::vec3 translation = glm::vec3(0.0f);
    if (node.translation.size() == 3) {
        translation = glm::make_vec3(node.translation.data());
        newNode->translation = translation;
    }
    glm::mat4 rotation = glm::mat4(1.0f);
    if (node.rotation.size() == 4) {
        glm::quat q = glm::make_quat(node.rotation.data());
        newNode->rotation = glm::mat4(q);
    }
    glm::vec3 scale = glm::vec3(1.0f);
    if (node.scale.size() == 3) {
        scale = glm::make_vec3(node.scale.data());
        newNode->scale = scale;
    }
    if (node.matrix.size() == 16) {
        newNode->matrix = glm::make_mat4x4(node.matrix.data());
        if (globalscale != 1.0f) {
            //newNode->matrix = glm::scale(newNode->matrix, glm::vec3(globalscale));
        }
    };

    // Node with children
    if (node.children.size() > 0) {
        for (auto i = 0; i < node.children.size(); i++) {
            loadNodeDefault(newNode, model.nodes[node.children[i]], node.children[i], model, indexBuffer, vertexBuffer, globalscale);
        }
    }

    // Node contains mesh data
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        Mesh *newMesh = new Mesh(device, newNode->matrix);
        newMesh->name = mesh.name;
        for (size_t j = 0; j < mesh.primitives.size(); j++) {
            const tinygltf::Primitive &primitive = mesh.primitives[j];
            if (primitive.indices < 0) {
                continue;
            }
            uint32_t indexStart = static_cast<uint32_t>(indexBuffer.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            glm::vec3 posMin{};
            glm::vec3 posMax{};
            bool hasSkin = false;
            // Vertices
            {
                const float *bufferPos = nullptr;
                const float *bufferNormals = nullptr;
                const float *bufferTexCoords = nullptr;

                // Position attribute is required
                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
                bufferPos = reinterpret_cast<const float *>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
                posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
                    bufferNormals = reinterpret_cast<const float *>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
                    bufferTexCoords = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                }

                vertexCount = static_cast<uint32_t>(posAccessor.count);

                for (size_t v = 0; v < posAccessor.count; v++) {
                    GltfPoint vert{};
                    vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * 3]), 1.0f);
                    vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
                    vert.uv = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
                    vertexBuffer.push_back(vert);
                }
            }
            // Indices
            {
                const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);

                switch (accessor.componentType) {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                        uint32_t *buf = new uint32_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        uint16_t *buf = new uint16_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                        uint8_t *buf = new uint8_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    default:
                        std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                        return;
                }
            }
            Primitive *newPrimitive = new Primitive(indexStart, indexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
            newPrimitive->firstVertex = vertexStart;
            newPrimitive->vertexCount = vertexCount;
            newPrimitive->setDimensions(posMin, posMax);
            newMesh->primitives.push_back(newPrimitive);
        }
        newNode->mesh = newMesh;
    }
    if (parent) {
        parent->children.push_back(newNode);
    } else {
        nodes.push_back(newNode);
    }
    linearNodes.push_back(newNode);
}

void vkglTF::Model::loadNodePositionNormal(vkglTF::Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, std::vector<uint32_t>& indexBuffer, std::vector<Point3d_Normal>& vertexBuffer, float globalscale){
    vkglTF::Node *newNode = new Node{};
    newNode->index = nodeIndex;
    newNode->parent = parent;
    newNode->name = node.name;
    newNode->skinIndex = node.skin;
    newNode->matrix = glm::mat4(1.0f);

    // Generate local node matrix
    glm::vec3 translation = glm::vec3(0.0f);
    if (node.translation.size() == 3) {
        translation = glm::make_vec3(node.translation.data());
        newNode->translation = translation;
    }
    glm::mat4 rotation = glm::mat4(1.0f);
    if (node.rotation.size() == 4) {
        glm::quat q = glm::make_quat(node.rotation.data());
        newNode->rotation = glm::mat4(q);
    }
    glm::vec3 scale = glm::vec3(1.0f);
    if (node.scale.size() == 3) {
        scale = glm::make_vec3(node.scale.data());
        newNode->scale = scale;
    }
    if (node.matrix.size() == 16) {
        newNode->matrix = glm::make_mat4x4(node.matrix.data());
        if (globalscale != 1.0f) {
            //newNode->matrix = glm::scale(newNode->matrix, glm::vec3(globalscale));
        }
    };

    // Node with children
    if (node.children.size() > 0) {
        for (auto i = 0; i < node.children.size(); i++) {
            loadNodePositionNormal(newNode, model.nodes[node.children[i]], node.children[i], model, indexBuffer, vertexBuffer, globalscale);
        }
    }

    // Node contains mesh data
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        Mesh *newMesh = new Mesh(device, newNode->matrix);
        newMesh->name = mesh.name;
        for (size_t j = 0; j < mesh.primitives.size(); j++) {
            const tinygltf::Primitive &primitive = mesh.primitives[j];
            if (primitive.indices < 0) {
                continue;
            }
            uint32_t indexStart = static_cast<uint32_t>(indexBuffer.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            glm::vec3 posMin{};
            glm::vec3 posMax{};
            bool hasSkin = false;
            // Vertices
            {
                const float *bufferPos = nullptr;
                const float *bufferNormals = nullptr;

                // Position attribute is required
                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
                bufferPos = reinterpret_cast<const float *>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
                posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
                    bufferNormals = reinterpret_cast<const float *>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
                }

                vertexCount = static_cast<uint32_t>(posAccessor.count);

                for (size_t v = 0; v < posAccessor.count; v++) {
                    Point3d_Normal vert{};
                    vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * 3]), 1.0f);
                    vert.nor = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
                    vertexBuffer.push_back(vert);
                }
            }
            // Indices
            {
                const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);

                switch (accessor.componentType) {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                        uint32_t *buf = new uint32_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        uint16_t *buf = new uint16_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                        uint8_t *buf = new uint8_t[accessor.count];
                        memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    default:
                        std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                        return;
                }
            }
            Primitive *newPrimitive = new Primitive(indexStart, indexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
            newPrimitive->firstVertex = vertexStart;
            newPrimitive->vertexCount = vertexCount;
            newPrimitive->setDimensions(posMin, posMax);
            newMesh->primitives.push_back(newPrimitive);
        }
        newNode->mesh = newMesh;
    }
    if (parent) {
        parent->children.push_back(newNode);
    } else {
        nodes.push_back(newNode);
    }
    linearNodes.push_back(newNode);
}

void vkglTF::Model::loadSkins(tinygltf::Model &gltfModel)
{
	for (tinygltf::Skin &source : gltfModel.skins) {
		Skin *newSkin = new Skin{};
        newSkin->device = device;

		newSkin->name = source.name;
				
		// Find skeleton root node
		if (source.skeleton > -1) {
			newSkin->skeletonRoot = nodeFromIndex(source.skeleton);
		}

        // Find Joint nodes
        for (int jointIndex : source.joints) {
            Node* node = nodeFromIndex(jointIndex);
            if (node) {
                newSkin->joints.push_back(node);
            }
        }

        // Get inverse bind matrices from buffer
		if (source.inverseBindMatrices > -1) {
			const tinygltf::Accessor &accessor      = gltfModel.accessors[source.inverseBindMatrices];
			const tinygltf::BufferView &bufferView  = gltfModel.bufferViews[accessor.bufferView];
			const tinygltf::Buffer &buffer          = gltfModel.buffers[bufferView.buffer];
			newSkin->inverseBindMatrices.resize(accessor.count);
            newSkin->jointMatrices.resize(accessor.count);
			memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));

            uint64_t size = one_matrix * newSkin->inverseBindMatrices.size();
            VK_CHECK_RESULT(device->createBuffer(
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    size,
                    &newSkin->skinBuffer.buffer,
                    &newSkin->skinBuffer.memory,
                    newSkin->inverseBindMatrices.data()));
            VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, newSkin->skinBuffer.memory, 0, size, 0, &newSkin->skinBuffer.mapped));
            newSkin->skinBuffer.descriptor = { newSkin->skinBuffer.buffer, 0, size };
        }

        skins.push_back(newSkin);
	}
}

void vkglTF::Model::loadImages(tinygltf::Model &gltfModel, Device *device, VkQueue transferQueue)
{
	for (tinygltf::Image &image : gltfModel.images) {
		vkglTF::Texture texture;
        texture.fromGlTfImage(image, path, device, transferQueue);
		textures.push_back(texture);
	}
	// Create an empty texture to be used for empty material images
	createEmptyTexture(transferQueue);
}

void vkglTF::Model::loadMaterials(tinygltf::Model &gltfModel)
{
	for (tinygltf::Material &mat : gltfModel.materials) {
		vkglTF::Material material(device);
		if (mat.values.find("baseColorTexture") != mat.values.end()) {
			material.baseColorTexture = getTexture(gltfModel.textures[mat.values["baseColorTexture"].TextureIndex()].source);
		}
		// Metallic roughness workflow
		if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
			material.metallicRoughnessTexture = getTexture(gltfModel.textures[mat.values["metallicRoughnessTexture"].TextureIndex()].source);
		}
		if (mat.values.find("roughnessFactor") != mat.values.end()) {
			material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
		}
		if (mat.values.find("metallicFactor") != mat.values.end()) {
			material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
		}
		if (mat.values.find("baseColorFactor") != mat.values.end()) {
			material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
		}				
		if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
			material.normalTexture = getTexture(gltfModel.textures[mat.additionalValues["normalTexture"].TextureIndex()].source);
		} else {
			material.normalTexture = &emptyTexture;
		}
		if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
			material.emissiveTexture = getTexture(gltfModel.textures[mat.additionalValues["emissiveTexture"].TextureIndex()].source);
		}
		if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
			material.occlusionTexture = getTexture(gltfModel.textures[mat.additionalValues["occlusionTexture"].TextureIndex()].source);
		}
		if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
			tinygltf::Parameter param = mat.additionalValues["alphaMode"];
			if (param.string_value == "BLEND") {
				material.alphaMode = Material::ALPHAMODE_BLEND;
			}
			if (param.string_value == "MASK") {
				material.alphaMode = Material::ALPHAMODE_MASK;
			}
		}
		if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
			material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
		}

		materials.push_back(material);
	}
	// Push a default material at the end of the list for meshes with no material assigned
	materials.push_back(Material(device));
}

void vkglTF::Model::loadAnimations(tinygltf::Model &gltfModel)
{
	for (tinygltf::Animation &anim : gltfModel.animations) {
		vkglTF::Animation animation{};
		animation.name = anim.name;
		if (anim.name.empty()) {
			animation.name = std::to_string(animations.size());
		}

		// Samplers
		for (auto &samp : anim.samplers) {
			vkglTF::AnimationSampler sampler{};

			if (samp.interpolation == "LINEAR") {
				sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
			}
			if (samp.interpolation == "STEP") {
				sampler.interpolation = AnimationSampler::InterpolationType::STEP;
			}
			if (samp.interpolation == "CUBICSPLINE") {
				sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
			}

			// Read sampler input time values
			{
				const tinygltf::Accessor &accessor = gltfModel.accessors[samp.input];
				const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

				const auto *buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++) {
					sampler.inputs.push_back(buf[index]);
				}
				for (auto input : sampler.inputs) {
					if (input < animation.start) {
						animation.start = input;
					};
					if (input > animation.end) {
						animation.end = input;
					}
				}
			}

			// Read sampler output T/R/S values 
			{
				const tinygltf::Accessor &accessor = gltfModel.accessors[samp.output];
				const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                switch (accessor.type) {
				case TINYGLTF_TYPE_VEC3: {
                    const auto *buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						sampler.outputsVec4.emplace_back(buf[index], 0.0f);
					}
                    break;
				}
				case TINYGLTF_TYPE_VEC4: {
                    const auto *buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++) {
						sampler.outputsVec4.push_back(buf[index]);
					}
                    break;
				}
				default: {
					std::cout << "unknown type" << std::endl;
					break;
				}
				}
			}

			animation.samplers.push_back(sampler);
		}

		// Channels
		for (auto &source: anim.channels) {
			vkglTF::AnimationChannel channel{};

			if (source.target_path == "rotation") {
				channel.path = AnimationChannel::PathType::ROTATION;
			}
			if (source.target_path == "translation") {
				channel.path = AnimationChannel::PathType::TRANSLATION;
			}
			if (source.target_path == "scale") {
				channel.path = AnimationChannel::PathType::SCALE;
			}
			if (source.target_path == "weights") {
				std::cout << "weights not yet supported, skipping channel" << std::endl;
				continue;
			}
			channel.samplerIndex = source.sampler;
			channel.node = nodeFromIndex(source.target_node);
			if (!channel.node) {
				continue;
			}

			animation.channels.push_back(channel);
		}

		animations.push_back(animation);
	}
}

void vkglTF::Model::initInstanceMesh(Device* _device, size_t _count)
{
    for (auto& node : linearNodes) {
//        node->initInstanceMesh(_device, _count);
    }
}

void vkglTF::Model::loadFromBinary(const std::vector<unsigned char>& str,
                                   const std::string& _filename,
                                   Device *pDevice,
                                   VkQueue transferQueue,
                                   uint32_t fileLoadingFlags,
                                   const glm::mat4& world_matrix,
                                   float scale){
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    if (fileLoadingFlags & FileLoadingFlags::DontLoadImages) {
        gltfContext.SetImageLoader(loadImageDataFuncEmpty, nullptr);
    } else {
        gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
    }
#if defined(__ANDROID__)
    // On Android all assets are packed with the apk in a compressed form, so we need to open them using the asset manager
	// We let tinygltf handle this, by passing the asset manager of our app
	tinygltf::asset_manager = androidApp->activity->assetManager;
#endif
    size_t pos = _filename.find_last_of('/');
    path = _filename.substr(0, pos);

    std::string error, warning;

    this->device = pDevice;

#if defined(__ANDROID__)
    // On Android all assets are packed with the apk in a compressed form, so we need to open them using the asset manager
	// We let tinygltf handle this, by passing the asset manager of our app
	tinygltf::asset_manager = androidApp->activity->assetManager;
#endif
    bool fileLoaded = gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning,
                                                      str.data(), str.size(), _filename);

    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;

    if (fileLoaded) {
        if (!(fileLoadingFlags & FileLoadingFlags::DontLoadImages)) {
            loadImages(gltfModel, pDevice, transferQueue);
        }
        loadMaterials(gltfModel);
        const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
        for (int i : scene.nodes) {
            const tinygltf::Node node = gltfModel.nodes[i];
            loadNode(nullptr, node, i, gltfModel, indexBuffer, vertexBuffer, scale);
        }
        if (!gltfModel.animations.empty()) {
            loadAnimations(gltfModel);
        }
        loadSkins(gltfModel);

        for (auto node : linearNodes) {
            // Assign skins
            if (node->skinIndex > -1) {
                node->skin = skins[node->skinIndex];
            }
            // Initial pose
            if (node->mesh) {
                if (node->skin)
                {
                    node->updateSkin(world_matrix);
                } else {
                    node->update(world_matrix);
                }
            }
        }
    }
    else {
        // TODO: throw
        throw std::runtime_error("Could not load glTF file \"" + _filename + "\": " + error);
        return;
    }

    // Pre-Calculations for requested features
    if ((fileLoadingFlags & FileLoadingFlags::PreTransformVertices) || (fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors) || (fileLoadingFlags & FileLoadingFlags::FlipY)) {
        const bool preTransform = fileLoadingFlags & FileLoadingFlags::PreTransformVertices;
        const bool preMultiplyColor = fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors;
        const bool flipY = fileLoadingFlags & FileLoadingFlags::FlipY;
        for (Node* node : linearNodes) {
            if (node->mesh) {
                const glm::mat4 localMatrix = node->getMatrix();
                for (Primitive* primitive : node->mesh->primitives) {
                    for (uint32_t i = 0; i < primitive->vertexCount; i++) {
                        Vertex& vertex = vertexBuffer[primitive->firstVertex + i];
                        // Pre-transform vertex positions by node-hierarchy
                        if (preTransform) {
                            vertex.pos = glm::vec3(localMatrix * glm::vec4(vertex.pos, 1.0f));
                            vertex.normal = glm::normalize(glm::mat3(localMatrix) * vertex.normal);
                        }
                        // Flip Y-Axis of vertex positions
                        if (flipY) {
                            vertex.pos.y *= -1.0f;
                            vertex.normal.y *= -1.0f;
                        }
                        // Pre-Multiply vertex colors with material base color
                        if (preMultiplyColor) {
                            vertex.color = primitive->material.baseColorFactor * vertex.color;
                        }
                    }
                }
            }
        }
    }

    for (const auto& extension : gltfModel.extensionsUsed) {
        if (extension == "KHR_materials_pbrSpecularGlossiness") {
            std::cout << "Required extension: " << extension;
            metallicRoughnessWorkflow = false;
        }
    }

    size_t vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);
    size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    indices.count = static_cast<uint32_t>(indexBuffer.size());
    vertices.count = static_cast<uint32_t>(vertexBuffer.size());

    assert((vertexBufferSize > 0) && (indexBufferSize > 0));

    struct StagingBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
    } vertexStaging, indexStaging;

    // Create staging buffers
    // Vertex data
    VK_CHECK_RESULT(pDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBufferSize,
            &vertexStaging.buffer,
            &vertexStaging.memory,
            vertexBuffer.data()));
    // Index data
    VK_CHECK_RESULT(pDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indexBufferSize,
            &indexStaging.buffer,
            &indexStaging.memory,
            indexBuffer.data()));

    // Create pDevice local buffers
    // Vertex buffer
    VK_CHECK_RESULT(pDevice->createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | memoryPropertyFlags,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBufferSize,
            &vertices.buffer,
            &vertices.memory));
    // Index buffer
    VK_CHECK_RESULT(pDevice->createBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | memoryPropertyFlags,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBufferSize,
            &indices.buffer,
            &indices.memory));

    // Copy from staging buffers
    VkCommandBuffer copyCmd  = Shatter::render::ShatterRender::getRender().beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};

    copyRegion.size = vertexBufferSize;
    vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertices.buffer, 1, &copyRegion);

    copyRegion.size = indexBufferSize;
    vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indices.buffer, 1, &copyRegion);

    Shatter::render::ShatterRender::getRender().endSingleTimeCommands(copyCmd);

    vkDestroyBuffer(pDevice->logicalDevice, vertexStaging.buffer, nullptr);
    vkFreeMemory(pDevice->logicalDevice, vertexStaging.memory, nullptr);
    vkDestroyBuffer(pDevice->logicalDevice, indexStaging.buffer, nullptr);
    vkFreeMemory(pDevice->logicalDevice, indexStaging.memory, nullptr);

    getSceneDimensions();

    // Setup descriptors
    uint32_t uboCount{ 0 };
    uint32_t imageCount{ 0 };
    uint32_t skinCount{ 0 };
    for (auto node : linearNodes) {
        if (node->mesh) {
            uboCount++;
        }
    }
    for (auto material : materials) {
        if (material.baseColorTexture != nullptr) {
            imageCount++;
        }
    }
    for (auto skin : skins) {
        if (!skin->inverseBindMatrices.empty()) {
            skinCount++;
        }
    }
    std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20 },
    };
    if (imageCount > 0) {
        if (descriptorBindingFlags & DescriptorBindingFlags::ImageBaseColor) {
            poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 });
        }
        if (descriptorBindingFlags & DescriptorBindingFlags::ImageNormalMap) {
            poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 });
        }
    }
    VkDescriptorPoolCreateInfo descriptorPoolCI{};
    descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCI.pPoolSizes = poolSizes.data();
    descriptorPoolCI.maxSets = uboCount + imageCount + skinCount;
    VK_CHECK_RESULT(vkCreateDescriptorPool(pDevice->logicalDevice, &descriptorPoolCI, nullptr, &descriptorPool));

    // Descriptors for per-node uniform buffers
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
        descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorLayoutCI.pBindings = setLayoutBindings.data();
        VkDescriptorSetLayout uboSetLayout;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(pDevice->logicalDevice, &descriptorLayoutCI, nullptr, &uboSetLayout));
        for (auto node : nodes) {
            prepareNodeDescriptor(node, uboSetLayout);
        }
        vkDestroyDescriptorSetLayout(pDevice->logicalDevice, uboSetLayout, nullptr);
    }

    // Descriptors for per-skin images
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
        descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorLayoutCI.pBindings = setLayoutBindings.data();
        VkDescriptorSetLayout skinSetLayout;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(pDevice->logicalDevice, &descriptorLayoutCI, nullptr, &skinSetLayout));
        for (auto skin : skins) {
            prepareSkinDescriptor(skin, skinSetLayout);
        }
        vkDestroyDescriptorSetLayout(pDevice->logicalDevice, skinSetLayout, nullptr);
    }

    // Descriptors for per-material images
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        if (descriptorBindingFlags & DescriptorBindingFlags::ImageBaseColor) {
            setLayoutBindings.push_back(tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(setLayoutBindings.size())));
        }
        if (descriptorBindingFlags & DescriptorBindingFlags::ImageNormalMap) {
            setLayoutBindings.push_back(tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(setLayoutBindings.size())));
        }
        VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
        descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorLayoutCI.pBindings = setLayoutBindings.data();
        VkDescriptorSetLayout imageSetLayout;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(pDevice->logicalDevice, &descriptorLayoutCI, nullptr, &imageSetLayout));
        for (auto& material : materials) {
            if (material.baseColorTexture != nullptr) {
                material.createDescriptorSet(descriptorPool, imageSetLayout, descriptorBindingFlags);
            }
        }
        vkDestroyDescriptorSetLayout(pDevice->logicalDevice, imageSetLayout, nullptr);
    }
}

void vkglTF::Model::loadFromFile(const std::string& filename,
                                 Device *pDevice,
                                 VkQueue transferQueue,
                                 uint32_t fileLoadingFlags,
                                 const glm::mat4& world_matrix,
                                 float scale,
                                 bool binary)
{
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF gltfContext;
	if (fileLoadingFlags & FileLoadingFlags::DontLoadImages) {
		gltfContext.SetImageLoader(loadImageDataFuncEmpty, nullptr);
	} else {
		gltfContext.SetImageLoader(loadImageDataFunc, nullptr);
	}
#if defined(__ANDROID__)
	// On Android all assets are packed with the apk in a compressed form, so we need to open them using the asset manager
	// We let tinygltf handle this, by passing the asset manager of our app
	tinygltf::asset_manager = androidApp->activity->assetManager;
#endif
	size_t pos = filename.find_last_of('/');
	path = filename.substr(0, pos);

	std::string error, warning;

	this->device = pDevice;

#if defined(__ANDROID__)
	// On Android all assets are packed with the apk in a compressed form, so we need to open them using the asset manager
	// We let tinygltf handle this, by passing the asset manager of our app
	tinygltf::asset_manager = androidApp->activity->assetManager;
#endif
    bool fileLoaded;
    if (binary) {
        fileLoaded = gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, filename);
    } else {
        fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename);
    }

	std::vector<uint32_t> indexBuffer;
	std::vector<Vertex> vertexBuffer;

	if (fileLoaded) {
		if (!(fileLoadingFlags & FileLoadingFlags::DontLoadImages)) {
			loadImages(gltfModel, pDevice, transferQueue);
		}
		loadMaterials(gltfModel);
		const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
		for (int i : scene.nodes) {
			const tinygltf::Node node = gltfModel.nodes[i];
			loadNode(nullptr, node, i, gltfModel, indexBuffer, vertexBuffer, scale);
		}
		if (!gltfModel.animations.empty()) {
			loadAnimations(gltfModel);
		}
		loadSkins(gltfModel);

		for (auto node : linearNodes) {
			// Assign skins
			if (node->skinIndex > -1) {
				node->skin = skins[node->skinIndex];
			}
			// Initial pose
			if (node->mesh) {
                if (node->skin)
                {
                    node->updateSkin(world_matrix);
                } else {
                    node->update(world_matrix);
                }
			}
		}
	}
	else {
		// TODO: throw
        throw std::runtime_error("Could not load glTF file \"" + filename + "\": " + error);
		return;
	}

	// Pre-Calculations for requested features
	if ((fileLoadingFlags & FileLoadingFlags::PreTransformVertices) || (fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors) || (fileLoadingFlags & FileLoadingFlags::FlipY)) {
		const bool preTransform = fileLoadingFlags & FileLoadingFlags::PreTransformVertices;
		const bool preMultiplyColor = fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors;
		const bool flipY = fileLoadingFlags & FileLoadingFlags::FlipY;
		for (Node* node : linearNodes) {
			if (node->mesh) {
				const glm::mat4 localMatrix = node->getMatrix();
				for (Primitive* primitive : node->mesh->primitives) {
					for (uint32_t i = 0; i < primitive->vertexCount; i++) {
						Vertex& vertex = vertexBuffer[primitive->firstVertex + i];
						// Pre-transform vertex positions by node-hierarchy
						if (preTransform) {
							vertex.pos = glm::vec3(localMatrix * glm::vec4(vertex.pos, 1.0f));
							vertex.normal = glm::normalize(glm::mat3(localMatrix) * vertex.normal);
						}
						// Flip Y-Axis of vertex positions
						if (flipY) {
							vertex.pos.y *= -1.0f;
							vertex.normal.y *= -1.0f;
						}
						// Pre-Multiply vertex colors with material base color
						if (preMultiplyColor) {
							vertex.color = primitive->material.baseColorFactor * vertex.color;
						}
					}
				}
			}
		}
	}

	for (const auto& extension : gltfModel.extensionsUsed) {
		if (extension == "KHR_materials_pbrSpecularGlossiness") {
			std::cout << "Required extension: " << extension;
			metallicRoughnessWorkflow = false;
		}
	}

	size_t vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);
	size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
	indices.count = static_cast<uint32_t>(indexBuffer.size());
	vertices.count = static_cast<uint32_t>(vertexBuffer.size());

	assert((vertexBufferSize > 0) && (indexBufferSize > 0));

	struct StagingBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	} vertexStaging, indexStaging;

	// Create staging buffers
	// Vertex data
	VK_CHECK_RESULT(pDevice->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		vertexBufferSize,
		&vertexStaging.buffer,
		&vertexStaging.memory,
		vertexBuffer.data()));
	// Index data
	VK_CHECK_RESULT(pDevice->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indexBufferSize,
		&indexStaging.buffer,
		&indexStaging.memory,
		indexBuffer.data()));

	// Create pDevice local buffers
	// Vertex buffer
	VK_CHECK_RESULT(pDevice->createBuffer(
	    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | memoryPropertyFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBufferSize,
		&vertices.buffer,
		&vertices.memory));
	// Index buffer
	VK_CHECK_RESULT(pDevice->createBuffer(
	    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | memoryPropertyFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBufferSize,
		&indices.buffer,
		&indices.memory));

	// Copy from staging buffers
    VkCommandBuffer copyCmd  = Shatter::render::ShatterRender::getRender().beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};

	copyRegion.size = vertexBufferSize;
	vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertices.buffer, 1, &copyRegion);

	copyRegion.size = indexBufferSize;
	vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indices.buffer, 1, &copyRegion);

    Shatter::render::ShatterRender::getRender().endSingleTimeCommands(copyCmd);

	vkDestroyBuffer(pDevice->logicalDevice, vertexStaging.buffer, nullptr);
	vkFreeMemory(pDevice->logicalDevice, vertexStaging.memory, nullptr);
	vkDestroyBuffer(pDevice->logicalDevice, indexStaging.buffer, nullptr);
	vkFreeMemory(pDevice->logicalDevice, indexStaging.memory, nullptr);

	getSceneDimensions();

	// Setup descriptors
	uint32_t uboCount{ 0 };
	uint32_t imageCount{ 0 };
    uint32_t skinCount{ 0 };
    for (auto node : linearNodes) {
		if (node->mesh) {
			uboCount++;
		}
	}
	for (auto material : materials) {
		if (material.baseColorTexture != nullptr) {
			imageCount++;
		}
	}
    for (auto skin : skins) {
        if (skin->inverseBindMatrices.size() != 0) {
            skinCount++;
        }
    }
    std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20 },
    };
	if (imageCount > 0) {
		if (descriptorBindingFlags & DescriptorBindingFlags::ImageBaseColor) {
			poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 });
		}
		if (descriptorBindingFlags & DescriptorBindingFlags::ImageNormalMap) {
			poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 });
		}
	}
	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCI.pPoolSizes = poolSizes.data();
	descriptorPoolCI.maxSets = uboCount + imageCount + skinCount;
	VK_CHECK_RESULT(vkCreateDescriptorPool(pDevice->logicalDevice, &descriptorPoolCI, nullptr, &descriptorPool));

    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
        descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorLayoutCI.pBindings = setLayoutBindings.data();
        VkDescriptorSetLayout uboSetLayout;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(pDevice->logicalDevice, &descriptorLayoutCI, nullptr, &uboSetLayout));
        for (auto node : nodes) {
            prepareNodeDescriptor(node, uboSetLayout);
        }
        vkDestroyDescriptorSetLayout(pDevice->logicalDevice, uboSetLayout, nullptr);
    }

    // Descriptors for per-skin images
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
        descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorLayoutCI.pBindings = setLayoutBindings.data();
        VkDescriptorSetLayout skinSetLayout;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(pDevice->logicalDevice, &descriptorLayoutCI, nullptr, &skinSetLayout));
        for (auto skin : skins) {
            prepareSkinDescriptor(skin, skinSetLayout);
        }
        vkDestroyDescriptorSetLayout(pDevice->logicalDevice, skinSetLayout, nullptr);
    }

    // Descriptors for per-material images
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        if (descriptorBindingFlags & DescriptorBindingFlags::ImageBaseColor) {
            setLayoutBindings.push_back(tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(setLayoutBindings.size())));
        }
        if (descriptorBindingFlags & DescriptorBindingFlags::ImageNormalMap) {
            setLayoutBindings.push_back(tool::getSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(setLayoutBindings.size())));
        }
        VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
        descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorLayoutCI.pBindings = setLayoutBindings.data();
        VkDescriptorSetLayout imageSetLayout;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(pDevice->logicalDevice, &descriptorLayoutCI, nullptr, &imageSetLayout));
        for (auto& material : materials) {
            if (material.baseColorTexture != nullptr) {
                material.createDescriptorSet(descriptorPool, imageSetLayout, descriptorBindingFlags);
            }
        }
        vkDestroyDescriptorSetLayout(pDevice->logicalDevice, imageSetLayout, nullptr);
    }
}

void vkglTF::Model::loadFromFileDefault(const std::string& filename,
                         Device* device,
                         VkQueue transferQueue,
                         std::vector<GltfPoint>& point,
                         std::vector<uint32_t>& indexBuffer,
                         uint32_t fileLoadingFlags,
                         float scale)
{
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    size_t pos = filename.find_last_of('/');
    path = filename.substr(0, pos);
    std::string error, warning;
    this->device = device;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename);
    if (fileLoaded) {
        const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
            loadNodeDefault(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, point, scale);
        }
        for (auto node : linearNodes) {
            // Initial pose
            if (node->mesh) {
                node->update();
            }
        }
    }
    else {
        // TODO: throw
        throw std::runtime_error("Could not load glTF file \"" + filename + "\": " + error);
        return;
    }

    // Pre-Calculations for requested features
    if (fileLoadingFlags & FileLoadingFlags::PreTransformVertices) {
        const bool preTransform = fileLoadingFlags & FileLoadingFlags::PreTransformVertices;
        for (Node* node : linearNodes) {
            if (node->mesh) {
                const glm::mat4 localMatrix = node->getMatrix();
                for (Primitive* primitive : node->mesh->primitives) {
                    for (uint32_t i = 0; i < primitive->vertexCount; i++) {
                        GltfPoint& vertex = point[primitive->firstVertex + i];
                        // Pre-transform vertex positions by node-hierarchy
                        if (preTransform) {
                            vertex.pos = glm::vec3(localMatrix * glm::vec4(vertex.pos, 1.0f));
                            vertex.normal = glm::normalize(glm::mat3(localMatrix) * vertex.normal);
                        }
                    }
                }
            }
        }
    }

    for (auto extension : gltfModel.extensionsUsed) {
        if (extension == "KHR_materials_pbrSpecularGlossiness") {
            std::cout << "Required extension: " << extension;
            metallicRoughnessWorkflow = false;
        }
    }
}

void vkglTF::Model::loadFromFilePositionNormal(const std::string& filename,
                                Device* device,
                                VkQueue transferQueue,
                                std::vector<Point3d_Normal>& point,
                                std::vector<uint32_t>& indexBuffer,
                                uint32_t fileLoadingFlags,
                                float scale){
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    size_t pos = filename.find_last_of('/');
    path = filename.substr(0, pos);
    std::string error, warning;
    this->device = device;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename);
    if (fileLoaded) {
        const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
            loadNodePositionNormal(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, point, scale);
        }
        for (auto node : linearNodes) {
            // Initial pose
            if (node->mesh) {
                node->update();
            }
        }
    }
    else {
        // TODO: throw
        throw std::runtime_error("Could not load glTF file \"" + filename + "\": " + error);
        return;
    }

    // Pre-Calculations for requested features
    if (fileLoadingFlags & FileLoadingFlags::PreTransformVertices) {
        const bool preTransform = fileLoadingFlags & FileLoadingFlags::PreTransformVertices;
        for (Node* node : linearNodes) {
            if (node->mesh) {
                const glm::mat4 localMatrix = node->getMatrix();
                for (Primitive* primitive : node->mesh->primitives) {
                    for (uint32_t i = 0; i < primitive->vertexCount; i++) {
                        Point3d_Normal& vertex = point[primitive->firstVertex + i];
                        // Pre-transform vertex positions by node-hierarchy
                        if (preTransform) {
                            vertex.pos = glm::vec3(localMatrix * glm::vec4(vertex.pos, 1.0f));
                            vertex.nor = glm::normalize(glm::mat3(localMatrix) * vertex.nor);
                        }
                    }
                }
            }
        }
    }

    for (auto extension : gltfModel.extensionsUsed) {
        if (extension == "KHR_materials_pbrSpecularGlossiness") {
            std::cout << "Required extension: " << extension;
            metallicRoughnessWorkflow = false;
        }
    }
}

void vkglTF::Model::bindBuffers(VkCommandBuffer commandBuffer)
{
	const VkDeviceSize offsets[1] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
	buffersBound = true;
}

void vkglTF::Model::drawNode(Node *node, VkCommandBuffer commandBuffer, uint32_t renderFlags, VkPipelineLayout pipelineLayout, uint32_t bindImageSet)
{
	if (node->mesh) {
        if (node->skin) {
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout,
                                    2,
                                    1,
                                    &node->skin->skinBuffer.descriptorSet,
                                    0,
                                    nullptr);
        }
        if (VK_NULL_HANDLE != pipelineLayout)
        {
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout,
                                    0,
                                    1,
                                    &node->mesh->uniformBuffer.descriptorSet,
                                    0,
                                    nullptr);
        }
		for (Primitive* primitive : node->mesh->primitives) {
			bool skip = false;
			const vkglTF::Material& material = primitive->material;
			if (renderFlags & RenderFlags::RenderOpaqueNodes) {
				skip = (material.alphaMode != Material::ALPHAMODE_OPAQUE);
			}
			if (renderFlags & RenderFlags::RenderAlphaMaskedNodes) {
				skip = (material.alphaMode != Material::ALPHAMODE_MASK);
			}
			if (renderFlags & RenderFlags::RenderAlphaBlendedNodes) {
				skip = (material.alphaMode != Material::ALPHAMODE_BLEND);
			}
			if (!skip) {
				if (renderFlags & RenderFlags::BindImages) {
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, bindImageSet, 1, &material.descriptorSet, 0, nullptr);
				}
				vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
			}
		}
	}
	for (auto& child : node->children) {
		drawNode(child, commandBuffer, renderFlags,pipelineLayout,bindImageSet);
	}
}

void vkglTF::Model::drawNodeInstance(Node* node,
                                     VkCommandBuffer commandBuffer,
                                     uint32_t instanceCount,
                                     uint32_t renderFlags,
                                     VkPipelineLayout pipelineLayout,
                                     uint32_t bindImageSet,
                                     bool useUniform){
    if (node->mesh) {
        if (node->skin) {
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout,
                                    2,
                                    1,
                                    &node->skin->skinBuffer.descriptorSet,
                                    0,
                                    nullptr);
        }
        if(VK_NULL_HANDLE != pipelineLayout)
        {
            if (useUniform) {
                vkCmdBindDescriptorSets(commandBuffer,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipelineLayout,
                                        0,
                                        1,
                                        &node->mesh->uniformBuffer.descriptorSet,
                                        0,
                                        nullptr);
            }
        }
        for (Primitive* primitive : node->mesh->primitives) {
            bool skip = false;
            const vkglTF::Material& material = primitive->material;
            if (renderFlags & RenderFlags::RenderOpaqueNodes) {
                skip = (material.alphaMode != Material::ALPHAMODE_OPAQUE);
            }
            if (renderFlags & RenderFlags::RenderAlphaMaskedNodes) {
                skip = (material.alphaMode != Material::ALPHAMODE_MASK);
            }
            if (renderFlags & RenderFlags::RenderAlphaBlendedNodes) {
                skip = (material.alphaMode != Material::ALPHAMODE_BLEND);
            }
            if (!skip) {
                if (renderFlags & RenderFlags::BindImages && material.descriptorSet != VK_NULL_HANDLE) {
                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                                bindImageSet, 1, &material.descriptorSet, 0, nullptr);
                        vkCmdDrawIndexed(commandBuffer, primitive->indexCount, instanceCount, primitive->firstIndex, 0,
                                         0);
                } else if ((renderFlags & RenderFlags::BindImages) == 0 && material.descriptorSet == VK_NULL_HANDLE){
                    vkCmdDrawIndexed(commandBuffer, primitive->indexCount, instanceCount, primitive->firstIndex, 0, 0);
                }
            }
        }
    }
    for (auto& child : node->children) {
        drawNodeInstance(child, commandBuffer, instanceCount, renderFlags, pipelineLayout, bindImageSet, useUniform);
    }
}

void vkglTF::Model::draw(VkCommandBuffer commandBuffer, uint32_t renderFlags, VkPipelineLayout pipelineLayout, uint32_t bindImageSet)
{
	if (!buffersBound) {
		const VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
	}
	for (auto& node : nodes) {
		drawNode(node, commandBuffer, renderFlags, pipelineLayout, bindImageSet);
	}
}

void vkglTF::Model::drawInstance(VkCommandBuffer commandBuffer,
                                 uint32_t instanceCount,
                                 uint32_t renderFlags,
                                 VkPipelineLayout pipelineLayout,
                                 uint32_t bindImageSet,
                                 bool useUniform){
    if (!buffersBound) {
        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
    }
    for (auto& node : nodes) {
        drawNodeInstance(node, commandBuffer, instanceCount, renderFlags, pipelineLayout, bindImageSet, useUniform);
    }
}

void vkglTF::Model::getNodeDimensions(Node *node, glm::vec3 &min, glm::vec3 &max)
{
	if (node->mesh) {
		for (Primitive *primitive : node->mesh->primitives) {
			glm::vec4 locMin = glm::vec4(primitive->dimensions.min, 1.0f) * node->getMatrix();
			glm::vec4 locMax = glm::vec4(primitive->dimensions.max, 1.0f) * node->getMatrix();
			if (locMin.x < min.x) { min.x = locMin.x; }
			if (locMin.y < min.y) { min.y = locMin.y; }
			if (locMin.z < min.z) { min.z = locMin.z; }
			if (locMax.x > max.x) { max.x = locMax.x; }
			if (locMax.y > max.y) { max.y = locMax.y; }
			if (locMax.z > max.z) { max.z = locMax.z; }
		}
	}
	for (auto child : node->children) {
		getNodeDimensions(child, min, max);
	}
}

void vkglTF::Model::getSceneDimensions()
{
	dimensions.min = glm::vec3(FLT_MAX);
	dimensions.max = glm::vec3(-FLT_MAX);
	for (auto node : nodes) {
		getNodeDimensions(node, dimensions.min, dimensions.max);
	}
	dimensions.size = dimensions.max - dimensions.min;
	dimensions.center = (dimensions.min + dimensions.max) / 2.0f;
	dimensions.radius = glm::distance(dimensions.min, dimensions.max) / 2.0f;
}

void vkglTF::Model::resetAnimation()
{
    for (auto &node : nodes) {
        node->resetMatrix();
    }
}

void vkglTF::Model::updateAnimation(uint32_t index, float time, const glm::mat4& world_matrix, bool ifSkin)
{
	if (index > static_cast<uint32_t>(animations.size()) - 1) {
		std::cout << "No animation with index " << index << std::endl;
		return;
	}
	Animation &animation = animations[index];

    static int times = 0;
    int tmp_times = int(time / animation.end);
    if(tmp_times != times)
    {
        times = tmp_times;
//        resetAnimation();
    }
    float local_time = time - float(times) * animation.end;
	bool updated = false;
	for (auto& channel : animation.channels) {
		vkglTF::AnimationSampler &sampler = animation.samplers[channel.samplerIndex];
		if (sampler.inputs.size() > sampler.outputsVec4.size()) {
			continue;
		}

		for (auto i = 0; i < sampler.inputs.size() - 1; i++) {
			if ((local_time >= sampler.inputs[i]) && (local_time <= sampler.inputs[i + 1])) {
				float u = std::max(0.0f, local_time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (u <= 1.0f) {
					switch (channel.path) {
					case vkglTF::AnimationChannel::PathType::TRANSLATION: {
                        new ((glm::vec3*)&channel.node->translation) glm::vec3(glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u));
						break;
					}
					case vkglTF::AnimationChannel::PathType::SCALE: {
                        new ((glm::vec3*)&channel.node->scale) glm::vec3(glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u));
						break;
					}
					case vkglTF::AnimationChannel::PathType::ROTATION: {
						glm::quat q1;
						q1.x = sampler.outputsVec4[i].x;
						q1.y = sampler.outputsVec4[i].y;
						q1.z = sampler.outputsVec4[i].z;
						q1.w = sampler.outputsVec4[i].w;
						glm::quat q2;
						q2.x = sampler.outputsVec4[i + 1].x;
						q2.y = sampler.outputsVec4[i + 1].y;
						q2.z = sampler.outputsVec4[i + 1].z;
						q2.w = sampler.outputsVec4[i + 1].w;
                        new ((glm::quat*)&channel.node->rotation) glm::quat(glm::normalize(glm::slerp(q1, q2, u)));
						break;
					}
					}
					updated = true;
				}
			}
		}
	}
	if (updated) {
		for (auto &node : nodes) {
            if (ifSkin)
            {
                node->updateSkin(world_matrix);
            } else {
                node->update(world_matrix);
            }
		}
	}
}

/*
	Helper functions
*/
vkglTF::Node* vkglTF::Model::findNode(Node *parent, uint32_t index) {
	Node* nodeFound = nullptr;
	if (parent->index == index) {
		return parent;
	}
	for (auto& child : parent->children) {
		nodeFound = findNode(child, index);
		if (nodeFound) {
			break;
		}
	}
	return nodeFound;
}

vkglTF::Node* vkglTF::Model::nodeFromIndex(uint32_t index) {
	Node* nodeFound = nullptr;
	for (auto &node : nodes) {
		nodeFound = findNode(node, index);
		if (nodeFound) {
			break;
		}
	}
	return nodeFound;
}

void vkglTF::Model::prepareNodeDescriptor(vkglTF::Node* node, VkDescriptorSetLayout descriptorSetLayout) {
	if (node->mesh) {
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool = descriptorPool;
		descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
		descriptorSetAllocInfo.descriptorSetCount = 1;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device->logicalDevice, &descriptorSetAllocInfo, &node->mesh->uniformBuffer.descriptorSet));

		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstSet = node->mesh->uniformBuffer.descriptorSet;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &node->mesh->uniformBuffer.descriptor;

		vkUpdateDescriptorSets(device->logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
	}
	for (auto& child : node->children) {
		prepareNodeDescriptor(child, descriptorSetLayout);
	}
}

void vkglTF::Model::prepareSkinDescriptor(vkglTF::Skin* skin, VkDescriptorSetLayout descriptorSetLayout) {
    if (!skin->inverseBindMatrices.empty()) {
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = descriptorPool;
        descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device->logicalDevice, &descriptorSetAllocInfo, &skin->skinBuffer.descriptorSet));

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = skin->skinBuffer.descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pBufferInfo = &skin->skinBuffer.descriptor;

        vkUpdateDescriptorSets(device->logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
    }
}

std::pair<glm::vec3,glm::vec3> getExtremeVec3(void * _data,size_t _count)
{
    auto ptr = static_cast<glm::vec3*>(_data);
    glm::vec3 min(std::numeric_limits<float>::max()),max(std::numeric_limits<float>::min());
    for(size_t index = 0; index < _count; index++)
    {
        if(min.x > ptr[index].x) min.x = ptr[index].x;
        if(min.y > ptr[index].y) min.y = ptr[index].y;
        if(min.z > ptr[index].z) min.z = ptr[index].z;

        if(max.x < ptr[index].x) max.x = ptr[index].x;
        if(max.y < ptr[index].y) max.y = ptr[index].y;
        if(max.z < ptr[index].z) max.z = ptr[index].z;
    }
    return std::pair<glm::vec3,glm::vec3>{min, max};
}

std::pair<glm::vec2,glm::vec2> getExtremeVec2(void * _data,size_t _count)
{
    auto ptr = static_cast<glm::vec2*>(_data);
    glm::vec2 min(std::numeric_limits<float>::max()),max(std::numeric_limits<float>::min());
    for(size_t index = 0; index < _count; index++)
    {
        if(min.x > ptr[index].x) min.x = ptr[index].x;
        if(min.y > ptr[index].y) min.y = ptr[index].y;

        if(max.x < ptr[index].x) max.x = ptr[index].x;
        if(max.y < ptr[index].y) max.y = ptr[index].y;
    }
    return std::pair<glm::vec2,glm::vec2>{min, max};
}

std::pair<glm::vec4,glm::vec4> getExtremeVec4(void * _data,size_t _count)
{
    auto ptr = static_cast<glm::vec4*>(_data);
    glm::vec4 min(std::numeric_limits<float>::max()),max(std::numeric_limits<float>::min());
    for(size_t index = 0; index < _count; index++)
    {
        if(min.x > ptr[index].x) min.x = ptr[index].x;
        if(min.y > ptr[index].y) min.y = ptr[index].y;
        if(min.z > ptr[index].z) min.z = ptr[index].z;
        if(min.w > ptr[index].w) min.w = ptr[index].w;

        if(max.x < ptr[index].x) max.x = ptr[index].x;
        if(max.y < ptr[index].y) max.y = ptr[index].y;
        if(max.z < ptr[index].z) max.z = ptr[index].z;
        if(max.w < ptr[index].w) max.w = ptr[index].w;
    }
    return std::pair<glm::vec4,glm::vec4>{min, max};
}

template <class T>
std::pair<T, T> getExtreme(void * _data, size_t _count){
    auto ptr = static_cast<T*>(_data);
    auto ptr_3 = static_cast<glm::vec3*>(_data);
    auto ptr_4 = static_cast<glm::vec4*>(_data);
    float max_x = std::numeric_limits<float>::min(), min_x = std::numeric_limits<float>::max(),
    max_y = std::numeric_limits<float>::min(), min_y = std::numeric_limits<float>::max(),
    max_z = std::numeric_limits<float>::min(), min_z = std::numeric_limits<float>::max(),
    max_w = std::numeric_limits<float>::min(), min_w = std::numeric_limits<float>::max();

    for(size_t index = 0; index < _count; index++)
    {
        min_x = glm::min(min_x, ptr[index].x);
        max_x = glm::max(max_x, ptr[index].x);

        min_y = glm::min(min_y, ptr[index].y);
        max_y = glm::max(max_y, ptr[index].y);

        if constexpr(std::is_same_v<T, glm::vec4>)
        {
            min_z = glm::min(min_z, ptr_4[index].z);
            max_z = glm::max(max_z, ptr_4[index].z);
            min_w = glm::min(min_w, ptr_4[index].w);
            max_w = glm::max(max_w, ptr_4[index].w);
        }else if constexpr(std::is_same_v<T, glm::vec3>)
        {
            min_z = glm::min(min_z, ptr_3[index].z);
            max_z = glm::max(max_z, ptr_3[index].z);
        }
    }
    return std::pair<T, T>{T{glm::vec4(min_x, min_y, min_z, min_w)}, T{glm::vec4(max_x, max_y, max_z, max_w)}};
}


void vkglTF::Model::writeMeshToFile(const std::string& _filename,
                                    size_t _count,
                                    const std::vector<void*>& _points,
                                    const std::vector<uint32_t>& _indices,
                                    const std::vector<VertexComponent>& _components) {
    tinygltf::Model m;
    tinygltf::Scene scene;
    tinygltf::Mesh mesh;
    tinygltf::Primitive primitive;
    tinygltf::Node node;
    tinygltf::Buffer index_buffer;
    tinygltf::BufferView index_view;
    tinygltf::Accessor index_access;
    tinygltf::Asset asset;

    size_t index_size = _indices.size() * sizeof(uint32_t);
    index_buffer.data.resize(index_size);
    memcpy(index_buffer.data.data(), _indices.data(), index_size);

    index_view.buffer = 0;
    index_view.byteOffset = 0;
    index_view.byteLength = index_size;
    index_view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    index_access.bufferView = 0;
    index_access.byteOffset = 0;
    index_access.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    index_access.count = _indices.size();
    index_access.type = TINYGLTF_TYPE_SCALAR;
    index_access.maxValues = {static_cast<double>(_count - 1)};
    index_access.minValues = {0};

    primitive.indices = 0;                 // The index of the accessor for the vertex indices
    primitive.material = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    m.buffers.push_back(index_buffer);
    m.bufferViews.push_back(index_view);
    m.accessors.push_back(index_access);

    for(size_t index = 0; index < _components.size(); index++)
    {
        tinygltf::Buffer buffer;
        tinygltf::BufferView view;
        tinygltf::Accessor access;
        switch (_components[index]) {
            case VertexComponent::Position:
            {
                primitive.attributes["POSITION"] = index + 1;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec3);
//                auto pair = getExtremeVec3(_points[index], _count);
                auto pair = getExtreme<glm::vec3>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec3);
                view.buffer = index + 1;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec3;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index + 1;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC3;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z};
                break;
            }
            case VertexComponent::Normal:
            {
                primitive.attributes["NORMAL"] = index + 1;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec3);
//                auto pair = getExtremeVec3(_points[index], _count);
                auto pair = getExtreme<glm::vec3>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec3);
                view.buffer = index + 1;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec3;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index + 1;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC3;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z};
                break;
            }

            case VertexComponent::UV:
            {
                primitive.attributes["TEXCOORD_0"] = index + 1;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec2);
                auto pair = getExtreme<glm::vec2>(_points[index], _count);
//                auto pair = getExtremeVec2(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec2);
                view.buffer = index + 1;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec2;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index + 1;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC2;
                access.maxValues = {pair.second.x, pair.second.y};
                access.minValues = {pair.first.x, pair.first.y};
                break;
            }

            case VertexComponent::Color:
            {
                primitive.attributes["COLOR_0"] = index + 1;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index + 1;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index + 1;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }

            case VertexComponent::Tangent:
            {
                primitive.attributes["TANGENT"] = index + 1;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index + 1;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index + 1;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }

            case VertexComponent::Joint0:
            {
                primitive.attributes["JOINTS_0"] = index + 1;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index + 1;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index + 1;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }

            case VertexComponent::Weight0:
            {
                primitive.attributes["WEIGHTS_0"] = index + 1;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index + 1;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index + 1;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }
            default:{
                break;
            }

        }
        m.buffers.push_back(buffer);
        m.bufferViews.push_back(view);
        m.accessors.push_back(access);
    }

    mesh.primitives.push_back(primitive);

    // Other tie ups
    node.mesh = 0;
    scene.nodes.push_back(0); // Default scene

    // Define the asset. The version is required
    asset.version = "2.0";
    asset.generator = "tinygltf";

    // Now all that remains is to tie back all the loose objects into the
    // our single model.
    m.scenes.push_back(scene);
    m.meshes.push_back(mesh);
    m.nodes.push_back(node);

    m.asset = asset;

    // Create a simple material
    tinygltf::Material mat;
    mat.pbrMetallicRoughness.baseColorFactor = {1.0f, 0.9f, 0.9f, 1.0f};
    mat.doubleSided = true;
    m.materials.push_back(mat);

    // Save it to a file
    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, _filename,
                              true, // embedImages
                              true, // embedBuffers
                              true, // pretty print
                              false); // write binary
}

void vkglTF::Model::writeGeometryListToFile(const std::string& _filename,
                                            size_t _count,
                                            std::vector<void*> _points,
                                            std::vector<VertexComponent> _components,
                                            MeshDrawType _type) {
    tinygltf::Model m;
    tinygltf::Scene scene;
    tinygltf::Mesh mesh;
    tinygltf::Primitive primitive;
    tinygltf::Node node;
    tinygltf::Asset asset;

    primitive.material = 0;
    if (MeshDrawType::Line == _type) {
        primitive.mode = TINYGLTF_MODE_LINE;
    } else if (MeshDrawType::Point == _type) {
        primitive.mode = TINYGLTF_MODE_POINTS;
    }

    for(size_t index = 0; index < _components.size(); index++)
    {
        tinygltf::Buffer buffer;
        tinygltf::BufferView view;
        tinygltf::Accessor access;
        switch (_components[index]) {
            case VertexComponent::Position:
            {
                primitive.attributes["POSITION"] = index;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec3);
//                auto pair = getExtremeVec3(_points[index], _count);
                auto pair = getExtreme<glm::vec3>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec3);
                view.buffer = index;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec3;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC3;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z};
                break;
            }
            case VertexComponent::Normal:
            {
                primitive.attributes["NORMAL"] = index;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec3);
                auto pair = getExtreme<glm::vec3>(_points[index], _count);
//                auto pair = getExtremeVec3(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec3);
                view.buffer = index;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec3;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC3;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z};
                break;
            }

            case VertexComponent::UV:
            {
                primitive.attributes["TEXCOORD_0"] = index;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec2);
                auto pair = getExtreme<glm::vec2>(_points[index], _count);
//                auto pair = getExtremeVec2(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec2);
                view.buffer = index;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec2;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC2;
                access.maxValues = {pair.second.x, pair.second.y};
                access.minValues = {pair.first.x, pair.first.y};
                break;
            }

            case VertexComponent::Color:
            {
                primitive.attributes["COLOR_0"] = index;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }

            case VertexComponent::Tangent:
            {
                primitive.attributes["TANGENT"] = index;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }

            case VertexComponent::Joint0:
            {
                primitive.attributes["JOINTS_0"] = index;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }

            case VertexComponent::Weight0:
            {
                primitive.attributes["WEIGHTS_0"] = index;  // The index of the accessor for positions
                buffer.data.resize(_count * one_vec4);
//                auto pair = getExtremeVec4(_points[index], _count);
                auto pair = getExtreme<glm::vec4>(_points[index], _count);
                memcpy(buffer.data.data(), _points[index], _count * one_vec4);
                view.buffer = index;
                view.byteOffset = 0;
                view.byteLength = _count * one_vec4;
                view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

                access.bufferView = index;
                access.byteOffset = 0;
                access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                access.count = _count;
                access.type = TINYGLTF_TYPE_VEC4;
                access.maxValues = {pair.second.x, pair.second.y, pair.second.z, pair.second.w};
                access.minValues = {pair.first.x, pair.first.y, pair.first.z, pair.first.w};
                break;
            }
            default:{
                break;
            }

        }
        m.buffers.push_back(buffer);
        m.bufferViews.push_back(view);
        m.accessors.push_back(access);
    }

    mesh.primitives.push_back(primitive);

    // Other tie ups
    node.mesh = 0;
    scene.nodes.push_back(0); // Default scene

    // Define the asset. The version is required
    asset.version = "2.0";
    asset.generator = "tinygltf";

    // Now all that remains is to tie back all the loose objects into the
    // our single model.
    m.scenes.push_back(scene);
    m.meshes.push_back(mesh);
    m.nodes.push_back(node);

    m.asset = asset;

    // Create a simple material
    tinygltf::Material mat;
    mat.pbrMetallicRoughness.baseColorFactor = {1.0f, 0.9f, 0.9f, 1.0f};
    mat.doubleSided = true;
    m.materials.push_back(mat);

    // Save it to a file
    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, _filename,
                              false, // embedImages
                              true, // embedBuffers
                              true, // pretty print
                              false); // write binary
}

vkglTF::InstanceMesh::InstanceMesh(Device *device, size_t instanceCount) {
    this->device = device;
    this->instanceCount = instanceCount;
    this->instanceMatrix.resize(instanceCount);
    VK_CHECK_RESULT(device->createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            sizeof(glm::mat4) * instanceCount,
            &vertexBuffer.buffer,
            &vertexBuffer.memory,
            nullptr));
    VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, vertexBuffer.memory, 0, sizeof(glm::mat4) * instanceCount, 0, &vertexBuffer.mapped));
}

vkglTF::InstanceMesh::~InstanceMesh() {
    if(vertexBuffer.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device->logicalDevice, vertexBuffer.buffer, nullptr);
    }
    if(vertexBuffer.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device->logicalDevice, vertexBuffer.memory, nullptr);
    }
}

void vkglTF::InstanceNode::initInstanceMesh(Device* _device, size_t _count) {
    instanceIndex.resize(_count);
    for (size_t i = 0; i < _count; i++) {
        instanceIndex[i] = i;
    }
    instanceMesh = new InstanceMesh(_device, _count);
}

void vkglTF::InstanceNode::resetMatrix() {
    translation.assign(instanceCount, initialTranslation);
    scale.assign(instanceCount, initialScale);
    rotation.assign(instanceCount, initialRotation);
}

void vkglTF::InstanceNode::resetMatrix(size_t _index) {
    translation[_index] = initialTranslation;
    scale[_index] = initialScale;
    rotation[_index] = initialRotation;
}

glm::mat4 vkglTF::InstanceNode::localMatrix(size_t _index) {
    return glm::translate(glm::mat4(1.0f), translation[_index]) * glm::mat4(rotation[_index]) * glm::scale(glm::mat4(1.0f), scale[_index]) * matrix;
}

glm::mat4 vkglTF::InstanceNode::getMatrix(size_t _index) {
    glm::mat4 m = localMatrix(_index);
    vkglTF::InstanceNode *p = parent;
    while (p) {
        m = p->localMatrix(_index) * m;
        p = p->parent;
    }
    return m;
}

vkglTF::InstanceNode::~InstanceNode() {
    delete instanceMesh;
    for (auto& child : children) {
        delete child;
    }
}

void vkglTF::InstanceNode::update() {
    if (instanceMesh) {
        std::for_each(std::execution::par, instanceIndex.begin(), instanceIndex.end(),
            [&](size_t& _index){
                    glm::mat4 m = localMatrix(_index);
                    vkglTF::InstanceNode *p = parent;
                    while (p) {
                        m = p->localMatrix(_index) * m;
                        p = p->parent;
                    }
                    memcpy(&instanceMesh->instanceMatrix[_index], &m, sizeof(glm::mat4));
            });
        for (auto& child : children) {
            child->update();
        }
    }
}

void vkglTF::InstanceNode::updateSkin() {

}
