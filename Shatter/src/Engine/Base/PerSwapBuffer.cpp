//
// Created by jonnxie on 2022/5/8.
//

#include "precompiledhead.h"
#include "PerSwapBuffer.h"
#include "Engine/Item/shatter_enum.h"
#include MacroCatalog
#include "Engine/Buffer/shatterbuffer.h"
#include "Engine/Pool/bpool.h"
#include AppCatalog

auto PerSwapBuffer::getSwapBuffer(const std::string _name) {
    int swapChainIndex = SingleAPP.getWorkImageIndex();
    return SingleBPool.getBuffer(tool::combine(_name, swapChainIndex),Buffer_Type::Uniform_Buffer);
}

void PerSwapBuffer::createSwapBuffer(const std::string _name, VkDeviceSize _size, Buffer_Type _type, void *data) {
    int imageCount = SingleAPP.getSwapChainCount();
    switch (_type) {
        case Buffer_Type::Vertex_Buffer:
            break;
        case Buffer_Type::Index_Host_Buffer:
            break;
        case Buffer_Type::Index_Buffer:
            break;
        case Buffer_Type::Uniform_Buffer:{
            for (int i = 0; i < imageCount; i++)
            {
                SingleBPool.createUniformBuffer(tool::combine(_name, i), _size);
            }
            break;
        }
        case Buffer_Type::Storage_Buffer:
            break;
        case Buffer_Type::VS_Buffer:
            break;
        case Buffer_Type::Vertex_Host_Buffer:
            break;
        case Buffer_Type::Storage_Host_Buffer:
            break;
    }

}

