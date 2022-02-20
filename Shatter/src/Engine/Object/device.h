//
// Created by AnWell on 2021/6/23.
//

#ifndef SHATTER_ENGINE_DEVICE_H
#define SHATTER_ENGINE_DEVICE_H

#include <vulkan/vulkan.h>
#include <mutex>
#include <memory>
#include <string>

namespace Shatter::buffer{
    class ShatterBuffer;
};

using namespace Shatter::buffer;

class Device {
public:
    static Device& getDevice();
    Device(const Device&) = delete;
    Device& operator = (const Device&) = delete;
    VkDevice operator()() const {
        return logicalDevice;
    }

    uint32_t getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    VkResult createBuffer(VkBufferUsageFlags _bufferUsage, VkMemoryPropertyFlags _property, uint64_t _size, VkBuffer* _buffer, VkDeviceMemory* _memory, void* _data);

    VkResult createBuffer(VkBufferUsageFlags _bufferUsage, VkMemoryPropertyFlags _property, uint64_t _size, VkBuffer* _buffer, VkDeviceMemory* _memory);

    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, ShatterBuffer *buffer, VkDeviceSize size, void *data = nullptr);

    VkDeviceMemory createBuffer(VkBufferUsageFlags _bufferUsage, VkMemoryPropertyFlags _property, VkBuffer* _buffer, uint64_t _size);

    static void copyBuffer(ShatterBuffer *src, ShatterBuffer *dst, VkQueue queue, VkBufferCopy *copyRegion = nullptr);

    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;
private:
    Device(): logicalDevice(VK_NULL_HANDLE){};
};

#define SingleDevice Device::getDevice()

#endif //SHATTER_ENGINE_DEVICE_H
