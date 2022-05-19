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
#include CaptureCatalog
#include ManipulateCatalog

ABasic::ABasic(const std::string& _files,
             glm::vec3 _pos,
             glm::vec3 _rotationAxis,
             float _angle,
             glm::vec3 _scale,
             int _id,
             std::string  _pipeline,
             std::vector<std::string>  _sets,
             DrawObjectType _type,
             bool _binary):
        m_pipeline(std::move(_pipeline)),
        m_sets{std::move(_sets)}
//        m_draw_type(_type)
{
    setDrawType(_type);
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    m_model = new vkglTF::Model;

    m_manipulate = std::make_unique<Manipulate>();
    m_manipulate->setScale(_scale);
    m_manipulate->_rotationAxis = _rotationAxis;
    m_manipulate->_angle = _angle;
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = _pos;
    m_manipulate->setChanged(true);

    m_model->loadFromFile(_files, &SingleDevice, VkQueue{}, glTFLoadingFlags, m_manipulate->getMatrix(), 1.0f, _binary);
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

    (*dpool)[d]->m_matrix = m_manipulate->getMatrix();
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->setData(m_model);
    (*dpool)[d]->setUpdate(true);
    auto func = [&, d](VkCommandBuffer _cb){
        UnionViewPort& tmp = SingleAPP.getPresentViewPort();
        vkCmdSetViewport(_cb, 0, 1, &tmp.view);
        VkRect2D& scissor = tmp.scissor;
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
        ((vkglTF::Model*)(*SingleDPool)[d]->getData())->draw(_cb, 0, PPool::getPool()[m_pipeline]->getPipelineLayout());
//        m_model->draw(_cb, 0, PPool::getPool()[m_pipeline]->getPipelineLayout());
    };
    if(getDrawType() == DrawObjectType::Default)
    {
        (*dpool)[d]->m_gGraphics = func;
    }else{
        (*dpool)[d]->m_newDraw = func;
    }
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("BasicAnimation", m_id),[&, d](float _abs_time){
        if(m_update)
        {
            ((vkglTF::Model*)(*SingleDPool)[d]->getData())->updateAnimation(m_animation_index, _abs_time, (*SingleDPool)[d]->m_matrix);
        }else{
            ((vkglTF::Model*)(*SingleDPool)[d]->getData())->updateAnimation(m_animation_index, m_localTime, (*SingleDPool)[d]->m_matrix);
        }
    });
    insertRenderObject(d);
    setCapture(CaptureObject::mallocCapture(this,
                                                   m_model->dimensions.min,
                                                   m_model->dimensions.max,
                                                   d,
                                                   "ABasic"));
}
