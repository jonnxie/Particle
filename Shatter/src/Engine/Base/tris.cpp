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

DPlane::~DPlane(){
    TaskPool::popUpdateTask(tool::combine("DPlane",id));
}

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
        glm::vec4{SingleAPP.getWorkTargetPlane().x_coordinate,0.0f},
        glm::vec4{SingleAPP.getWorkTargetPlane().y_coordinate,0.0f},
        glm::vec4{SingleAPP.getWorkTargetPlane().z_coordinate,0.0f},
        glm::vec4{glm::vec3{0.0f},1.0f},
    };
//    matrix = glm::inverse(matrix);
    (*dpool)[d]->prepare(matrix,
                         ms_index,
                         DrawType::Index,
                         0,
                         tool::combine("DPlane", id),
                         4,
                         tool::combine("DPlane", id),
                         6,
                         0,
                         "GPlane",
                         s_vec,
                         "GPlane",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("DPlane",id),[&, ms_index, d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
//    Shatter::app::ShatterApp::getApp().getNObjects()->push_back(d);
    SingleRender.getDObjects()->push_back(d);
};

DrawNPlane::~DrawNPlane() {
    TaskPool::popUpdateTask("DrawNPlaneUpdate");
}


DrawNPlane::DrawNPlane() {
    m_action[Event::SingleClick] = [&]() {
        static bool draw = false;
        static glm::vec3 pre_pos;
        static glm::vec3 realPos;
        static NPlane plane;
        if (draw) {
            TaskPool::popUpdateTask("DrawNPlaneUpdate");
            draw = false;
        } else {
            pre_pos = input::getCursor();
            pre_pos = glm::vec3(1.0f, 0.0f, 0.0f) * glm::dot(SingleAPP.getWorkTargetPlane().x_coordinate, pre_pos - SingleCamera.center)
                    + glm::vec3(0.0f, 1.0f, 0.0f) * glm::dot(SingleAPP.getWorkTargetPlane().y_coordinate, pre_pos - SingleCamera.center);
            genPlane(pre_pos, pre_pos, plane);
            auto p = std::make_unique<DPlane>(plane);
            planes.push_back(std::move(p));
            TaskPool::pushUpdateTask("DrawNPlaneUpdate",[&](float _abs_time){
                auto localPlane = std::move(planes.back());
                planes.pop_back();
                int id = localPlane->id;
                realPos = input::getCursor();
                realPos = glm::vec3(1.0f, 0.0f, 0.0f) * glm::dot(SingleAPP.getWorkTargetPlane().x_coordinate, realPos - SingleCamera.center)
                          + glm::vec3(0.0f, 1.0f, 0.0f) * glm::dot(SingleAPP.getWorkTargetPlane().y_coordinate, realPos - SingleCamera.center);
                genPlane(pre_pos, realPos, plane);
                auto buffer = SingleBPool.getBuffer(tool::combine("DPlane", id), Buffer_Type::Vertex_Host_Buffer);
                auto* ptr = (Point*)buffer->mapped;
                memcpy(ptr, &plane, NPlaneSize);
                planes.push_back(std::move(localPlane));
            });
            SingleRender.drawChanged = true;
            draw = true;
        };
    };
}

static int mallocCubeId()
{
    static int initIdVal = 0;
    static std::mutex idLock;
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

DCube::DCube(const Cube &_cube):
cube(_cube)
{
    id = mallocCubeId();
    init();
}

DCube::~DCube() {
    TaskPool::popUpdateTask(tool::combine("DCube",id));
}

void DCube::constructG() {
    SingleBPool.createVertexHostBuffer(tool::combine("DCube",id), CubeSize, &cube);
    SingleBPool.getBuffer(tool::combine("DCube",id),Buffer_Type::Vertex_Host_Buffer)->map();
    uint32_t indices[36];
    uint32_t planeIndices[6]{0, 2, 1, 0, 3, 2};
    for (int i = 0; i < static_cast<int>(CubePlane::PlaneCount); ++i) {
        int base_count = i * static_cast<int>(CubePlane::PlaneCount);
        int vertex_count = i * 4;
        indices[base_count]     = planeIndices[0] + vertex_count;
        indices[base_count + 1] = planeIndices[1] + vertex_count;
        indices[base_count + 2] = planeIndices[2] + vertex_count;
        indices[base_count + 3] = planeIndices[3] + vertex_count;
        indices[base_count + 4] = planeIndices[4] + vertex_count;
        indices[base_count + 5] = planeIndices[5] + vertex_count;
    }
    SingleBPool.createIndexBuffer(tool::combine("DCube",id), 144, indices);
}

void DCube::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int ms_index = ModelSetPool::getPool().malloc();

    std::vector<std::string> s_vec(2);
    s_vec[0]="Camera";
    s_vec[1]="Planet";
    (*dpool)[d]->m_type = DType::Normal;

    glm::mat4 matrix{
            glm::vec4{SingleAPP.getWorkTargetPlane().x_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetPlane().y_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetPlane().z_coordinate, 0.0f},
            glm::vec4{glm::vec3{0.0f}, 1.0f},
    };
    m_plane = SingleAPP.getWorkTargetPlane();
    m_center = SingleAPP.getWorkTargetCenter();
//    matrix = glm::inverse(matrix);
    (*dpool)[d]->prepare(matrix,
                         ms_index,
                         DrawType::Index,
                         0,
                         tool::combine("DCube", id),
                         24,
                         tool::combine("DCube", id),
                         36,
                         0,
                         "GCube",
                         s_vec,
                         "GCube",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("DCube",id),[&, ms_index, d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
    SingleRender.getDObjects()->push_back(d);
}

DrawCube::~DrawCube() {
    TaskPool::popUpdateTask("DrawCubeUpdate");
    for(auto cube : cubes)
    {
        delete cube;
    }
}

DrawCube::DrawCube() {
    m_action[Event::MouseClick] = [&]() {
        static bool draw_plane = false;
        static bool draw_cube = false;
        static glm::vec2 pre_pos;
        static glm::vec2 realPos;
        static float height;
        static Cube cube;
        if(draw_cube)
        {
            TaskPool::popUpdateTask("DrawCubeUpdate");
            draw_cube = false;
        }else if (draw_plane) {
            TaskPool::popUpdateTask("DrawCubeUpdate");
            TaskPool::pushUpdateTask("DrawCubeUpdate", [&](float _abs_time){
                auto localCube = cubes.back();
                int id = localCube->id;
                height =  glm::dot(localCube->getTargetPlane().z_coordinate, input::getCursor() - localCube->getWorkCenter());
                genCube(pre_pos, realPos, height, cube);
                auto buffer = SingleBPool.getBuffer(tool::combine("DCube", id), Buffer_Type::Vertex_Host_Buffer);
                memcpy(buffer->mapped, &cube, CubeSize);
            });
            draw_plane = false;
            draw_cube = true;
        } else {
            pre_pos = glm::vec2(1.0f, 0.0f) * glm::dot(SingleAPP.getWorkTargetPlane().x_coordinate, input::getCursor() - SingleAPP.getWorkTargetCenter())
                    + glm::vec2(0.0f, 1.0f) * glm::dot(SingleAPP.getWorkTargetPlane().y_coordinate, input::getCursor() - SingleAPP.getWorkTargetCenter());
            genCube(pre_pos, pre_pos, .0f, cube);
            cubes.emplace_back(new DCube(cube));
            TaskPool::pushUpdateTask("DrawCubeUpdate", [&](float _abs_time){
                auto localCube = cubes.back();
                int id = localCube->id;
                realPos = glm::vec3(1.0f, 0.0f, 0.0f) * glm::dot(localCube->getTargetPlane().x_coordinate, input::getCursor() - localCube->getWorkCenter())
                          + glm::vec3(0.0f, 1.0f, 0.0f) * glm::dot(localCube->getTargetPlane().y_coordinate, input::getCursor() - localCube->getWorkCenter());
                genCube(pre_pos, realPos, .0f, cube);
                auto buffer = SingleBPool.getBuffer(tool::combine("DCube", id), Buffer_Type::Vertex_Host_Buffer);
                memcpy(buffer->mapped, &cube, CubeSize);
            });
            SingleRender.drawChanged = true;
            draw_plane = true;
        };
    };
}
