//
// Created by maybe on 2020/11/21.
//
#include "precompiledhead.h"

#include "shatterbuffer.h"
#include "Engine/Render/render.h"
#include <memory>
#include <string>
#include "string.h"
#include <set>
#include <array>

#include "Engine/Object/device.h"

namespace shatter::buffer{

    ShatterBuffer::ShatterBuffer():
    m_buffer{VK_NULL_HANDLE},
    m_memory{VK_NULL_HANDLE},
    m_device{nullptr},
    m_buffer_size{0},
    m_element_number{0}
    {
    }

    ShatterBuffer* ShatterBuffer::createBuffer(VkDeviceSize _size, Buffer_Type _type, void* _data){
        auto buffer = new ShatterBuffer;
        switch(_type){
            case Buffer_Type::Vertex_Buffer:{
                if(!buffer->createVertexBuffer(_size,_data)){
                    throw std::runtime_error("create vertex buffer error!");
                }
                break;
            }
            case Buffer_Type::Index_Buffer:{
                if(!buffer->createIndexBuffer(_size,_data)){
                    throw std::runtime_error("create index buffer error!");
                }
                break;
            }
            case Buffer_Type::Uniform_Buffer:{
                if(!buffer->createUniformBuffer(_size)){
                    throw std::runtime_error("create uniform buffer error!");
                }
                break;
            }
            case Buffer_Type::Storage_Buffer:{
                if(!buffer->createStorageBuffer(_size,_data)){
                    throw std::runtime_error("create storage buffer error!");
                }
                break;
            }
            case Buffer_Type::VS_Buffer:{
                if(!buffer->createVSBuffer(_size,_data)){
                    throw std::runtime_error("create vertex&storage buffer error!");
                }
                break;
            }
            case Buffer_Type::Vertex_Host_Buffer:{
                if(!buffer->createVertexHostBuffer(_size,_data)){
                    throw std::runtime_error("create vertex&host buffer error!");
                }
                break;
            }
            case Buffer_Type::Storage_Host_Buffer:{
                if(!buffer->createStorageHostBuffer(_size,_data)){
                    throw std::runtime_error("create vertex&host buffer error!");
                }
                break;
            }
            default:{
                break;
            }
        }
        buffer->setupDescriptor();
        return buffer;
    }

    bool ShatterBuffer::createVertexBuffer(VkDeviceSize _size, void* _data){
        bool val = false;
        do{
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(_size,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);
            void *data;
            vkMapMemory(*render::ShatterRender::getRender().getDevice(),
                        stagingBufferMemory,
                        0,
                        _size,
                        0,
                        &data);
            memset(data,0,(size_t)_size);
            memcpy(data,
                   _data,
                   _size);
            vkUnmapMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory);

            createBuffer(_size,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         m_buffer,
                         m_memory);
            copyBuffer(stagingBuffer, m_buffer, _size);

            vkDestroyBuffer(*render::ShatterRender::getRender().getDevice(), stagingBuffer, nullptr);
            vkFreeMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory, nullptr);
            val = true;
        }while(false);
        return val;
    }

    bool ShatterBuffer::createIndexBuffer(VkDeviceSize _size, void* _data){
        bool val = false;
        do{
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(_size,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);
            void *data;
            vkMapMemory(*render::ShatterRender::getRender().getDevice(),
                        stagingBufferMemory,
                        0,
                        _size,
                        0,
                        &data);
            memset(data,0,(size_t)_size);
            memcpy(data,
                   _data,
                   _size);
            vkUnmapMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory);

            createBuffer(_size,
                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         m_buffer,
                         m_memory);
            copyBuffer(stagingBuffer, m_buffer, _size);

            vkDestroyBuffer(*render::ShatterRender::getRender().getDevice(), stagingBuffer, nullptr);
            vkFreeMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory, nullptr);
            val = true;
        }while(false);
        return val;
    }

    bool ShatterBuffer::createStorageBuffer(VkDeviceSize _size, void* _data){
        bool val = false;
        do{
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(_size,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);
            void *data;
            vkMapMemory(*render::ShatterRender::getRender().getDevice(),
                        stagingBufferMemory,
                        0,
                        _size,
                        0,
                        &data);
            memset(data,0,(size_t)_size);
            memcpy(data,
                   _data,
                   _size);
            vkUnmapMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory);

            createBuffer(_size,
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         m_buffer,
                         m_memory);
            copyBuffer(stagingBuffer, m_buffer, _size);

            vkDestroyBuffer(*render::ShatterRender::getRender().getDevice(), stagingBuffer, nullptr);
            vkFreeMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory, nullptr);
            val = true;
        }while(false);
        return val;
    }

    bool ShatterBuffer::createStorageHostBuffer(VkDeviceSize _size,void* _data){
        bool val = false;
        do{
            createBuffer(_size,
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         m_buffer,
                         m_memory);
            if(_data)
            {
                void *data;
                vkMapMemory(*render::ShatterRender::getRender().getDevice(),
                            m_memory,
                            0,
                            _size,
                            0,
                            &data);
                memset(data,0,(size_t)_size);
                memcpy(data,
                       _data,
                       _size);
                vkUnmapMemory(*render::ShatterRender::getRender().getDevice(), m_memory);
            }

            val = true;
        }while(false);
        return val;
    }

    bool ShatterBuffer::createVSBuffer(VkDeviceSize _size,void* _data){
        bool val = false;
        do{
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(_size,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);
            void *data;
            vkMapMemory(*render::ShatterRender::getRender().getDevice(),
                        stagingBufferMemory,
                        0,
                        _size,
                        0,
                        &data);
            memset(data,0,(size_t)_size);
            memcpy(data,
                   _data,
                   _size);
            vkUnmapMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory);

            createBuffer(_size,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         m_buffer,
                         m_memory);
            copyBuffer(stagingBuffer, m_buffer, _size);

            vkDestroyBuffer(*render::ShatterRender::getRender().getDevice(), stagingBuffer, nullptr);
            vkFreeMemory(*render::ShatterRender::getRender().getDevice(), stagingBufferMemory, nullptr);
            val = true;
        }while(false);
        return val;
    }

    bool ShatterBuffer::createVertexHostBuffer(VkDeviceSize _size,void* _data) {
        bool val = false;
        do{
            createBuffer(_size,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         m_buffer,
                         m_memory);
            if(_data)
            {
                void *data;
                vkMapMemory(*render::ShatterRender::getRender().getDevice(),
                            m_memory,
                            0,
                            _size,
                            0,
                            &data);
                memset(data,0,(size_t)_size);
                memcpy(data,
                       _data,
                       _size);
                vkUnmapMemory(*render::ShatterRender::getRender().getDevice(), m_memory);
            }

            val = true;
        }while(false);
        return val;
    }

    bool ShatterBuffer::createUniformBuffer(VkDeviceSize _size){
        bool val = false;
        do{
            createBuffer(_size,
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         m_buffer,
                         m_memory);
            val = true;
        }while(false);
        return val;
    }

    void ShatterBuffer::createBuffer(VkDeviceSize size,
                                     VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags properties,
                                     VkBuffer &buffer,
                                     VkDeviceMemory &bufferMemory){
        m_buffer_size = size;
        VkBufferCreateInfo bufferInfo = {};
        std::array<uint32_t,2> queueFamilyIndices = {uint32_t(getIndices().computeFamily),uint32_t(getIndices().graphicsFamily)};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = VK_NULL_HANDLE;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(*render::ShatterRender::getRender().getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*render::ShatterRender::getRender().getDevice(),
                                      buffer,
                                      &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = render::ShatterRender::getRender().findMemoryType(memRequirements.memoryTypeBits,
                                                                                      properties);

        if (vkAllocateMemory(*render::ShatterRender::getRender().getDevice(),
                             &allocInfo,
                             nullptr,
                             &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
        vkBindBufferMemory(*render::ShatterRender::getRender().getDevice(),
                           buffer,
                           bufferMemory,
                           0);
    }

    VkResult ShatterBuffer::map(VkDeviceSize size, VkDeviceSize offset)
    {
        if(mapped == nullptr)
        {
            return vkMapMemory(Device::getDevice().logicalDevice, m_memory, offset, size, 0, &mapped);
        }
        return VK_SUCCESS;
    }

    void ShatterBuffer::unmap(){
        if (mapped)
        {
            vkUnmapMemory(Device::getDevice().logicalDevice, m_memory);
            mapped = nullptr;
        }
    }

    VkResult ShatterBuffer::flush(VkDeviceSize size , VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(Device::getDevice().logicalDevice, 1, &mappedRange);
    }

    void ShatterBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    }

    ShatterBuffer::~ShatterBuffer(){
        if(!m_released)
        {
            release();
        }
    }

    void ShatterBuffer::release(){
//        if(!m_released)
//        {
//            if(VK_NULL_HANDLE != m_buffer){
                vkDestroyBuffer(*render::ShatterRender::getRender().getDevice(),
                                m_buffer,
                                nullptr);
//            }

//            if(VK_NULL_HANDLE != m_memory){
                vkFreeMemory(*render::ShatterRender::getRender().getDevice(),
                             m_memory,
                             nullptr);
//            }
            m_released = true;
//        }
    }

    void ShatterBuffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {
        descriptor.offset = offset;
        descriptor.buffer = m_buffer;
        descriptor.range = size;
    }

    VkResult ShatterBuffer::bind(VkDeviceSize offset) {
        return vkBindBufferMemory(*m_device, m_buffer, m_memory, offset);
    }


}




