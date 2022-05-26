//
// Created by jonnxie on 2022/1/28.
//
#include "precompiledhead.h"

#include "tris.h"

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
#include CameraCatalog
#include TexturePoolCatalog
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

void DTris::constructD() {
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
    SingleRender.pushNObjects(d);
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

DPlane::DPlane(const NPlane& _plane,
               bool _textured,
               std::string _setId)
:plane(_plane),
textured(_textured),
setId(std::move(_setId))
{
    id = mallocId();
    m_manipulate = std::make_unique<Manipulate>();
    init();
};

DPlane::~DPlane() {
    if (!m_memReleased) {
        TaskPool::popUpdateTask(tool::combine("DPlane",id));
    }
}

void DPlane::releaseMem() {
    SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Default);
    Object::release();
    TaskPool::popUpdateTask(tool::combine("DPlane",id));
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer(tool::combine("DPlane",id), Buffer_Type::Vertex_Host_Buffer);
    SingleBPool.freeBuffer(tool::combine("DPlane",id), Buffer_Type::Index_Buffer);
    m_memReleased = true;
    SingleRender.normalChanged = true;
}

void DPlane::constructG() {
    SingleBPool.createVertexHostBuffer(tool::combine("DPlane", id), NPlaneSize, &plane);
    SingleBPool.getBuffer(tool::combine("DPlane", id),Buffer_Type::Vertex_Host_Buffer)->map();
    uint32_t indices[6]{0, 2, 1, 0, 3, 2};
    SingleBPool.createIndexBuffer(tool::combine("DPlane", id), 24, indices);
};

void DPlane::constructD(){
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
//    int ms_index = ModelSetPool::getPool().malloc();
    int modelId = m_manipulate->getModelId();

    std::vector<std::string> s_vec(2);
    std::string pipeline;
    s_vec[0]="Camera";
    if (textured) {
        s_vec[1] = setId;
        pipeline = "GPlaneTex";
    } else {
        s_vec[1]="Planet";
        pipeline = "GPlane";
    }
    (*dpool)[d]->m_type = DType::Normal;

    glm::mat4 matrix {
            glm::vec4{SingleAPP.getWorkTargetPlane().x_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetPlane().y_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetPlane().z_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetCenter(), 1.0f},
    };
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->plane = SingleAPP.getWorkTargetPlane();
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = SingleAPP.getWorkTargetCenter();
    m_manipulate->setMatrix(matrix);
    m_manipulate->setChanged(true);
    (*dpool)[d]->prepare(m_manipulate->getMatrix(),
                         modelId,
                         DrawType::Index,
                         0,
                         tool::combine("DPlane", id),
                         4,
                         tool::combine("DPlane", id),
                         6,
                         0,
                         pipeline,
                         s_vec,
                         pipeline,
                         s_vec);
    insertDObject(d);
    SingleRender.pushDObjects(d);
};

DrawNPlane::~DrawNPlane() {
    TaskPool::popUpdateTask("DrawNPlaneUpdate");
}


DrawNPlane::DrawNPlane() {
    m_action[Event::MouseClick] = [&]() {
        static bool draw = false;
        static glm::vec3 pre_pos;
        static glm::vec3 realPos;
        static NPlane plane;
        if (draw) {
            TaskPool::popUpdateTask("DrawNPlaneUpdate");
            draw = false;
        } else {
            computeLocalCoordinate(pre_pos);
            genPlane(pre_pos, pre_pos, plane);
            auto p = std::make_unique<DPlane>(plane);
            planes.push_back(std::move(p));
            TaskPool::pushUpdateTask("DrawNPlaneUpdate",[&](float _abs_time){
                auto localPlane = std::move(planes.back());
                planes.pop_back();
                int id = localPlane->id;
                computeLocalCoordinate(realPos);
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

DCube::DCube(const Cube& _cube,
             bool _textured,
             std::string  _setId):
cube(_cube),
textured(_textured),
setId(_setId)
{
    id = mallocCubeId();
    m_manipulate = std::make_unique<Manipulate>();

    init();
}

DCube::~DCube() {
    TaskPool::popUpdateTask(tool::combine("DCube",id));
}

void DCube::releaseMem() {
    SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Default);
    Object::release();
    TaskPool::popUpdateTask(tool::combine("DCube",id));
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer(tool::combine("DCube",id), Buffer_Type::Vertex_Host_Buffer);
    SingleBPool.freeBuffer(tool::combine("DCube",id), Buffer_Type::Index_Buffer);
    m_memReleased = true;
    SingleRender.normalChanged = true;
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
    int modelIndex = m_manipulate->getModelId();

    std::vector<std::string> s_vec(2);
    std::string pipeline;
    s_vec[0]="Camera";
    s_vec[1]="Planet";
    (*dpool)[d]->m_type = DType::Normal;
    if (textured) {
        s_vec[1] = setId;
        pipeline = "GCubeTex";
    } else {
        s_vec[1]="Planet";
        pipeline = "GCube";
    }
    /*
     * There must have displacement matrix component.
     */
    glm::mat4 matrix{
            glm::vec4{SingleAPP.getWorkTargetPlane().x_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetPlane().y_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetPlane().z_coordinate, 0.0f},
            glm::vec4{SingleAPP.getWorkTargetCenter(), 1.0f},
    };
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->plane = SingleAPP.getWorkTargetPlane();
    (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center = SingleAPP.getWorkTargetCenter();
    m_manipulate->setMatrix(matrix);
    m_manipulate->setChanged(true);
    (*dpool)[d]->prepare(m_manipulate->getMatrix(),
                         modelIndex,
                         DrawType::Index,
                         0,
                         tool::combine("DCube", id),
                         24,
                         tool::combine("DCube", id),
                         36,
                         0,
                         pipeline,
                         s_vec,
                         pipeline,
                         s_vec);
    insertDObject(d);
//    TaskPool::pushUpdateTask(tool::combine("DCube",id),[&, modelIndex, d](float _abs_time){
//        glm::mat4* ptr = SingleBPool.getModels();
//        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
//    });
    SingleRender.pushDObjects(d);
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
        static glm::vec3 preLocalPosition;
        static glm::vec3 realLocalPosition;
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
                height = computeHeight((preLocalPosition + realLocalPosition) / 2.0f);
                genCube(glm::vec2(preLocalPosition.x, preLocalPosition.y), glm::vec2(realLocalPosition.x, realLocalPosition.y), height, cube);
                auto buffer = SingleBPool.getBuffer(tool::combine("DCube", id), Buffer_Type::Vertex_Host_Buffer);
                memcpy(buffer->mapped, &cube, CubeSize);
            });
            draw_plane = false;
            draw_cube = true;
        } else {
            computeLocalCoordinate(preLocalPosition);
            genCube(glm::vec2(preLocalPosition.x, preLocalPosition.y), glm::vec2(preLocalPosition.x, preLocalPosition.y), .0f, cube);
            cubes.emplace_back(new DCube(cube));
            TaskPool::pushUpdateTask("DrawCubeUpdate", [&](float _abs_time){
                auto localCube = cubes.back();
                int id = localCube->id;
                computeLocalCoordinate(realLocalPosition);
                genCube(glm::vec2(preLocalPosition.x, preLocalPosition.y), glm::vec2(realLocalPosition.x, realLocalPosition.y), .0f, cube);
                auto buffer = SingleBPool.getBuffer(tool::combine("DCube", id), Buffer_Type::Vertex_Host_Buffer);
                memcpy(buffer->mapped, &cube, CubeSize);
            });
            SingleRender.drawChanged = true;
            draw_plane = true;
        };
    };
}

DPlaneHandle::DPlaneHandle() {
    TaskPool::pushTask([&](){
        m_pipeline = "GPlane";
        m_sets = {"Camera", "Planet"};
        m_color = RED_COLOR;
        m_listener = new DrawNPlaneHandle(this);
        pushUI();
    });
}

DPlaneHandle::~DPlaneHandle() {
    GUI::popUI("DPlaneHandle");
    TaskPool::popUpdateTask("DPlaneHandle");
}

NPlane &DPlaneHandle::operator[](size_t _index) {
    return planes[_index]->plane;
}

void DPlaneHandle::pushNPlane(const NPlane &_plane) {
    planes.emplace_back(std::make_unique<DPlane>(_plane));
}

void DPlaneHandle::pushDPlane(std::unique_ptr<DPlane> _plane) {
    planes.emplace_back(std::move(_plane));
}

void DPlaneHandle::pushUI() {
    GUI::pushUI("DPlaneHandle", [&](){
        ImGui::Begin("DPlaneHandleSetting");

        static char buf[32] = "default";
        ImGui::InputText("filename", buf, IM_ARRAYSIZE(buf));
        if(ImGui::Button("LoadFile"))
        {
            loadFile(std::string(buf) + ".gltf");
        }

        if(ImGui::Button("drawPlane"))
        {
            drawNPlane();
        }

        if (ImGui::Button("batch"))
        {
            batch();
        }

        if (ImGui::TreeNode("Select Pipeline"))
        {
            static int selected = -1;
            int num = 0;
            for(auto& [id, val] : SinglePPool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selected == num))
                {
                    selected = num;
                    m_pipeline = id;
                }
                num++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("AddTexture"))
        {
            static int selected = -1;
            int num = 0;
            for(auto& [id, val] : SingleTexturePool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selected == num))
                {
                    selected = num;
                    setTextureId(id);
                }
                num++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Select Descriptor Sets"))
        {
            ShowHelpMarker("Hold CTRL and click to select multiple items.");
            auto count = SingleSetPool.m_map.size();
            auto static selection = std::vector<int>(count);
            int num = 0;
            for(auto& [id, val] : SingleSetPool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selection[num] == 1))
                {
                    if (!checkKey(GLFW_KEY_LEFT_CONTROL) && !checkKey(GLFW_KEY_RIGHT_CONTROL))
                    {
                        for(auto& s : selection)
                        {
                            s = 0;
                        }
                        m_sets.clear();
                    }// Clear selection when CTRL is not held
                    selection[num] ^= 1;
                    m_sets.push_back(id);
                }
                num++;
            }
            ImGui::TreePop();
        }

        ImGui::SliderFloat3("Color",
                            reinterpret_cast<float *>(&m_color),
                            0.0f,
                            1.0f);
        ImGui::SliderFloat3("Coordinate",
                            reinterpret_cast<float *>(&m_localCoordiante),
                            std::numeric_limits<float>::min() / 2.0f,
                            std::numeric_limits<float>::max() / 2.0f);

        if(ImGui::Button("end")) {
            destroy();
        }

        ImGui::End();// End setting
    });
}

int DPlaneHandle::getNPlaneCount() {
    return planes.size();
}

void DPlaneHandle::releaseMem() {
    SingleRender.releaseObject(drawId, DrawObjectType::Default);
    auto dpool = MPool<DObject>::getPool();
    dpool->free(drawId);
    auto& model_pool = ModelSetPool::getPool();
    model_pool.free(modelId);
    TaskPool::popUpdateTask("DPlaneHandle");
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer("DPlaneHandle", Buffer_Type::Vertex_Buffer);
    SingleBPool.freeBuffer("DPlaneHandle", Buffer_Type::Index_Buffer);
}

void DPlaneHandle::batch() {
    if (!batched) {
        batched = true;
    } else {
        releaseMem();
    }
    int planeCount = planes.size();
    std::vector<NPlane> batch_plane(planeCount);
    std::vector<uint32_t> batch_index(planeCount * 6);
    int indexOffset = 0;
    for (int i = 0; i < planeCount; ++i) {
        batch_plane[i] = planes[i]->plane;
        indexOffset = i * 6;
        batch_index[indexOffset]     = i * 4;
        batch_index[indexOffset + 1] = i * 4 + 2;
        batch_index[indexOffset + 2] = i * 4 + 1;
        batch_index[indexOffset + 3] = i * 4;
        batch_index[indexOffset + 4] = i * 4 + 3;
        batch_index[indexOffset + 5] = i * 4 + 2;
        planes[i]->releaseMem();
    }
    SingleBPool.createVertexHostBuffer("DPlaneHandle", NPlaneSize * planeCount, batch_plane.data());
    SingleBPool.createIndexBuffer("DPlaneHandle", 4 * batch_index.size(), batch_index.data());

    auto dpool = MPool<DObject>::getPool();
    drawId = dpool->malloc();
    modelId = ModelSetPool::getPool().malloc();

    glm::mat4 mat(1.0f);
    if(m_localCoordiante != -1)
    {
        Target* target = (*MPool<Target>::getPool())[m_localCoordiante];

        mat = glm::mat4 {
                glm::vec4{target->plane.x_coordinate, 0.0f},
                glm::vec4{target->plane.y_coordinate, 0.0f},
                glm::vec4{target->plane.z_coordinate, 0.0f},
                glm::vec4{target->center, 1.0f},
        };
    }
    (*dpool)[drawId]->m_type = DType::Normal;
    (*dpool)[drawId]->prepare(mat,
                              modelId,
                              DrawType::Index,
                              0,
                              "DPlaneHandle",
                              planeCount * 4,
                              "DPlaneHandle",
                              batch_index.size(),
                              0,
                              m_pipeline,
                              m_sets,
                              m_pipeline,
                              m_sets);
    TaskPool::pushUpdateTask("DPlaneHandle",[&](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + modelId, &(*SingleDPool)[drawId]->m_matrix, one_matrix);
    });
    SingleRender.pushDObjects(drawId);
    SingleRender.drawChanged = true;
}

void DPlaneHandle::loadFile(const std::string &_filename) {
    auto planeCount = planes.size();

    size_t vertexCount = 4 * planeCount;
    std::vector<glm::vec3> pos_vec(vertexCount);
    std::vector<glm::vec2> uv_vec(vertexCount);

    size_t indexCount = 6 * planeCount;
    std::vector<uint32_t> index_vec(indexCount);

    for (int i = 0; i < planeCount; ++i) {
        pos_vec[i * 4] = planes[i]->plane.points[0].pos;
        printPoint(planes[i]->plane.points[0].pos);
        uv_vec[i * 4] = planes[i]->plane.points[0].uv;

        pos_vec[i * 4 + 1] = planes[i]->plane.points[1].pos;
        printPoint(planes[i]->plane.points[1].pos);
        uv_vec[i * 4 + 1] = planes[i]->plane.points[1].uv;

        pos_vec[i * 4 + 2] = planes[i]->plane.points[2].pos;
        printPoint(planes[i]->plane.points[2].pos);
        uv_vec[i * 4 + 2] = planes[i]->plane.points[2].uv;

        pos_vec[i * 4 + 3] = planes[i]->plane.points[3].pos;
        printPoint(planes[i]->plane.points[3].pos);
        uv_vec[i * 4 + 3] = planes[i]->plane.points[3].uv;

        index_vec[i * 6]     = i * 4;
        index_vec[i * 6 + 1] = i * 4 + 2;
        index_vec[i * 6 + 2] = i * 4 + 1;
        index_vec[i * 6 + 3] = i * 4;
        index_vec[i * 6 + 4] = i * 4 + 3;
        index_vec[i * 6 + 5] = i * 4 + 2;
    }
    std::vector<void*> data_vec{pos_vec.data(), uv_vec.data()};

    vkglTF::Model::writeMeshToFile(_filename,
                         vertexCount,
                         data_vec,
                         index_vec,
                         std::vector<vkglTF::VertexComponent>{vkglTF::VertexComponent::Position,
                                                                          vkglTF::VertexComponent::UV});
}

void DPlaneHandle::drawNPlane() {
    if (appendState) {
        SingleAPP.appendListener("drawNPlane", m_listener);
        appendState = false;
    } else {
        SingleAPP.removeListener("drawNPlane");
        appendState = true;
    }
}

void DPlaneHandle::destroy() const {
    if (!appendState){
        TaskPool::pushTask([](){
            SingleAPP.deleteListener("drawNPlane");
        });
    } else {
        TaskPool::pushTask([=](){
            delete m_listener;
        });
    }
}

DrawNPlaneHandle::DrawNPlaneHandle(DPlaneHandle *_handle):
        handle(_handle) {
    m_action[Event::MouseClick] = [&]() {
        static bool draw = false;
        static glm::vec3 pre_pos;
        static glm::vec3 realPos;
        static NPlane plane;
        if (draw) {
            TaskPool::popUpdateTask("DrawNPlaneUpdate");
            draw = false;
        } else {
            computeLocalCoordinate(pre_pos, handle->getCoordinate());
            genPlane(pre_pos, pre_pos, plane);
            auto p = std::make_unique<DPlane>(plane,
                                              handle->getTextured(),
                                              handle->getTextureId());
            handle->pushDPlane(std::move(p));
            TaskPool::pushUpdateTask("DrawNPlaneUpdate",[&](float _abs_time){
                auto localPlane = std::move(handle->getPlanes().back());
                handle->getPlanes().pop_back();
                int id = localPlane->id;
                computeLocalCoordinate(realPos, handle->getCoordinate());
                genPlane(pre_pos, realPos, plane);
                auto buffer = SingleBPool.getBuffer(tool::combine("DPlane", id), Buffer_Type::Vertex_Host_Buffer);
                auto* ptr = (Point*)buffer->mapped;
                localPlane->plane = plane;
                memcpy(ptr, &plane, NPlaneSize);
                handle->getPlanes().push_back(std::move(localPlane));
            });
            SingleRender.drawChanged = true;
            draw = true;
        };
    };
}

DrawNPlaneHandle::~DrawNPlaneHandle() {
    delete handle;
}

DCubeHandle::DCubeHandle() {
    TaskPool::pushTask([&](){
        m_pipeline = "GCube";
        m_sets = {"Camera", "Planet"};
        m_color = RED_COLOR;
        m_listener = new DrawCubeHandle(this);
        pushUI();
    });
}

DCubeHandle::~DCubeHandle() {
    GUI::popUI("DCubeHandle");
}

Cube &DCubeHandle::operator[](size_t _index) {
    return cubes[_index]->cube;
}

void DCubeHandle::pushCube(const Cube &_cube) {
    cubes.emplace_back(std::make_unique<DCube>(_cube));
}

void DCubeHandle::pushDCube(std::unique_ptr<DCube> _cube) {
    cubes.push_back(std::move(_cube));
}

void DCubeHandle::pushUI() {
    GUI::pushUI("DCubeHandle", [&](){
        ImGui::Begin("DCubeHandleSetting");

        static char buf[32] = "default";
        ImGui::InputText("filename", buf, IM_ARRAYSIZE(buf));
        if (ImGui::Button("LoadFile"))
        {
            loadFile(std::string(buf) + ".gltf");
        }

        if (ImGui::Button("drawCube"))
        {
            drawCube();
        }

        if (ImGui::Button("batch"))
        {
            batch();
        }

        if (ImGui::TreeNode("Select Pipeline"))
        {
            static int selected = -1;
            int num = 0;
            for(auto& [id, val] : SinglePPool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selected == num))
                {
                    selected = num;
                    m_pipeline = id;
                }
                num++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("AddTexture"))
        {
            static int selected = -1;
            int num = 0;
            for(auto& [id, val] : SingleTexturePool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selected == num))
                {
                    selected = num;
                    setTextureId(id);
                }
                num++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Select Descriptor Sets"))
        {
            ShowHelpMarker("Hold CTRL and click to select multiple items.");
            auto count = SingleSetPool.m_map.size();
            auto static selection = std::vector<int>(count);
            int num = 0;
            for(auto& [id, val] : SingleSetPool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selection[num] == 1))
                {
                    if (!checkKey(GLFW_KEY_LEFT_CONTROL) && !checkKey(GLFW_KEY_RIGHT_CONTROL))
                    {
                        for(auto& s : selection)
                        {
                            s = 0;
                        }
                        m_sets.clear();
                    }// Clear selection when CTRL is not held
                    selection[num] ^= 1;
                    m_sets.push_back(id);
                }
                num++;
            }
            ImGui::TreePop();
        }

        ImGui::SliderFloat3("Color",
                            reinterpret_cast<float *>(&m_color),
                            0.0f,
                            1.0f);
        ImGui::SliderFloat3("Coordinate",
                            reinterpret_cast<float *>(&m_localCoordiante),
                            std::numeric_limits<float>::min() / 2.0f,
                            std::numeric_limits<float>::max() / 2.0f);

        if (ImGui::Button("end")) {
            destroy();
        }

        ImGui::End();// End setting
    });

}

int DCubeHandle::getCubeCount() {
    return cubes.size();
}

void DCubeHandle::releaseMem() {
    SingleRender.releaseObject(drawId, DrawObjectType::Default);
    auto dpool = MPool<DObject>::getPool();
    dpool->free(drawId);
    auto& model_pool = ModelSetPool::getPool();
    model_pool.free(modelId);
    TaskPool::popUpdateTask("DCubeHandle");
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer("DCubeHandle", Buffer_Type::Vertex_Buffer);
    SingleBPool.freeBuffer("DCubeHandle", Buffer_Type::Index_Buffer);
}

void DCubeHandle::batch() {
    if (!batched) {
        batched = true;
    } else {
        releaseMem();
    }
    auto cubeCount = cubes.size();
    size_t vertexCount = 24 * cubeCount;
    std::vector<Cube::Plane::Point> point_vec(vertexCount);
    size_t indexCount = 36 * cubeCount;
    std::vector<uint32_t> index_vec(indexCount);

    for (int i = 0; i < cubeCount; ++i) {
        memcpy(point_vec.data() + i * CubeSize, &cubes[i]->cube, CubeSize);
        cubes[i]->releaseMem();
    }
    combineIndex(index_vec);
    SingleBPool.createVertexHostBuffer("DCubeHandle", CubeSize * cubeCount, point_vec.data());
    SingleBPool.createIndexBuffer("DCubeHandle", 4 * index_vec.size(), index_vec.data());

    auto dpool = MPool<DObject>::getPool();
    drawId = dpool->malloc();
    modelId = ModelSetPool::getPool().malloc();

    glm::mat4 mat(1.0f);
    if(m_localCoordiante != -1)
    {
        Target* target = (*MPool<Target>::getPool())[m_localCoordiante];

        mat = glm::mat4 {
                glm::vec4{target->plane.x_coordinate, 0.0f},
                glm::vec4{target->plane.y_coordinate, 0.0f},
                glm::vec4{target->plane.z_coordinate, 0.0f},
                glm::vec4{target->center, 1.0f},
        };
    }
    (*dpool)[drawId]->m_type = DType::Normal;
    (*dpool)[drawId]->prepare(mat,
                              modelId,
                              DrawType::Index,
                              0,
                              "DCubeHandle",
                              vertexCount,
                              "DCubeHandle",
                              indexCount,
                              0,
                              m_pipeline,
                              m_sets,
                              m_pipeline,
                              m_sets);
    TaskPool::pushUpdateTask("DCubeHandle",[&](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + modelId, &(*SingleDPool)[drawId]->m_matrix, one_matrix);
    });
    SingleRender.pushDObjects(drawId);
    SingleRender.drawChanged = true;
}

void DCubeHandle::combine(std::vector<glm::vec3>& _posVec,
             std::vector<glm::vec3>& _normalVec,
             std::vector<glm::vec2>& _uvVec,
             std::vector<uint32_t>& _index) {
    auto cubeCount = cubes.size();
    uint32_t baseVertex = 0;
    uint32_t baseIndex = 0;
    uint32_t baseFaceVertex = 0;
    uint32_t baseFaceIndex = 0;
    for (int cubeIndex = 0; cubeIndex < cubeCount; ++cubeIndex) {
        baseVertex = cubeIndex * 24;
        baseIndex = cubeIndex * 36;
        Cube& cube = cubes[cubeIndex]->cube;
        for (int faceIndex = 0; faceIndex < int(CubePlane::PlaneCount); faceIndex++) {
            Cube::Plane& plane = cube.planes[faceIndex];
            baseFaceVertex = faceIndex * 4;
            baseFaceIndex = cubeIndex * 6;
            _posVec[baseVertex + baseFaceVertex] = plane.points[0].pos;
            _normalVec[baseVertex + baseFaceVertex] = plane.points[0].normal;
            _uvVec[baseVertex + baseFaceVertex] = plane.points[0].uv;

            _posVec[baseVertex + baseFaceVertex + 1] = plane.points[1].pos;
            _normalVec[baseVertex + baseFaceVertex + 1] = plane.points[1].normal;
            _uvVec[baseVertex + baseFaceVertex + 1] = plane.points[1].uv;

            _posVec[baseVertex + baseFaceVertex + 2] = plane.points[2].pos;
            _normalVec[baseVertex + baseFaceVertex + 2] = plane.points[2].normal;
            _uvVec[baseVertex + baseFaceVertex + 2] = plane.points[2].uv;

            _posVec[baseVertex + baseFaceVertex + 3] = plane.points[3].pos;
            _normalVec[baseVertex + baseFaceVertex + 3] = plane.points[3].normal;
            _uvVec[baseVertex + baseFaceVertex + 3] = plane.points[3].uv;
        }
    }
    combineIndex(_index);
}

void DCubeHandle::combineIndex(std::vector<uint32_t>& _index) {
    auto cubeCount = cubes.size();
    uint32_t baseVertex = 0;
    uint32_t baseIndex = 0;
    uint32_t baseFaceVertex = 0;
    uint32_t baseFaceIndex = 0;
    for (int cubeIndex = 0; cubeIndex < cubeCount; ++cubeIndex) {
        baseVertex = cubeIndex * 24;
        baseIndex = cubeIndex * 36;
        Cube& cube = cubes[cubeIndex]->cube;
        for (int faceIndex = 0; faceIndex < int(CubePlane::PlaneCount); faceIndex++) {
            Cube::Plane& plane = cube.planes[faceIndex];
            baseFaceVertex = faceIndex * 4;
            baseFaceIndex = faceIndex * 6;
            _index[baseIndex + baseFaceIndex]     = baseVertex + baseFaceVertex;
            _index[baseIndex + baseFaceIndex + 1] = baseVertex + baseFaceVertex + 2;
            _index[baseIndex + baseFaceIndex + 2] = baseVertex + baseFaceVertex + 1;
            _index[baseIndex + baseFaceIndex + 3] = baseVertex + baseFaceVertex;
            _index[baseIndex + baseFaceIndex + 4] = baseVertex + baseFaceVertex + 3;
            _index[baseIndex + baseFaceIndex + 5] = baseVertex + baseFaceVertex + 2;
        }
    }
}

void DCubeHandle::loadFile(const std::string &_filename) {
    auto cubeCount = cubes.size();
    size_t vertexCount = 24 * cubeCount;
    std::vector<glm::vec3> pos_vec(vertexCount);
    std::vector<glm::vec3> normal_vec(vertexCount);
    std::vector<glm::vec2> uv_vec(vertexCount);

    size_t indexCount = 36 * cubeCount;
    std::vector<uint32_t> index_vec(indexCount);

    combine(pos_vec, normal_vec, uv_vec, index_vec);
    std::vector<void*> data_vec{pos_vec.data(), normal_vec.data(), uv_vec.data()};

    vkglTF::Model::writeMeshToFile(_filename,
                                   vertexCount,
                                   data_vec,
                                   index_vec,
                                   std::vector<vkglTF::VertexComponent>{vkglTF::VertexComponent::Position,
                                                                                    vkglTF::VertexComponent::Normal,
                                                                                    vkglTF::VertexComponent::UV});
}

void DCubeHandle::drawCube() {
    if (appendState) {
        SingleAPP.appendListener("drawCube", m_listener);
        appendState = false;
    } else {
        SingleAPP.removeListener("drawCube");
        appendState = true;
    }
}

void DCubeHandle::destroy() const {
    if (!appendState){
        TaskPool::pushTask([](){
            SingleAPP.deleteListener("drawCube");
        });
    } else {
        TaskPool::pushTask([=](){
            delete m_listener;
        });
    }
}

DrawCubeHandle::DrawCubeHandle(DCubeHandle *_handle):
        handle(_handle) {
    m_action[Event::MouseClick] = [&]() {
        static bool draw_plane = false;
        static bool draw_cube = false;
        static glm::vec3 preLocalPosition;
        static glm::vec3 realLocalPosition;
        static float height;
        static Cube cube;
        if(draw_cube)
        {
            TaskPool::popUpdateTask("DrawCubeUpdate");
            draw_cube = false;
        }else if (draw_plane) {
            TaskPool::popUpdateTask("DrawCubeUpdate");
            TaskPool::pushUpdateTask("DrawCubeUpdate", [&](float _abs_time){
                int id = handle->getCubes().back()->id;
                height = computeHeight((preLocalPosition + realLocalPosition) / 2.0f);
                genCube(glm::vec2(preLocalPosition.x, preLocalPosition.y), glm::vec2(realLocalPosition.x, realLocalPosition.y), height, cube);
                handle->getCubes().back()->cube = cube;
                auto buffer = SingleBPool.getBuffer(tool::combine("DCube", id), Buffer_Type::Vertex_Host_Buffer);
                memcpy(buffer->mapped, &cube, CubeSize);
            });
            draw_plane = false;
            draw_cube = true;
        } else {
            computeLocalCoordinate(preLocalPosition);
            genCube(glm::vec2(preLocalPosition.x, preLocalPosition.y), glm::vec2(preLocalPosition.x, preLocalPosition.y), .0f, cube);
            handle->getCubes().emplace_back(std::make_unique<DCube>(cube,
                                                                    handle->getTextured(),
                                                                    handle->getTextureId()));
            handle->getCubes().back()->cube = cube;
            TaskPool::pushUpdateTask("DrawCubeUpdate", [&](float _abs_time){
                int id = handle->getCubes().back()->id;
                computeLocalCoordinate(realLocalPosition);
                genCube(glm::vec2(preLocalPosition.x, preLocalPosition.y), glm::vec2(realLocalPosition.x, realLocalPosition.y), .0f, cube);
                auto buffer = SingleBPool.getBuffer(tool::combine("DCube", id), Buffer_Type::Vertex_Host_Buffer);
                handle->getCubes().back()->cube = cube;
                memcpy(buffer->mapped, &cube, CubeSize);
            });
            SingleRender.drawChanged = true;
            draw_plane = true;
        };
    };
}

DrawCubeHandle::~DrawCubeHandle() {
    TaskPool::popUpdateTask("DrawCubeHandle");
    delete handle;
}
