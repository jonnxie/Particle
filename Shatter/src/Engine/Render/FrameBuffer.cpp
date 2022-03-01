//
// Created by AnWell on 2022/3/1.
//

#include "FrameBuffer.h"

#include <utility>
#include "Platform/Vulkan/VulkanFrameBuffer.h"

FrameBuffer *FrameBuffer::createFramebuffer(FrameBufferSpecification _spec) {
#ifdef SHATTER_GRAPHICS_VULKAN
    return new VulkanFrameBuffer(std::move(_spec));
#endif
    return nullptr;
}
