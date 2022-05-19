//
// Created by jonnxie on 2022/4/8.
//

#include "SkinBasic.h"
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
#include <utility>
#include <vector>
#include ManipulateCatalog

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;

    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

SkinBasic::SkinBasic(const std::string &_files, glm::vec3 _pos, glm::vec3 _rotationAxis, float _angle, glm::vec3 _scale,
                     std::string _pipeline, std::vector<std::string> _sets, DrawObjectType _type):
        m_pipeline(std::move(_pipeline)),
        m_sets{std::move(_sets)}
{
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreMultiplyVertexColors;
    m_model = new vkglTF::Model;
    m_manipulate = std::make_unique<Manipulate>();
    m_manipulate->setScale(_scale);
    m_manipulate->_rotationAxis = _rotationAxis;
    m_manipulate->_angle = _angle;
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = _pos;
    m_manipulate->setChanged(true);
    m_model->loadFromFile(_files, &SingleDevice, VkQueue{}, glTFLoadingFlags, m_manipulate->getMatrix());
    m_id = mallocId();
    init();
}

void SkinBasic::constructG() {
}

void SkinBasic::constructD() {
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
        ((vkglTF::Model*)(*SingleDPool)[d]->getData())->draw(_cb,
                                                             vkglTF::RenderFlags::BindImages,
                                                             PPool::getPool()[m_pipeline]->getPipelineLayout(),
                                                             3);
//        m_model->draw(_cb, 0, PPool::getPool()[m_pipeline]->getPipelineLayout());
    };
    if(getDrawType() == DrawObjectType::Default)
    {
        (*dpool)[d]->m_gGraphics = func;
    }else{
        (*dpool)[d]->m_newDraw = func;
    }
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("SkinAnimation", m_id),[&, d](float _abs_time){
        if (m_update)
        {
            ((vkglTF::Model*)(*SingleDPool)[d]->getData())->updateAnimation(m_animation_index, _abs_time, (*SingleDPool)[d]->m_matrix, true);
        } else {
            ((vkglTF::Model*)(*SingleDPool)[d]->getData())->updateAnimation(m_animation_index, m_localTime, (*SingleDPool)[d]->m_matrix, true);
        }
    });
    insertRenderObject(d);
    setCapture(CaptureObject::mallocCapture(this,
                                                   m_model->dimensions.min,
                                                   m_model->dimensions.max,
                                                   d,
                                                   "SkinBasic"));
}

SkinBasicInstance::SkinBasicInstance(const std::string &_files, const std::vector<glm::vec3> &_instances,
                                     glm::vec3 _rotationAxis, float _angle, glm::vec3 _scale,
                                     std::string _pipeline,
                                     std::vector<std::string> _sets, DrawObjectType _type):
                                     m_pipeline(std::move(_pipeline)),
m_sets{std::move(_sets)},
GeoPool<glm::vec3>(_instances)
{
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreMultiplyVertexColors;
    m_model = new vkglTF::Model;
//    m_scale = glm::scale(glm::mat4(1.0f), _scale);
//    m_rotate = glm::rotate(glm::mat4(1.0f), _angle, _rotationAxis);
//    m_translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
//    m_world = m_translation * m_scale * m_rotate;
    m_manipulate = std::make_unique<Manipulate>();
    m_manipulate->setScale(_scale);
    m_manipulate->_rotationAxis = _rotationAxis;
    m_manipulate->_angle = _angle;
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = glm::vec3(0.0f);
    m_manipulate->setChanged(true);
    m_model->loadFromFile(_files, &SingleDevice, VkQueue{}, glTFLoadingFlags, m_manipulate->getMatrix());
    m_id = mallocId();
    init();
}

SkinBasicInstance::SkinBasicInstance(SkinBasic* _skin, const std::vector<glm::vec3>& _instances) :
m_pipeline(_skin->getPipeline()),
m_sets(_skin->getSets()),
GeoPool<glm::vec3>(_instances)
{
    m_model = _skin->getModel();
    _skin->setModel(nullptr);
    m_manipulate = std::make_unique<Manipulate>(_skin->getManipulate());
//    m_scale = _skin->getScale();
//    m_rotate = _skin->getRotate();
//    m_translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
//    m_world = m_translation * m_scale * m_rotate;
    m_id = mallocId();
    init();
}

void SkinBasicInstance::reallocate() {
    GeoPool::reallocate();
    BPool::getPool().freeBuffer(tool::combine("SkinBasicInstance",m_id), Buffer_Type::Vertex_Buffer);
    BPool::getPool().createVertexBuffer(tool::combine("SkinBasicInstance",m_id),
                                        sizeof(glm::vec3) * geos.size(),
                                        geos.data());//Particle data
}

void SkinBasicInstance::constructG() {
    BPool::getPool().createVertexBuffer(tool::combine("SkinBasicInstance",m_id),
                                        sizeof(glm::vec3) * geos.size(),
                                        geos.data());//Particle data
}

void SkinBasicInstance::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();

    (*dpool)[d]->m_matrix = m_manipulate->getMatrix();
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->setData(m_model);
    (*dpool)[d]->setUpdate(true);
    auto instanceBuffer = SingleBPool.getBuffer(tool::combine("SkinBasicInstance", m_id),
                                                Buffer_Type::Vertex_Buffer)->getBuffer();
    auto func = [&, d, instanceBuffer](VkCommandBuffer _cb) {
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

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(_cb, 1, 1, &instanceBuffer, &offset);

        // Mesh containing the LODs
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()[m_pipeline]->getPipeline());
        m_model->drawInstance(_cb,
                              count,
                              vkglTF::RenderFlags::BindImages,
                              PPool::getPool()[m_pipeline]->getPipelineLayout(),
                              3);
    };
    if(getDrawType() == DrawObjectType::Default)
    {
        (*dpool)[d]->m_gGraphics = func;
    } else {
        (*dpool)[d]->m_newDraw = func;
    }
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("SkinBasicInstance", m_id),[&, d](float _abs_time){
        if (m_update)
        {
            m_model->updateAnimation(m_animation_index, _abs_time, (*SingleDPool)[d]->m_matrix, true);
        } else {
            m_model->updateAnimation(m_animation_index, m_localTime, (*SingleDPool)[d]->m_matrix, true);
        }
    });
    insertRenderObject(d);
//    m_captureObject = CaptureObject::mallocCapture(this,
//                                                   m_model->dimensions.min,
//                                                   m_model->dimensions.max,
//                                                   d,
//                                                   "SkinBasicInstance");
}
