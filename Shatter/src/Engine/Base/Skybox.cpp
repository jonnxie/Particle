//
// Created by jonnxie on 2021/9/28.
//
#include "precompiledhead.h"

#include "Skybox.h"
#include ModelSetCatalog
#include BPoolCatalog
#include TaskCatalog
#include SetPoolCatalog
#include "Engine/Buffer/shattertexture.h"
#include "Engine/Render/shatter_render_include.h"

Skybox::Skybox(const std::vector<std::string>& _files){
    m_model = new vkglTF::Model;
    m_cube = _files;
    m_model->loadFromFileDefault(tool::combineModel("cube.gltf"), &Device::getDevice(), VkQueue{},m_point,m_index);
    size_t count = m_point.size();
    std::vector<glm::vec3> pos_vec(count);
    std::vector<glm::vec3> normal_vec(count);
    std::vector<glm::vec2> uv_vec(count);

    for(size_t index = 0; index < count; index++)
    {
        pos_vec[index] = m_point[index].pos;
        normal_vec[index] = m_point[index].normal;
        uv_vec[index] = m_point[index].uv;
    }
    std::vector<void*> data_vec{pos_vec.data(), normal_vec.data(), uv_vec.data()};
//    m_model->writeFile("writeTest.gltf",
//                       m_point.size(),
//                       data_vec,
//                       m_index,
//                       std::vector<vkglTF::VertexComponent>{vkglTF::VertexComponent::Position,
//                                                            vkglTF::VertexComponent::Normal,
//                                                            vkglTF::VertexComponent::UV}
//                       );
    init();
}

void Skybox::constructG(){
    BPool::getPool().createVertexBuffer("skybox",sizeof(GltfPoint) * m_point.size(),m_point.data());//Particle data
    BPool::getPool().createIndexBuffer("skybox",sizeof(uint32_t) * m_index.size(),m_index.data());//Particle data
    BPool::getPool().createCubeTexture("skybox",m_cube);

    TaskPool::pushTask("Skybox",[](){
        std::array<VkWriteDescriptorSet,2> mc_write = {};

        VkDescriptorBufferInfo camBuffer{SingleBPool.getBuffer("CameraPos",Buffer_Type::Uniform_Buffer)->m_buffer,
                                         0,
                                         sizeof(glm::vec3)};

        mc_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[0].pNext = VK_NULL_HANDLE;
        mc_write[0].dstSet =  SetPool::getPool()["Texture"];
        mc_write[0].dstBinding = 0;
        mc_write[0].dstArrayElement = 0;
        mc_write[0].descriptorCount = 1;
        mc_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mc_write[0].pImageInfo = VK_NULL_HANDLE;
        mc_write[0].pBufferInfo = &camBuffer;

        auto cube_tex = BPool::getPool().getTexture("skybox");
        VkDescriptorImageInfo img_info;
        img_info.sampler = cube_tex->getSampler();
        img_info.imageView = cube_tex->getImageView();
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        mc_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mc_write[1].pNext = VK_NULL_HANDLE;
        mc_write[1].dstSet =  SetPool::getPool()["Texture"];
        mc_write[1].dstBinding = 1;
        mc_write[1].dstArrayElement = 0;
        mc_write[1].descriptorCount = 1;
        mc_write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mc_write[1].pImageInfo = &img_info;
        mc_write[1].pBufferInfo = VK_NULL_HANDLE;

        vkUpdateDescriptorSets(Device::getDevice()(),
                               2,
                               mc_write.data(),
                               0,
                               nullptr);
    });
}

void Skybox::constructD(){
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int mc_index = ModelSetPool::getPool().malloc();

    std::vector<std::string> s_vec;
    s_vec.resize(2);
    s_vec[0]="Camera";
    s_vec[1]="Texture";

    (*dpool)[d]->prepare(glm::mat4(1.0f),
                         mc_index,
                         DrawType::Index,
                         0,
                         "skybox",
                         m_point.size(),
                         "skybox",
                         m_index.size(),
                         0,
                         "Skybox",
                         s_vec);
    TaskPool::pushUpdateTask("Skybox",[&,mc_index,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + mc_index,&(*SingleDPool)[d]->m_matrix,one_matrix);
    });
//    Shatter::app::ShatterApp::getApp().getNObjects()->push_back(d);
    SingleRender.getNObjects()->push_back(d);
    insertDObject(d);
}
