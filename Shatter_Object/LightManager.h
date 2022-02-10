//
// Created by jonnxie on 2022/1/13.
//

#ifndef SHATTER_ENGINE_LIGHTMANAGER_H
#define SHATTER_ENGINE_LIGHTMANAGER_H

#include "../Shatter_Item/shatter_enum.h"
#include UniformCatalog
#include MacroCatalog
#include <vector>
#include BPoolCatalog
#include BufferCatalog
#include SetPoolCatalog
#include DeviceCatalog

class LightManager {
public:
    static LightManager& getLight();
    LightManager(const LightManager&) = delete;
    LightManager(LightManager&&) = delete;
    LightManager& operator = (const LightManager&) = delete;
    LightManager& operator = (LightManager&&) = delete;
    PassLight& operator[](size_t _index){
        if(_index >= lights.size())
        {
//            throw std::runtime_error("light buffer size is not enough big");
            lights.resize(lights.size() * 2);
            size *= 2;
            SingleBPool.freeBuffer("MultiLight",Buffer_Type::Storage_Buffer);
            SingleBPool.createStorageBuffer("MultiLight",size,lights.data());
            (*SingleBPool.getBuffer("MultiLight",Buffer_Type::Storage_Buffer)).map();
            std::array<VkWriteDescriptorSet,3> mc_write = {};
            VkDescriptorBufferInfo bufferInfo;
            bufferInfo.buffer = (*SingleBPool.getBuffer("CameraPos",Buffer_Type::Uniform_Buffer))();
            bufferInfo.offset = 0;
            bufferInfo.range = 12;
            mc_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mc_write[0].pNext = VK_NULL_HANDLE;
            mc_write[0].dstSet =  SetPool::getPool()["MultiLight"];
            mc_write[0].dstBinding = 0;
            mc_write[0].dstArrayElement = 0;
            mc_write[0].descriptorCount = 1;
            mc_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            mc_write[0].pImageInfo = VK_NULL_HANDLE;
            mc_write[0].pBufferInfo = &bufferInfo;

            bufferInfo.buffer = (*SingleBPool.getBuffer("LightCount",Buffer_Type::Uniform_Buffer))();
            bufferInfo.offset = 0;
            bufferInfo.range = 4;
            mc_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mc_write[0].pNext = VK_NULL_HANDLE;
            mc_write[0].dstSet =  SetPool::getPool()["MultiLight"];
            mc_write[0].dstBinding = 1;
            mc_write[0].dstArrayElement = 0;
            mc_write[0].descriptorCount = 1;
            mc_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            mc_write[0].pImageInfo = VK_NULL_HANDLE;
            mc_write[0].pBufferInfo = &bufferInfo;

            bufferInfo.buffer = (*SingleBPool.getBuffer("MultiLight",Buffer_Type::Storage_Buffer))();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(PassLight) * lights.size();

            mc_write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mc_write[2].pNext = VK_NULL_HANDLE;
            mc_write[2].dstSet =  SetPool::getPool()["MultiLight"];
            mc_write[2].dstBinding = 2;
            mc_write[2].dstArrayElement = 0;
            mc_write[2].descriptorCount = 1;
            mc_write[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mc_write[2].pImageInfo = VK_NULL_HANDLE;
            mc_write[2].pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(Device::getDevice()(),
                                   3,
                                   mc_write.data(),
                                   0,
                                   nullptr);
        }
        return lights[_index];
    }
    void init(size_t _size);
    void initLight();
private:
    std::vector<PassLight> lights;
    size_t size{};
    bool changed = true;
private:
    LightManager() = default;
};

#define PassLightSize 32

#define SingleLM LightManager::getLight()

#endif //SHATTER_ENGINE_LIGHTMANAGER_H
