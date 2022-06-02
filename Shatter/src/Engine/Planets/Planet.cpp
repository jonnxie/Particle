//
// Created by AnWell on 2022/2/26.
//

#include "precompiledhead.h"
#include "Planet.h"

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
#include AppCatalog
#include RenderCatalog
#include ManipulateCatalog

Planet::Planet(uint32_t _resolution, glm::vec3 _pos, glm::vec3 _rotationAxis, float _angle, glm::vec3 _scale, float _radius,
               glm::vec3 _color,
               std::string  _pipeline, std::vector<std::string>  _sets,
               DrawObjectType _type){
    setDrawType(_type);
    m_pipeline = std::move(_pipeline);
    m_sets = std::move(_sets);
    m_radius = _radius;
    m_color = _color;
    m_resolution = _resolution;
    m_manipulate = std::make_unique<Manipulate>();
    m_manipulate->setScale(_scale);
    m_manipulate->_rotationAxis = _rotationAxis;
    m_manipulate->_angle = _angle;
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = _pos;
    m_manipulate->setChanged(true);
    m_manipulate->getMatrix();
    m_id = mallocPlanetId();
    init();
}

Planet::~Planet() = default;

void Planet::generateMesh() {
    m_points.clear();
    m_indices.clear();
    for(auto& face : m_faces)
    {
        std::vector<glm::vec3> pos;
        std::vector<uint32_t> indices;
        face.generateMesh(pos, m_coordinates, indices, m_radius);
        m_points.insert(m_points.end(), pos.begin(), pos.end());
        m_indices.insert(m_indices.end(), indices.begin(), indices.end());
    }
    uint32_t count = (m_resolution - 1) * (m_resolution - 1) * 6;
    uint32_t point_count = m_resolution * m_resolution;
    for(size_t index = 0; index < count; index++)
    {
        m_indices[index + count] += point_count;
        m_indices[index + 2 * count] += 2 * point_count;
        m_indices[index + 3 * count] += 3 * point_count;
        m_indices[index + 4 * count] += 4 * point_count;
        m_indices[index + 5 * count] += 5 * point_count;
    }
    SingleBPool.createVertexHostBuffer(tool::combine("Planet",m_id),one_vec3 * m_points.size(),m_points.data());
    SingleBPool.getBuffer(tool::combine("Planet",m_id),Buffer_Type::Vertex_Host_Buffer)->map();

    SingleBPool.createIndexHostBuffer(tool::combine("Planet",m_id),sizeof(uint32_t) * m_indices.size(), m_indices.data());
    SingleBPool.getBuffer(tool::combine("Planet",m_id),Buffer_Type::Index_Host_Buffer)->map();

    {
        SingleBPool.createUniformBuffer(tool::combine("Planet",m_id),one_vec3);
        SingleBPool.getBuffer(tool::combine("Planet",m_id),Buffer_Type::Uniform_Buffer)->map();

        memcpy(SingleBPool.getBuffer(tool::combine("Planet",m_id),Buffer_Type::Uniform_Buffer)->mapped, &m_color, one_vec3);

        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = SingleBPool.getBuffer(tool::combine("Planet", m_id),
                                                   Buffer_Type::Uniform_Buffer)->getBuffer();
        buffer_info.offset = 0;
        buffer_info.range = one_vec3;

        VkWriteDescriptorSet planet_write = tool::writeDescriptorSet(SingleSetPool["Planet"],
                                                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                     0,
                                                                     &buffer_info);

        vkUpdateDescriptorSets(SingleDevice(),
                               1,
                               &planet_write,
                               0,
                               nullptr);
    }

}

void Planet::constructG() {
    std::vector<glm::vec3> directions{UpDirection, DownDirection, LeftDirection, RightDirection, ForwardDirection, BackDirection};
    for(auto& dir : directions)
    {
        m_faces.emplace_back(TerrainFace(m_resolution,dir));
    }
    generateMesh();
}

void Planet::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int modelIndex = m_manipulate->getModelId();

    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(glm::mat4(1.0f),
                         modelIndex,
                         DrawType::Index,
                         0,
                         tool::combine("Planet", m_id),
                         m_points.size(),
                         tool::combine("Planet", m_id),
                         m_indices.size(),
                         0,
                         m_pipeline,
                         m_sets,
                         m_pipeline,
                         m_sets);
    insertDObject(d);
//    TaskPool::pushUpdateTask(tool::combine("Planet", m_id),[&,modelIndex,d](float _abs_time){
//        glm::mat4* ptr = SingleBPool.getModels();
//        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
//    });
//    SingleRender.pushNObjects(d);
    insertRenderObject(d);
}

void Planet::constructC() {

}
