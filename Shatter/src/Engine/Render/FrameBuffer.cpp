//
// Created by AnWell on 2022/3/1.
//

#include "FrameBuffer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"

FrameBuffer *FrameBuffer::createFramebuffer(FrameBufferSpecification _spec) {
#ifdef SHATTER_GRAPHICS_VULKAN
    return new VulkanFrameBuffer(_spec);
#endif
    return nullptr;
}
