//
// Created by AnWell on 2022/2/10.
//

#include "points.h"
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

Points::Points(size_t _initCount) {
    points = std::vector<Point3dColorSize>(_initCount);
    id = mallocPointId();
}

Points::Points(const std::vector<Point3dColorSize> &_points) {
    points = _points;
    id = mallocPointId();
}

void Points::constructG() {
    SingleBPool.createVertexHostBuffer(tool::combine("Points",id), Point3dColorSizeSize * points.size(),points.data());
    SingleBPool.getBuffer(tool::combine("Points",id),Buffer_Type::Vertex_Host_Buffer)->map();
}

void Points::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int ms_index = ModelSetPool::getPool().malloc();

    std::vector<std::string> s_vec(1);
    s_vec[0]="Camera";
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(glm::mat4(1.0f),
                         ms_index,
                         DrawType::Vertex,
                         0,
                         tool::combine("Points",id),
                         points.size(),
                         "",
                         0,
                         0,
                         "Point",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("PointBasic",id),[&,ms_index,d](float _abs_time){
        auto buffer = BPool::getPool().getBuffer("Model",Buffer_Type::Uniform_Buffer);
        auto* ptr = static_cast<glm::mat4 *>(buffer->mapped);
        ptr += ms_index;
        memcpy(ptr,&(*SingleDPool)[d]->m_matrix,one_matrix);
    });
    SingleRender.getNObjects()->push_back(d);
}

void Points::pushPoint(const Point3dColorSize &_point) {
    points.push_back(_point);
}

void Points::pushPoint(const std::vector<Point3dColorSize> &_points) {
    points.insert(points.end(),_points.begin(),_points.end());
}
