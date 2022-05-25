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
             std::string _pipeline, bool _height, std::string _textureId, std::string _heightTextureId) :
        m_longitudeResolution(_longitudeResolution),
        m_latitudeResolution(_latitudeResolution),
        m_radius(_radius),
        m_pipeline(std::move(_pipeline)),
        m_texture(std::move(_textureId)),
        m_height(_height),
        m_heightTexture(std::move(_heightTextureId))
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

struct EarthInfo {
    double radius;
    double scale;
};

void Earth::constructG() {
    double reverseLongitudeResolution = 360.000000000000f / double(m_longitudeResolution);
    double reverseLatitudeResolution = 180.000000000000f / double(m_latitudeResolution);
    uint32_t index = 0;
    for (int i = 0; i < m_longitudeResolution ; ++i) {
        for (int j = 0; j < m_latitudeResolution ; ++j) {
            index = i * (m_latitudeResolution ) + j;
            m_vertices[index * 6]     = glm::dvec2(-180.000000000000f + double(i) * reverseLongitudeResolution, - 90.00000000000f +  double(j) * reverseLatitudeResolution);
            m_vertices[index * 6 + 1] = glm::dvec2(-180.000000000000f + double((i + 1)) * reverseLongitudeResolution, - 90.00000000000f +  double(j) * reverseLatitudeResolution);
            m_vertices[index * 6 + 2] = glm::dvec2(-180.000000000000f + double(i) * reverseLongitudeResolution, - 90.00000000000f +  double((j + 1)) * reverseLatitudeResolution);
            m_vertices[index * 6 + 3] = glm::dvec2(-180.000000000000f + double(i) * reverseLongitudeResolution, - 90.00000000000f +  double((j + 1)) * reverseLatitudeResolution);
            m_vertices[index * 6 + 4] = glm::dvec2(-180.000000000000f + double((i + 1)) * reverseLongitudeResolution, - 90.00000000000f +  double(j) * reverseLatitudeResolution);
            m_vertices[index * 6 + 5] = glm::dvec2(-180.000000000000f + double((i + 1)) * reverseLongitudeResolution, - 90.00000000000f +  double((j + 1)) * reverseLatitudeResolution);
        }
    }
    SingleBPool.createVertexBuffer(tool::combine("Earth", m_id), sizeof(glm::dvec2) * m_vertices.size(), m_vertices.data());
    SingleBPool.getBuffer(tool::combine("Earth", m_id), Buffer_Type::Vertex_Buffer)->map();

    SingleBPool.createUniformBuffer(tool::combine("Earth", m_id), sizeof(EarthInfo));
    EarthInfo info{};
    info.radius = m_radius;
    info.scale = (m_radius / double(12756000.0f));
    std::cout << std::fixed << "earth scale:" << info.scale << std::endl;
    auto buffer = SingleBPool.getBuffer(tool::combine("Earth", m_id), Buffer_Type::Uniform_Buffer);
    buffer->map();
    memcpy(buffer->mapped, &info, sizeof(EarthInfo));
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
        bufferInfo.range = sizeof(EarthInfo);
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

        if (m_height) {
            std::array<VkDescriptorSet,5> sets{*(*set_pool)[m_manipulate->getModelId()],
                                               SingleSetPool["Camera"],
                                               earthSet,
                                               SingleTexturePool[m_heightTexture].set,
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
        } else {
            std::array<VkDescriptorSet,4> sets{*(*set_pool)[m_manipulate->getModelId()],
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
        }

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
