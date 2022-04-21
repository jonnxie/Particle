//
// Created by AnWell on 2022/2/10.
//
#include <Engine/Pool/TexturePool.h>
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
#include ConfigCatalog
#include "Engine/Object/inputaction.h"

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

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

DPointPool::DPointPool(const std::vector<Point3dColorSize> &_points, int _coordinate, bool _updateFunc, std::string _pipeline,
                       std::vector<std::string> _sets):
        updateFunc(_updateFunc),
        m_pipeline(std::move(_pipeline)),
        m_sets(std::move(_sets)){
    m_localCoordiante = _coordinate;
    m_points = _points;
    id = mallocId();
    pointResolveCount = Config::getConfig("PointPoolInitialCount");
    pointCount = _points.size();
    poolSize = pointCount * Point3dColorSizeSize;
    init();
}

DPointPool::~DPointPool() {
    TaskPool::popUpdateTask(tool::combine("PointPoolBasic", id));
}

void DPointPool::constructG() {
    SingleBPool.createVertexHostBuffer(tool::combine("DPointPool",id),
                                       Point3dColorSizeSize * pointResolveCount,
                                       m_points.data());
    SingleBPool.getBuffer(tool::combine("DPointPool",id), Buffer_Type::Vertex_Host_Buffer)->map();
}

void DPointPool::constructD() {
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int ms_index = ModelSetPool::getPool().malloc();
    auto buffer = SingleBPool.getBuffer(tool::combine("DPointPool",id),Buffer_Type::Vertex_Host_Buffer);

    glm::mat4 mat(1.0f);
    if(m_localCoordiante != -1)
    {
        Target* target = (*MPool<Target>::getPool())[m_localCoordiante];

        mat = glm::mat4{
                glm::vec4{target->plane.x_coordinate, 0.0f},
                glm::vec4{target->plane.y_coordinate, 0.0f},
                glm::vec4{target->plane.z_coordinate, 0.0f},
                glm::vec4{target->center, 1.0f},
        };
    }
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(mat,
                         ms_index,
                         DrawType::Vertex,
                         0,
                         tool::combine("DPointPool",id),
                         pointResolveCount,
                         "",
                         0,
                         0,
                         m_pipeline,
                         m_sets);
    insertDObject(d);
    if(updateFunc)
    {
        TaskPool::pushUpdateTask(tool::combine("PointPoolBasic", id),[&, ms_index, d, buffer](float _abs_time){
            memcpy(buffer->mapped, m_points.data(), poolSize);
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + ms_index,&(*SingleDPool)[d]->m_matrix,one_matrix);
        });
    }
    SingleRender.getNObjects()->push_back(d);
}

void DPointPool::pushPoint(const Point3dColorSize &_point) {
    m_points.push_back(_point);
    poolSize += Point3dColorSizeSize;
    if(++pointCount >= pointResolveCount)
    {
        reallocated();
    }
}

void DPointPool::pushPoints(const std::vector<Point3dColorSize> &_points) {
    m_points.insert(m_points.end(), _points.begin(), _points.end());
    pointCount += _points.size();
    poolSize += _points.size() * Point3dColorSizeSize;
    if(pointCount >= pointResolveCount)
    {
        reallocated();
    }
}

void DPointPool::reallocated() {
    SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Normal);
    Object::release();
    if(updateFunc)
    {
        TaskPool::popUpdateTask(tool::combine("PointPoolBasic",id));
    }
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer(tool::combine("DPointPool",id), Buffer_Type::Vertex_Host_Buffer);
    pointResolveCount *= 2;
    init();
    SingleRender.normalChanged = true;
}

PointsHandle::PointsHandle() {
    m_pipeline = "Point";
    m_sets = {"Camera", "ViewPort"};
    m_size = 2;
    m_listener = new DrawPointHandle(this);
    m_points = std::make_unique<DPointPool>(std::vector<Point3dColorSize>(), m_localCoordiante, true, m_pipeline, m_sets);
    m_sets.emplace_back("test");
    m_pointsTex = std::make_unique<DPointPool>(std::vector<Point3dColorSize>(), m_localCoordiante, true, "PointTex", m_sets);
    pushUI();
}

PointsHandle::~PointsHandle() {
    GUI::popUI("PointsHandle");
}

Point3dColorSize &PointsHandle::operator[](size_t _index) {
    return m_points->getPoints()[_index];
}

void PointsHandle::pushPoint(const glm::vec3 &_point, bool _textured) {
    return pushPoint(Point3dColorSize{
                             _point,
                             m_color,
                             m_size
                     },
                     _textured);
}

void PointsHandle::pushPoint(const Point3dColorSize &_point, bool _textured) {
    if (!_textured) {
        return m_points->pushPoint(_point);
    } else {
        return m_pointsTex->pushPoint(_point);
    }
}

void PointsHandle::pushPoints(const std::vector<glm::vec3> &_points) {
    std::vector<Point3dColorSize> points(_points.size());
    for (int i = 0; i < points.size(); ++i) {
        new ((Line*)&points[i]) Point3dColorSize{_points[i], m_color, m_size};
    }
    return m_points->pushPoints(points);
}

void PointsHandle::pushUI() {
    GUI::pushUI("PointsHandle", [&](){
        ImGui::Begin("PointsHandleSetting");

        static char buf[32] = "default";
        ImGui::InputText("filename", buf, IM_ARRAYSIZE(buf));
        if(ImGui::Button("LoadFile"))
        {
            loadFile(std::string(buf) + ".gltf");
        }

        if(ImGui::Button("drawPoint"))
        {
            drawPoint();
        }

        if(ImGui::Button("drawPointTex"))
        {
            drawPointTex();
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
                    m_pointsTex->getSets()[2]= id;
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
        ImGui::SliderFloat("Size",
                           reinterpret_cast<float *>(&m_size),
                           0.0f,
                           2.0f);
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

int PointsHandle::getPointCount() {
    return m_points->getPointCount();
}

void PointsHandle::loadFile(const std::string &_filename) {
    size_t count = m_points->getPointCount();
    std::vector<glm::vec3> pos_vec(count);
    std::vector<glm::vec3> color_vec(count);

    for(size_t index = 0; index < count; index++)
    {
        pos_vec[index] = m_points->getPoints()[index].pos;
        color_vec[index] = m_points->getPoints()[index].color;
    }
    std::vector<void*> data_vec{pos_vec.data(), color_vec.data()};
    vkglTF::Model::writeGeometryListToFile(_filename,
                                           count,
                                           data_vec,
                                           std::vector<vkglTF::VertexComponent>{vkglTF::VertexComponent::Position,
                                                                                vkglTF::VertexComponent::Color},
                                           MeshDrawType::Point
    );
}

void PointsHandle::drawPoint() {
    if (appendState) {
        SingleAPP.appendListener("DrawPoint", m_listener);
        appendState = false;
    } else {
        SingleAPP.removeListener("DrawPoint");
        appendState = true;
    }
}

void PointsHandle::drawPointTex() {
    if (appendTexState) {
        m_listener->textured = true;
        appendTexState = false;
    } else {
        m_listener->textured = false;
        appendTexState = true;
    }
}

void PointsHandle::destroy() const {
    if (!appendState){
        TaskPool::pushTask([](){
            SingleAPP.deleteListener("DrawPoint");
        });
    } else {
        TaskPool::pushTask([=](){
            delete m_listener;
        });
    }
}

DrawPointHandle::DrawPointHandle(PointsHandle *_handle):
        handle(_handle)
{
    m_action[Event::MouseClick] = [&]() {
        static glm::vec3 dis;
        static glm::vec3 local;
        glm::vec3& pre_pos = input::getCursor();
        dis = input::getCursor() - handle->getWorkCenter();
        local = {glm::dot(dis, handle->getTargetPlane().x_coordinate),
                 glm::dot(dis, handle->getTargetPlane().y_coordinate),
                 glm::dot(dis, handle->getTargetPlane().z_coordinate)};
        handle->pushPoint(local, textured);
    };
}

DrawPointHandle::~DrawPointHandle() {
    delete handle;
}
