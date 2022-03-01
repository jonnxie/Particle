//
// Created by AnWell on 2022/3/1.
//

#ifndef MAIN_VULKANFORMATFILTER_H
#define MAIN_VULKANFORMATFILTER_H

#include <vulkan/vulkan.h>
#include "Engine/Render/FormatFilter.hpp"

VkFormat getFormat(TextureFormat _format){
    switch (_format) {
        case TextureFormat::RGBA8U: {
            return VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        case TextureFormat::DEPTH24STENCIL8: {
            return VK_FORMAT_D24_UNORM_S8_UINT;
            break;
        }
        default:{
            break;
        }
    }
    return VkFormat{};
}

VkImageUsageFlags getUsage(Usage _usage)
{
    switch (_usage) {
        case Usage::Color: {
            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            break;
        }
        case Usage::Depth: {
            return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            break;
        }
        default:{
            break;
        }
    }
    return VkImageUsageFlags{};
}


#endif //MAIN_VULKANFORMATFILTER_H
