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
#include RenderCatalog

ABasic::ABasic(const std::string& _files,
             glm::vec3 _pos,
             glm::vec3 _rotationAxis,
             float _angle,
             glm::vec3 _scale,
             int _id,
             std::string  _pipeline,
             std::vector<std::string>  _sets,
             DrawObjectType _type):
        m_pipeline(std::move(_pipeline)),
        m_sets{std::move(_sets)},
        m_draw_type(_type)
{
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;
    m_scale = glm::scale(glm::mat4(1.0f),_scale);
    m_rotate = glm::rotate(glm::mat4(1.0f),_angle,_rotationAxis);
    m_translation = glm::translate(glm::mat4(1.0f), _pos);
    m_world = m_translation * m_scale * m_rotate;
    m_model->loadFromFile(_files, &SingleDevice, VkQueue{}, glTFLoadingFlags, m_world);
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

    (*dpool)[d]->m_matrix = m_world;
    (*dpool)[d]->m_type = DType::Normal;
    auto func = [&](VkCommandBuffer _cb){
        VkViewport tmp = getViewPort();
        vkCmdSetViewport(_cb,0,1,&tmp);

        VkRect2D scissor = getScissor();
        vkCmdSetScissor(_cb,0,1,&scissor);

        std::vector<VkDescriptorSet> sets{};
        for(auto & s: m_sets)
        {
            sets.emplace_back(SingleSetPool[s]);
        }
        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()[m_pipeline]->getPipelineLayout(),
                                1,
                                sets.size(),
                                sets.data(),
                                0,
                                VK_NULL_HANDLE);

        // Mesh containing the LODs
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()[m_pipeline]->getPipeline());
        m_model->draw(_cb, 0, PPool::getPool()[m_pipeline]->getPipelineLayout());
    };
    if(m_draw_type == DrawObjectType::Default)
    {
        (*dpool)[d]->m_gGraphics = func;
    }else{
        (*dpool)[d]->m_newDraw = func;
    }
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("BasicAnimation",m_id),[&,d](float _abs_time){
        m_model->updateAnimation(m_animation_index, _abs_time,m_world);
    });
    insertRenderObject(m_draw_type, d);

    int model_index = ModelSetPool::getPool().malloc();
    TaskPool::pushUpdateTask(tool::combine("BasicAnimationWorld",m_id),[&,model_index](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + model_index, &m_world, one_matrix);
    });
    auto aabbPool = MPool<AABB>::getPool();
    (*aabbPool)[m_aabbIndex]->addInternalPoint(m_model->dimensions.min);
    (*aabbPool)[m_aabbIndex]->addInternalPoint(m_model->dimensions.max);
    (*aabbPool)[m_aabbIndex]->m_model_index = model_index;
    SingleRender.aabb_map[m_capture_id] = m_aabbIndex;
}
