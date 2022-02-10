//
// Created by jonnxie on 2021/10/25.
//

#include "basic.h"
#include SetPoolCatalog
#include PPoolCatalog
#include TaskCatalog
#include BPoolCatalog
#include ModelSetCatalog
#include BufferCatalog
#include PipelineCatalog
#include MathCatalog
#include OffScreenCatalog

Basic::Basic(const std::string& _files,glm::vec3 _pos,glm::vec3 _rotationAxis,float _angle,glm::vec3 _scale,int _id)
{
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromFile(_files,&SingleDevice,VkQueue{},glTFLoadingFlags);
    glm::mat4 s = glm::scale(glm::mat4(1.0f),_scale);
    glm::mat4 r;
    r = glm::rotate(glm::mat4(1.0f),_angle,_rotationAxis);
    glm::mat4 t = glm::translate(glm::mat4(1.0f), _pos);
    m_world = t * s * r;
    m_id = _id;
    init();
}

void Basic::constructG()
{

}

void Basic::constructD()
{
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int modelIndex = ModelSetPool::getPool().malloc();

    (*dpool)[d]->m_model_index = modelIndex;
    (*dpool)[d]->m_matrix = m_world;
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->m_gGraphics = [&,modelIndex](VkCommandBuffer _cb){
        VkViewport tmp = getViewPort();
        vkCmdSetViewport(_cb,0,1,&tmp);

        VkRect2D scissor = getScissor();
        vkCmdSetScissor(_cb,0,1,&scissor);

        auto set_pool = MPool<VkDescriptorSet>::getPool();
        std::vector<VkDescriptorSet> sets{(*(*set_pool)[modelIndex]),
                                          SingleSetPool["Camera"]};
        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()["Build"]->getPipelineLayout(),
                                0,
                                sets.size(),
                                sets.data(),
                                0,
                                VK_NULL_HANDLE);

        // Mesh containing the LODs
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()["Build"]->getPipeline());
        m_model->draw(_cb);
    };
    insertDObject(d);
}
