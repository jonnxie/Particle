//
// Created by jonnxie on 2022/1/16.
//
#include "precompiledhead.h"

#include "tbasic.h"
#include SetPoolCatalog
#include PPoolCatalog
#include TaskCatalog
#include BPoolCatalog
#include ModelSetCatalog
#include BufferCatalog
#include PipelineCatalog
#include OffScreenCatalog
#include RenderCatalog

TBasic::TBasic(const std::string& _files,glm::vec3 _pos,glm::vec3 _rotationAxis,float _angle,glm::vec3 _scale,int _id)
{
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromFile(_files,&SingleDevice,VkQueue{},glTFLoadingFlags);
    glm::mat4 s = glm::scale(glm::mat4(1.0f),_scale);
    glm::mat4 r;
//    genRotateFromEulerAngle(&_rotation,&r);
    r = glm::rotate(glm::mat4(1.0f),_angle,_rotationAxis);
    glm::mat4 t;
    t = glm::translate(glm::mat4(1.0f), _pos);
    m_world = t * s * r;
    m_id = _id;

    init();
}

void TBasic::constructG()
{
//    TaskPool::pushTask(tool::combine("UpdateTransparentSet",m_id),[](){
//
//    });
}

void TBasic::constructD()
{
    VkDescriptorSet set;
    void AllocateDescriptorSets(const std::vector<Set_id>& _ids,VkDescriptorSet* _set);

    SingleSetPool.AllocateDescriptorSets(std::vector<Set_id>{"TransparentInput"},&set);

    std::array<VkWriteDescriptorSet,2> writes = {};
    VkDescriptorImageInfo attachImgInfo{};
    attachImgInfo.sampler = VK_NULL_HANDLE;
    attachImgInfo.imageView = SingleRender.positionAttachment->view;
    attachImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].pNext = VK_NULL_HANDLE;
    writes[0].dstSet = set;
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    writes[0].pImageInfo = &attachImgInfo;
    writes[0].pBufferInfo = VK_NULL_HANDLE;

    auto glass = SingleBPool.getTexture("glass");
    VkDescriptorImageInfo glassImgInfo{};
    glassImgInfo.sampler = glass->getSampler();
    glassImgInfo.imageView = glass->getImageView();
    glassImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].pNext = VK_NULL_HANDLE;
    writes[1].dstSet = set;
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &glassImgInfo;
    writes[1].pBufferInfo = VK_NULL_HANDLE;

    vkUpdateDescriptorSets(Device::getDevice()(),
                           writes.size(),
                           writes.data(),
                           0,
                           nullptr);

    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int modelIndex = ModelSetPool::getPool().malloc();

    (*dpool)[d]->m_model_index = modelIndex;
    (*dpool)[d]->m_matrix = m_world;
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->m_newDraw = [&,set,modelIndex](VkCommandBuffer _cb){
        VkViewport tmp = getViewPort();
        vkCmdSetViewport(_cb,0,1,&tmp);

        VkRect2D scissor = getScissor();
        vkCmdSetScissor(_cb,0,1,&scissor);

        auto set_pool = MPool<VkDescriptorSet>::getPool();
        std::vector<VkDescriptorSet> sets{(*(*set_pool)[modelIndex]),
                                          SingleSetPool["Camera"],
                                          set
        };
        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()["Transparent"]->getPipelineLayout(),
                                0,
                                sets.size(),
                                sets.data(),
                                0,
                                VK_NULL_HANDLE);

        // Mesh containing the LODs
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()["Transparent"]->getPipeline());
        m_model->draw(_cb);
    };
    insertDObject(d);
    SingleRender.getTObjects()->insert(SingleRender.getTObjects()->end(), m_dobjs.begin(), m_dobjs.end());
    TaskPool::pushUpdateTask(tool::combine("TBasic",m_id),[&,modelIndex,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
}
