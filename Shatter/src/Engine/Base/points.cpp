//
// Created by AnWell on 2022/2/10.
//
#include "precompiledhead.h"

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

DPoints::DPoints(size_t _initCount) {
    points = std::vector<Point3dColorSize>(_initCount);
    id = mallocPointId();
}

DPoints::DPoints(const std::vector<Point3dColorSize> &_points) {
    points = _points;
    id = mallocPointId();
}

void DPoints::constructG() {
    SingleBPool.createVertexHostBuffer(tool::combine("DPoints",id), Point3dColorSizeSize * points.size(),points.data());
    SingleBPool.getBuffer(tool::combine("DPoints",id),Buffer_Type::Vertex_Host_Buffer)->map();
}

void DPoints::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int ms_index = ModelSetPool::getPool().malloc();

    std::vector<std::string> s_vec(2);
    s_vec[0]="Camera";
    s_vec[1]="ViewPort";
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(glm::mat4(1.0f),
                         ms_index,
                         DrawType::Vertex,
                         0,
                         tool::combine("DPoints",id),
                         points.size(),
                         "",
                         0,
                         0,
                         "Point",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("PointBasic",id),[&,ms_index,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
    SingleRender.getNObjects()->push_back(d);
}

void DPoints::pushPoint(const Point3dColorSize &_point) {
    points.push_back(_point);
}

void DPoints::pushPoint(const std::vector<Point3dColorSize> &_points) {
    points.insert(points.end(),_points.begin(),_points.end());
}

DPointPool::DPointPool(const std::vector<Line> &_lines, int _coordinate, bool _updateFunc, std::string _pipeline,
                       std::vector<std::string> _sets) {

}

DPointPool::~DPointPool() {

}

void DPointPool::constructG() {
    Object::constructG();
}

void DPointPool::constructD() {
    Object::constructD();
}

void DPointPool::pushLine(const Line &_line) {

}

void DPointPool::pushLines(const std::vector<Line> &_lines) {

}

void DPointPool::reallocated() {

}

PointsHandle::PointsHandle() {

}

PointsHandle::~PointsHandle() {

}

Line &PointsHandle::operator[](size_t _index) {
}

void PointsHandle::pushLine(const glm::vec3 &_begin, const glm::vec3 &_end) {

}

void PointsHandle::pushLine(const Line &_line) {

}

void PointsHandle::pushLines(const std::vector<std::pair<glm::vec3, glm::vec3>> &_lines) {

}

void PointsHandle::pushUI() {

}

int PointsHandle::getLineCount() {
    return 0;
}

void PointsHandle::loadFile(const std::string &_filename) {

}

void PointsHandle::drawLine() {

}

void PointsHandle::destroy() const {

}

glm::vec3 &PointsHandle::getWorkCenter() const {
}

TargetPlane &PointsHandle::getTargetPlane() const {
}

DrawPointHandle::DrawPointHandle(PointsHandle *_handle) {

}

DrawPointHandle::~DrawPointHandle() {
    int i = 0;
}
