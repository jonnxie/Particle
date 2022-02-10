//
// Created by maybe on 2020/11/21.
//

#ifndef SHATTER_ENGINE_SHATTERBUFFER_H
#define SHATTER_ENGINE_SHATTERBUFFER_H

#include <vulkan/vulkan.h>
#include <glfw3.h>
#include <iostream>
#include <memory>
#include "../Shatter_Item/shatter_item.h"
#include "../Shatter_Item/shatter_enum.h"

namespace shatter {
    namespace buffer {
        class ShatterBuffer : public std::enable_shared_from_this<ShatterBuffer> {
        public:
            ShatterBuffer();

            ~ShatterBuffer();

            void release();

            static ShatterBuffer* createBuffer(VkDeviceSize _size, Buffer_Type _type, void* _data);

            bool createVertexBuffer(VkDeviceSize _size,void* _data);

            bool createIndexBuffer(VkDeviceSize _size,void* _data);

            bool createStorageBuffer(VkDeviceSize _size,void* _data);

            bool createStorageHostBuffer(VkDeviceSize _size,void* _data);

            bool createUniformBuffer(VkDeviceSize _size);

            bool createVSBuffer(VkDeviceSize _size,void* _data);

            bool createVertexHostBuffer(VkDeviceSize _size,void* _data);

            void createBuffer(VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties,
                              VkBuffer &buffer,
                              VkDeviceMemory &bufferMemory);

            VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

            void unmap();

            VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

            static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

            void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

            VkResult bind(VkDeviceSize offset = 0);

        public:
            void Set_Device(VkDevice *device) { m_device = device; };

            VkBuffer Get_Buffer() { return m_buffer; };

            VkBuffer operator()(){return m_buffer;};

            VkDeviceMemory getMemory() { return m_memory; };

            const VkDeviceSize Get_Buffer_Size() { return m_buffer_size; };

            const uint32_t Get_Element_Number() { return m_element_number; };
        public:
            VkDeviceSize m_buffer_size;
            uint32_t m_element_number;
            VkDevice *m_device;
            VkBuffer m_buffer;
            VkDeviceMemory m_memory;
            VkDescriptorBufferInfo descriptor;
            VkDeviceSize alignment = 0;
            VkBufferUsageFlags usageFlags;
            VkMemoryPropertyFlags memoryPropertyFlags;
            bool m_released = false;
            void* mapped = nullptr;
        };

    };
};

#endif //SHATTER_ENGINE_SHATTERBUFFER_H
