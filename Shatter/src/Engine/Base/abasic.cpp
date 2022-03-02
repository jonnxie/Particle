//
// Created by jonnxie on 2022/3/2.
//

#include "precompiledhead.h"
#include "abasic.h"
#include <utility>
#include SetPoolCatalog
#include PPoolCatalog
#include TaskCatalog
#include BPoolCatalog
#include ModelSetCatalog
#include BufferCatalog
#include PipelineCatalog
#include MathCatalog
#include OffScreenCatalog

ABasic::ABasic(const std::string& _files,
             glm::vec3 _pos,
             glm::vec3 _rotationAxis,
             float _angle,
             glm::vec3 _scale,
             int _id,
             std::string  _pipeline,
             std::vector<std::string>  _sets):
        m_pipeline(std::move(_pipeline)),
        m_sets{std::move(_sets)}
{
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_model->loadFromFile(_files,&SingleDevice,VkQueue{},glTFLoadingFlags);
    m_scale = glm::scale(glm::mat4(1.0f),_scale);
    m_rotate = glm::rotate(glm::mat4(1.0f),_angle,_rotationAxis);
    m_translation = glm::translate(glm::mat4(1.0f), _pos);
    m_world = m_translation * m_scale * m_rotate;
    m_id = _id;
    init();
}

void ABasic::constructG()
{

}

void ABasic::constructD()
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
        std::vector<VkDescriptorSet> sets{(*(*set_pool)[modelIndex])};
        for(auto & s: m_sets)
        {
            sets.emplace_back(SingleSetPool[s]);
        }
        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()[m_pipeline]->getPipelineLayout(),
                                0,
                                sets.size(),
                                sets.data(),
                                0,
                                VK_NULL_HANDLE);

        // Mesh containing the LODs
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()[m_pipeline]->getPipeline());
        m_model->draw(_cb);
    };
    insertDObject(d);
    if(updateFunc)
    {
        TaskPool::pushUpdateTask(tool::combine("BasicAnimation",m_id),[&,modelIndex,d](float _abs_time){
            m_model->updateAnimation(m_animation_index, _abs_time);
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + modelIndex,&(*SingleDPool)[d]->m_matrix,one_matrix);
        });
    }
    SingleRender.getNObjects()->push_back(d);
}
