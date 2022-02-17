//
// Created by jonnxie on 2021/11/6.
//
#include "precompiledhead.h"

#include "PBRBasic.h"
#include ModelSetCatalog
#include BPoolCatalog
#include TaskCatalog
#include SetPoolCatalog
#include "Engine/Buffer/shattertexture.h"

PBRBasic::PBRBasic(const std::string& _files,int _id):m_id(_id){
    m_model = new vkglTF::Model;
    m_model->loadFromFilePositionNormal(_files, &Device::getDevice(), VkQueue{},m_point,m_index,vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::FlipY);
    const float p = 15.0f;
    m_lights[0] = glm::vec4(-p, -p*0.5f, -p, 1.0f);
    m_lights[1] = glm::vec4(-p, -p*0.5f,  p, 1.0f);
    m_lights[2] = glm::vec4( p, -p*0.5f,  p, 1.0f);
    m_lights[3] = glm::vec4( p, -p*0.5f, -p, 1.0f);
    init();
}

void PBRBasic::constructG(){
    BPool::getPool().createVertexBuffer(tool::combine("PBRBasic",m_id),sizeof(Point3d_Normal) * m_point.size(),m_point.data());//Particle data
    BPool::getPool().createIndexBuffer(tool::combine("PBRBasic",m_id),sizeof(uint32_t) * m_index.size(),m_index.data());//Particle data

    TaskPool::pushTask(tool::combine("PBRBasic",m_id),[](){
        std::array<VkWriteDescriptorSet,3> mc_write = {};

        VkDescriptorBufferInfo camBuffer{SingleBPool.getBuffer("CameraPos",Buffer_Type::Uniform_Buffer)->m_buffer,
                                         0,
                                         sizeof(glm::vec3)};

        VkDescriptorBufferInfo lightBuffer{SingleBPool.getBuffer("LightGroup",Buffer_Type::Uniform_Buffer)->m_buffer,
                                         0,
                                         sizeof(glm::vec4) * 4};

        VkDescriptorBufferInfo materialBuffer{SingleBPool.getBuffer("Material",Buffer_Type::Uniform_Buffer)->m_buffer,
                                         0,
                                         sizeof(Material)};

        mc_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[0].pNext = VK_NULL_HANDLE;
        mc_write[0].dstSet =  SingleSetPool["PhysicalBasicRender"];
        mc_write[0].dstBinding = 0;
        mc_write[0].dstArrayElement = 0;
        mc_write[0].descriptorCount = 1;
        mc_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mc_write[0].pImageInfo = VK_NULL_HANDLE;
        mc_write[0].pBufferInfo = &camBuffer;

        mc_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[1].pNext = VK_NULL_HANDLE;
        mc_write[1].dstSet =  SingleSetPool["PhysicalBasicRender"];
        mc_write[1].dstBinding = 1;
        mc_write[1].dstArrayElement = 0;
        mc_write[1].descriptorCount = 1;
        mc_write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mc_write[1].pImageInfo = VK_NULL_HANDLE;
        mc_write[1].pBufferInfo = &lightBuffer;

        mc_write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[2].pNext = VK_NULL_HANDLE;
        mc_write[2].dstSet =  SingleSetPool["PhysicalBasicRender"];
        mc_write[2].dstBinding = 2;
        mc_write[2].dstArrayElement = 0;
        mc_write[2].descriptorCount = 1;
        mc_write[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mc_write[2].pImageInfo = VK_NULL_HANDLE;
        mc_write[2].pBufferInfo = &materialBuffer;

        vkUpdateDescriptorSets(Device::getDevice()(),
                               3,
                               mc_write.data(),
                               0,
                               nullptr);
    });

    TaskPool::pushUpdateTask(tool::combine("PBRBasic",m_id),[&](float _abs_time){
        memcpy(SingleBPool.getBuffer("Material",Buffer_Type::Uniform_Buffer)->mapped, &getMaterial(), sizeof(Material));

        m_lights[0].x = glm::sin(glm::radians(_abs_time * 360.0f)) * 20.0f;
        m_lights[0].z = glm::cos(glm::radians(_abs_time * 360.0f)) * 20.0f;
        m_lights[1].x = glm::cos(glm::radians(_abs_time * 360.0f)) * 20.0f;
        m_lights[1].y = glm::sin(glm::radians(_abs_time * 360.0f)) * 20.0f;

        memcpy(SingleBPool.getBuffer("LightGroup",Buffer_Type::Uniform_Buffer)->mapped, m_lights, sizeof(glm::vec4) * 4);
    });

}

void PBRBasic::constructD(){
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int mc_index = ModelSetPool::getPool().malloc();

    std::vector<std::string> s_vec;
    s_vec.resize(2);
    s_vec[0]="Camera";
    s_vec[1]="PhysicalBasicRender";

    (*dpool)[d]->prepare(glm::mat4(1.0f),
                         mc_index,
                         DrawType::Index,
                         0,
                         tool::combine("PBRBasic",m_id),
                         m_point.size(),
                         tool::combine("PBRBasic",m_id),
                         m_index.size(),
                         0,
                         "PhysicalBasicRender",
                         s_vec);
    insertDObject(d);
}

