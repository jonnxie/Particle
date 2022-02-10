//
// Created by maybe on 2020/12/2.
//

#include "shatteruniformbuffer.h"

namespace shatter::buffer{

    std::shared_ptr<ShatterUniformBuffer> ShatterUniformBuffer::createUniformBuffer(VkDevice* device,
                                                                                    VkDeviceSize size){
        auto buffer = std::make_shared<ShatterUniformBuffer>();
        buffer->Set_Device(device);
        if(buffer->initUniformBuffer(size)){
            return buffer;
        }else{
            throw std::runtime_error("create uniform_buffer error!");
        }
    }

    bool ShatterUniformBuffer::initUniformBuffer(VkDeviceSize size){
        bool val = false;
        do{
            createBuffer(size,
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         m_buffer,
                         m_memory);
            val = true;
        }while(false);
        return val;
    }

    ShatterUniformBuffer::ShatterUniformBuffer()
    {
    };

};




