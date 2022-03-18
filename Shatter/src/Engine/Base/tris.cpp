//
// Created by jonnxie on 2022/1/28.
//
#include "precompiledhead.h"

#include "tris.h"
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
#include CameraCatalog
#include "Engine/Object/inputaction.h"

DTris::DTris(size_t _initCount, MeshDrawType _type): type(_type){
    tris = std::vector<Tri>(_initCount);
    id = mallocTriId();
}

DTris::DTris(const std::vector<Tri>& _tris, MeshDrawType _type): type(_type){
    tris = _tris;
    id = mallocTriId();
}

void DTris::constructG(){
    SingleBPool.createVertexHostBuffer(tool::combine("DTris",id),TriSize * tris.size(),tris.data());
    SingleBPool.getBuffer(tool::combine("DTris",id),Buffer_Type::Vertex_Host_Buffer)->map();
}

void DTris::constructD(){
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
                         tool::combine("DTris",id),
                         tris.size() * 3,
                         "",
                         0,
                         0,
                         type == MeshDrawType::Face ? "TriangleFace":"TriangleLine",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("TrisBasic",id),[&,ms_index,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
//    Shatter::app::ShatterApp::getApp().getNObjects()->push_back(d);
    SingleRender.getNObjects()->push_back(d);
}

void DTris::pushTri(const Tri &_tri) {
    tris.push_back(_tri);
}

void DTris::pushTris(const std::vector<Tri>& _tris) {
    tris.insert(tris.end(),tris.begin(),tris.end());
}

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

DPlane::DPlane(const NPlane& _plane)
:plane(_plane)
{
    id = mallocId();
    init();
};

void DPlane::constructG(){
    SingleBPool.createVertexHostBuffer(tool::combine("DPlane",id), NPlaneSize, &plane);
    SingleBPool.getBuffer(tool::combine("DPlane",id),Buffer_Type::Vertex_Host_Buffer)->map();
    uint32_t indices[6]{0, 2, 1, 0, 3, 2};
    SingleBPool.createIndexBuffer(tool::combine("DPlane",id), 24, indices);
};

void DPlane::constructD(){
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int ms_index = ModelSetPool::getPool().malloc();

    std::vector<std::string> s_vec(2);
    s_vec[0]="Camera";
    s_vec[1]="Planet";
    (*dpool)[d]->m_type = DType::Normal;

    glm::mat4 matrix{
        glm::vec4{SingleCamera.m_targetPlane.x_coordinate,0.0f},
        glm::vec4{SingleCamera.m_targetPlane.y_coordinate,0.0f},
        glm::vec4{SingleCamera.m_targetPlane.z_coordinate,0.0f},
        glm::vec4{glm::vec3{0.0f},1.0f},
    };

    (*dpool)[d]->prepare(matrix,
                         ms_index,
                         DrawType::Vertex,
                         0,
                         tool::combine("DPlane",id),
                         4,
                         tool::combine("DPlane",id),
                         6,
                         0,
                         "GPlane",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("TrisBasic",id),[&,ms_index,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
//    Shatter::app::ShatterApp::getApp().getNObjects()->push_back(d);
    SingleRender.getNObjects()->push_back(d);
};

DrawNPlane::~DrawNPlane() {
    TaskPool::popUpdateTask("DrawPlaneUpdate");
}


DrawNPlane::DrawNPlane() {
    m_action[Event::SingleClick] = [&]() {
        static bool draw = false;
        static glm::vec3 pre_pos;
        static glm::vec3 realPos;
        static NPlane plane;
        static VkDevice localDevice = SingleDevice();
        if (draw) {
            TaskPool::popUpdateTask("DrawPlaneUpdate");
            draw = false;
        } else {
            pre_pos = input::getCursor();
            pre_pos = SingleCamera.m_targetPlane.x_coordinate * glm::dot(SingleCamera.m_targetPlane.x_coordinate, pre_pos - SingleCamera.center)
                    + SingleCamera.m_targetPlane.y_coordinate * glm::dot(SingleCamera.m_targetPlane.y_coordinate, pre_pos - SingleCamera.center);
            genPlane(pre_pos, pre_pos, plane);
            auto p = std::make_unique<DPlane>(plane);
            planes.push_back(std::move(p));
            TaskPool::pushUpdateTask("DrawPlaneUpdate",[&](float _abs_time){
                auto localPlane = std::move(planes.back());
                planes.pop_back();
                int id = localPlane->id;
                realPos = input::getCursor();
                realPos = SingleCamera.m_targetPlane.x_coordinate * glm::dot(SingleCamera.m_targetPlane.x_coordinate, realPos - SingleCamera.center)
                          + SingleCamera.m_targetPlane.y_coordinate * glm::dot(SingleCamera.m_targetPlane.y_coordinate, realPos - SingleCamera.center);
                genPlane(pre_pos, realPos, plane);
                auto buffer = SingleBPool.getBuffer(tool::combine("DPlane", id), Buffer_Type::Vertex_Host_Buffer);
                auto* ptr = (Point*)buffer->mapped;
                memcpy(ptr, &plane, NPlaneSize);
                planes.push_back(std::move(localPlane));
            });
            SingleRender.normalChanged = true;
            draw = true;
        };
    };
}









