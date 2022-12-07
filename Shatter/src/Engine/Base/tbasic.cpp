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
#include ManipulateCatalog

TBasic::TBasic(const std::string& _files,glm::vec3 _pos,glm::vec3 _rotationAxis,float _angle,glm::vec3 _scale,int _id)
{
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromFile(_files,&SingleDevice,VkQueue{},glTFLoadingFlags);
    m_manipulate = std::make_unique<Manipulate>();
    m_manipulate->setScale(_scale);
    m_manipulate->_rotationAxis = _rotationAxis;
    m_manipulate->_angle = _angle;
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = _pos;
    m_manipulate->setChanged(true);
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
    SingleSetPool.AllocateDescriptorSets(std::vector<Set_id>{"TransparentInput"}, &m_set);

    VkWriteDescriptorSet writes = {};

    auto glass = SingleBPool.getTexture("glass");
    VkDescriptorImageInfo glassImgInfo{};
    glassImgInfo.sampler = glass->getSampler();
    glassImgInfo.imageView = glass->getImageView();
    glassImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes.pNext = VK_NULL_HANDLE;
    writes.dstSet = m_set;
    writes.dstBinding = 0;
    writes.dstArrayElement = 0;
    writes.descriptorCount = 1;
    writes.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes.pImageInfo = &glassImgInfo;
    writes.pBufferInfo = VK_NULL_HANDLE;

    vkUpdateDescriptorSets(Device::getDevice()(),
                           1,
                           &writes,
                           0,
                           nullptr);

    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int modelIndex = m_manipulate->getModelId();

    (*dpool)[d]->m_model_index = modelIndex;
    (*dpool)[d]->m_matrix = m_manipulate->getMatrix();
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->m_newDraw = [&,modelIndex](VkCommandBuffer _cb){
    tool::cmdDynamicState(_cb);
        auto set_pool = MPool<VkDescriptorSet>::getPool();
        std::vector<VkDescriptorSet> sets{(*(*set_pool)[modelIndex]),
                                          SingleSetPool["Camera"],
                                          m_set
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
    for (int i : m_dobjs)
    {
        SingleRender.pushTObjects(i);
    }
//    TaskPool::pushUpdateTask(tool::combine("TBasic",m_id),[&,modelIndex,d](float _abs_time){
//        glm::mat4* ptr = SingleBPool.getModels();
//        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
//    });
}
