//
// Created by jonnxie on 2022/1/13.
//

#include "LightManager.h"
#include <mutex>
#include <array>
#include <random>
#include ConfigCatalog
#include TaskCatalog

LightManager& LightManager::getLight(){
    static bool ready = false;
    static std::mutex lock;
    static LightManager manager;
    if(!ready)
    {
        manager.init(Config::getConfig("LightInitCount"));
    }
    return manager;
}

void LightManager::init(size_t _size){
    lights.resize(_size);
    initLight();
    size = sizeof(PassLight) * lights.size();
    SingleBPool.createStorageHostBuffer("MultiLight",size,lights.data());
    (*SingleBPool.getBuffer("MultiLight",Buffer_Type::Storage_Host_Buffer)).map();

    SingleBPool.createUniformBuffer("LightCount",4);
    (*SingleBPool.getBuffer("LightCount",Buffer_Type::Uniform_Buffer)).map();

    std::array<VkWriteDescriptorSet,3> mc_write = {};

    VkDescriptorBufferInfo posBufferInfo;
    posBufferInfo.buffer = (*SingleBPool.getBuffer("CameraPos",Buffer_Type::Uniform_Buffer))();
    posBufferInfo.offset = 0;
    posBufferInfo.range = 12;
    mc_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mc_write[0].pNext = VK_NULL_HANDLE;
    mc_write[0].dstSet =  SetPool::getPool()["MultiLight"];
    mc_write[0].dstBinding = 0;
    mc_write[0].dstArrayElement = 0;
    mc_write[0].descriptorCount = 1;
    mc_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mc_write[0].pImageInfo = VK_NULL_HANDLE;
    mc_write[0].pBufferInfo = &posBufferInfo;

    VkDescriptorBufferInfo LCBufferInfo;
    LCBufferInfo.buffer = (*SingleBPool.getBuffer("LightCount",Buffer_Type::Uniform_Buffer))();
    LCBufferInfo.offset = 0;
    LCBufferInfo.range = 4;
    mc_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mc_write[1].pNext = VK_NULL_HANDLE;
    mc_write[1].dstSet =  SetPool::getPool()["MultiLight"];
    mc_write[1].dstBinding = 1;
    mc_write[1].dstArrayElement = 0;
    mc_write[1].descriptorCount = 1;
    mc_write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mc_write[1].pImageInfo = VK_NULL_HANDLE;
    mc_write[1].pBufferInfo = &LCBufferInfo;

    VkDescriptorBufferInfo MLBufferInfo;
    MLBufferInfo.buffer = (*SingleBPool.getBuffer("MultiLight",Buffer_Type::Storage_Buffer))();
    MLBufferInfo.offset = 0;
    MLBufferInfo.range = sizeof(PassLight) * lights.size();

    mc_write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mc_write[2].pNext = VK_NULL_HANDLE;
    mc_write[2].dstSet =  SetPool::getPool()["MultiLight"];
    mc_write[2].dstBinding = 2;
    mc_write[2].dstArrayElement = 0;
    mc_write[2].descriptorCount = 1;
    mc_write[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    mc_write[2].pImageInfo = VK_NULL_HANDLE;
    mc_write[2].pBufferInfo = &MLBufferInfo;

    vkUpdateDescriptorSets(Device::getDevice()(),
                           3,
                           mc_write.data(),
                           0,
                           nullptr);

    TaskPool::pushUpdateTask("MultiLightUpdateTask",[&](float _absTime){
        if(changed)
        {
            size_t count = lights.size();
            memcpy((*SingleBPool.getBuffer("LightCount",Buffer_Type::Uniform_Buffer)).mapped,
                   &count,4);
            size_t local_size = count * PassLightSize;
            memcpy((*SingleBPool.getBuffer("MultiLight",Buffer_Type::Storage_Host_Buffer)).mapped,
                   lights.data(),local_size);
            changed = false;
        }
    });
}

void LightManager::initLight() {
    std::vector<glm::vec3> colors =
            {
                    glm::vec3(1.0f, 1.0f, 1.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec3(1.0f, 1.0f, 0.0f),
            };

    std::default_random_engine rndGen( (unsigned)time(nullptr));
    std::uniform_real_distribution<float> rndDist(-1.0f, 1.0f);
    std::uniform_int_distribution<uint32_t> rndCol(0, static_cast<uint32_t>(colors.size()-1));

    for (auto& light : lights)
    {
        light.position = glm::vec4(rndDist(rndGen) * 6.0f, 0.25f + std::abs(rndDist(rndGen)) * 4.0f, rndDist(rndGen) * 6.0f, 1.0f);
        light.color = colors[rndCol(rndGen)];
        light.radius = 1.0f + std::abs(rndDist(rndGen));
    }
}

