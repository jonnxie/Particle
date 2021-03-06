//
// Created by jonnxie on 2021/10/25.
//
#include "precompiledhead.h"

#include "basic.h"

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
#include ManipulateCatalog

Basic::Basic(const std::string& _files,
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
    m_model->loadFromFile(_files, &SingleDevice, VkQueue{}, glTFLoadingFlags);
    m_manipulate = std::make_unique<Manipulate>();
    m_manipulate->setScale(_scale);
    m_manipulate->_rotationAxis = _rotationAxis;
    m_manipulate->_angle = _angle;
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = _pos;
    m_manipulate->setChanged(true);
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
    int modelIndex = m_manipulate->getModelId();

    (*dpool)[d]->m_model_index = modelIndex;
    (*dpool)[d]->m_matrix = m_manipulate->getMatrix();
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->m_gGraphics = [&, modelIndex](VkCommandBuffer _cb){
    tool::cmdDynamicState(_cb);
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
    for (int i : m_dobjs) {
        SingleRender.pushDObjects(i);
    }
//    TaskPool::pushUpdateTask(tool::combine("Basic",m_id),[&, modelIndex, d](float _abs_time){
//        glm::mat4* ptr = SingleBPool.getModels();
//        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
//    });
}
