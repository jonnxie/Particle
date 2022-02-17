//
// Created by AnWell on 2021/6/23.
//

#include "device.h"
#include "Engine/Render/shatter_render_include.h"
#include "Engine/Buffer/shatterbufferinclude.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_macro.h"

static std::mutex device_mutex;
static bool if_geted = false;

using namespace shatter::render;

Device &Device::getDevice() {
    static Device device;
    std::lock_guard<std::mutex> guard_mutex(device_mutex);
    if(!if_geted){
        if_geted = true;
        device.logicalDevice = *ShatterRender::getRender().getDevice();
        device.physicalDevice = ShatterRender::getRender().physicalDevice;
    }
    return device;
}

VkResult Device::createBuffer(VkBufferUsageFlags _bufferUsage, VkMemoryPropertyFlags _property, uint64_t _size, VkBuffer *_buffer,
                     VkDeviceMemory *_memory, void *_data) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 _size,
                 &stagingBuffer,
                 &stagingBufferMemory);
    void *data;
    vkMapMemory(logicalDevice,
                stagingBufferMemory,
                0,
                _size,
                0,
                &data);
    memset(data,0,(size_t)_size);
    memcpy(data,
           _data,
           _size);
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    createBuffer(_bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 _property,
                 _size,
                 _buffer,
                 _memory);

    VkCommandBuffer commandBuffer = ShatterRender::getRender().beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.size = _size;
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, *_buffer, 1, &copyRegion);

    ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
    return VK_SUCCESS;
}

VkResult Device::createBuffer(VkBufferUsageFlags _bufferUsage, VkMemoryPropertyFlags _property, uint64_t _size, VkBuffer *_buffer,
                     VkDeviceMemory *_memory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = VK_NULL_HANDLE;
    bufferInfo.size = _size;
    bufferInfo.usage = _bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, _buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice,*_buffer,&memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = getMemoryType(memRequirements.memoryTypeBits, _property);

    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if(_bufferUsage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT){
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        allocInfo.pNext = &allocFlagsInfo;
    }

    if (vkAllocateMemory(logicalDevice,
                         &allocInfo,
                         nullptr,
                         _memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    vkBindBufferMemory(logicalDevice,*_buffer,*_memory,0);
    return VK_SUCCESS;
}

VkResult Device::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, ShatterBuffer *buffer, VkDeviceSize size, void *data)
{
    buffer->m_device = &logicalDevice;

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = tool::bufferCreateInfo(usageFlags, size);
    VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &buffer->m_buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = tool::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(logicalDevice, buffer->m_buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &buffer->m_memory));

    buffer->alignment = memReqs.alignment;
    buffer->m_buffer_size = size;
    buffer->usageFlags = usageFlags;
    buffer->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        VK_CHECK_RESULT(buffer->map());
        memcpy(buffer->mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            buffer->flush();

        buffer->unmap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    buffer->setupDescriptor();

    // Attach the memory to the buffer object
    return buffer->bind();
}


VkDeviceMemory Device::createBuffer(VkBufferUsageFlags _bufferUsage, VkMemoryPropertyFlags _property, VkBuffer *_buffer,
                              uint64_t _size) {
    VkDeviceMemory tmp_memory;
    createBuffer(_bufferUsage,_property,_size,_buffer,&tmp_memory);
    return tmp_memory;
}

void Device::copyBuffer(ShatterBuffer *src, ShatterBuffer *dst, VkQueue queue, VkBufferCopy *copyRegion) {
    assert(dst->m_buffer_size <= src->m_buffer_size);
    assert(src->m_buffer);
//    VkCommandBuffer  = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkCommandBuffer copyCmd = ShatterRender::getRender().beginSingleTimeCommands();

    VkBufferCopy bufferCopy{};
    if (copyRegion == nullptr)
    {
        bufferCopy.size = src->m_buffer_size;
    }
    else
    {
        bufferCopy = *copyRegion;
    }

    vkCmdCopyBuffer(copyCmd, src->m_buffer, dst->m_buffer, 1, &bufferCopy);

    ShatterRender::getRender().endSingleTimeCommands(copyCmd);

//    flushCommandBuffer(copyCmd, queue);
}

#define SingleDevice Device::getDevice()
