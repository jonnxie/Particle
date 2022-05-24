//
// Created by jonnxie on 2022/5/23.
//

#include "earth.h"
#include PPoolCatalog
#include PipelineCatalog
#include ShaderCatalog
#include <utility>
#include TexturePoolCatalog

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

Earth::Earth(glm::vec3 _pos, uint32_t _longitudeResolution, uint32_t _latitudeResolution, double _radius,
             std::string _pipeline, std::string _textureId) :
        m_longitudeResolution(_longitudeResolution),
        m_latitudeResolution(_latitudeResolution),
        m_radius(_radius),
        m_pipeline(std::move(_pipeline)),
        m_texture(std::move(_textureId))
{
    m_id = mallocId();
    m_vertices.resize(m_longitudeResolution * m_latitudeResolution * 6);
    m_manipulate = std::make_unique<Manipulate>();
    m_manipulate->setPosition(_pos);
    m_manipulate->setChanged(true);
    init();
}

Earth::~Earth() {

}

void Earth::constructG() {
    double reverseLongitudeResolution = two_pai / double(m_longitudeResolution);
    double reverseLatitudeResolution = pai / double(m_latitudeResolution);
    uint32_t index = 0;
    for (int i = 0; i < m_longitudeResolution - 1; ++i) {
        for (int j = 0; j < m_latitudeResolution - 1; ++j) {
            index = i * (m_latitudeResolution - 1) + j;
            m_vertices[index * 6]     = glm::dvec2(-pai + double(i) * reverseLongitudeResolution, - half_pai +  double(j) * reverseLatitudeResolution);
            m_vertices[index * 6 + 1] = glm::dvec2(-pai + double((i + 1)) * reverseLongitudeResolution, - half_pai +  double(j) * reverseLatitudeResolution);
            m_vertices[index * 6 + 2] = glm::dvec2(-pai + double(i) * reverseLongitudeResolution, - half_pai +  double((j + 1)) * reverseLatitudeResolution);
            m_vertices[index * 6 + 3] = glm::dvec2(-pai + double(i) * reverseLongitudeResolution, - half_pai +  double((j + 1)) * reverseLatitudeResolution);
            m_vertices[index * 6 + 4] = glm::dvec2(-pai + double((i + 1)) * reverseLongitudeResolution, - half_pai +  double(j) * reverseLatitudeResolution);
            m_vertices[index * 6 + 5] = glm::dvec2(-pai + double((i + 1)) * reverseLongitudeResolution, - half_pai +  double((j + 1)) * reverseLatitudeResolution);
        }
    }
    SingleBPool.createVertexBuffer(tool::combine("Earth", m_id), sizeof(glm::dvec2) * m_vertices.size(), m_vertices.data());
    SingleBPool.getBuffer(tool::combine("Earth", m_id), Buffer_Type::Vertex_Buffer)->map();

    SingleBPool.createUniformBuffer(tool::combine("Earth", m_id), sizeof(double));
    auto buffer = SingleBPool.getBuffer(tool::combine("Earth", m_id), Buffer_Type::Uniform_Buffer);
    buffer->map();
    memcpy(buffer->mapped, &m_radius, sizeof(double));
    buffer->unmap();
}

void Earth::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();

    (*dpool)[d]->m_matrix = m_manipulate->getMatrix();
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->setUpdate(false);

    VkDescriptorSet earthSet;
    SingleSetPool.AllocateDescriptorSets(std::vector<VkDescriptorSetLayout>{SingleShaderPool.getSLMap()["earth_vs"][2]}, &earthSet);
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = SingleBPool.getBuffer(tool::combine("Earth", m_id), Buffer_Type::Uniform_Buffer)->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(double);
        VkWriteDescriptorSet write = tool::writeDescriptorSet(earthSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                                              &bufferInfo);

        vkUpdateDescriptorSets(Device::getDevice()(),
                               1,
                               &write,
                               0,
                               nullptr);
    }

    auto func = [&, earthSet](VkCommandBuffer _cb){
        UnionViewPort& tmp = SingleAPP.getPresentViewPort();
        vkCmdSetViewport(_cb, 0, 1, &tmp.view);
        VkRect2D& scissor = tmp.scissor;
        vkCmdSetScissor(_cb,0,1,&scissor);

        auto set_pool = MPool<VkDescriptorSet>::getPool();

        std::vector<VkDescriptorSet> sets{*(*set_pool)[m_manipulate->getModelId()],
                                          SingleSetPool["Camera"],
                                          earthSet,
                                          SingleTexturePool[m_texture].set
                                          };

        vkCmdBindDescriptorSets(_cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                PPool::getPool()[m_pipeline]->getPipelineLayout(),
                                0,
                                sets.size(),
                                sets.data(),
                                0,
                                VK_NULL_HANDLE);
        auto buffer = SingleBPool.getBuffer(tool::combine("Earth", m_id), Buffer_Type::Vertex_Buffer)->getBuffer();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(_cb, 0, 1, &buffer, &offset);
        vkCmdBindPipeline(_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()[m_pipeline]->getPipeline());
        vkCmdDraw(_cb, m_longitudeResolution * m_latitudeResolution * 6, 1, 0, 0);
    };
    (*dpool)[d]->m_newDraw = func;
    insertDObject(d);
    insertRenderObject(DrawObjectType::Normal, d);
}
