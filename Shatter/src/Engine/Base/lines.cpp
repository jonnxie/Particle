//
// Created by jonnxie on 2022/1/24.
//
#include "precompiledhead.h"

#include "lines.h"
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
#include GuiCatalog
#include InputActionCatalog
#include CameraCatalog
#include GLTFCatalog
#include AABBCatalog
#include CaptureCatalog

static int initIdVal = 0;
static std::mutex idLock;

static int mallocId()
{
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

DLines::DLines(const std::vector<Line>& _lines, bool _updateFunc, int _modelIndex, bool _capture):
updateFunc(_updateFunc),
m_modelIndex(_modelIndex),
capture(_capture)
{
    lines = _lines;
    id = mallocId();
    m_boxIndex = MPool<AABB>::getPool()->malloc();
    for(auto& line : lines)
    {
        (*SingleAABBPool)[m_boxIndex]->addInternalPoint(line.begin.pos);
        (*SingleAABBPool)[m_boxIndex]->addInternalPoint(line.end.pos);
    }
}

DLines::~DLines(){
    TaskPool::popUpdateTask(tool::combine("LinesBasic",id));
}

void DLines::destroy(){
    SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Normal);
    Object::release();
    if(updateFunc)
    {
        TaskPool::popUpdateTask(tool::combine("LinesBasic",id));
    }
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer(tool::combine("DLines",id), Buffer_Type::Vertex_Host_Buffer);
    SingleRender.releaseObject(int(m_capture_id),DrawObjectType::AABB);
    SingleRender.normalChanged = true;
}

void DLines::constructG(){
    SingleBPool.createVertexHostBuffer(tool::combine("DLines",id),LineSize * lines.size(),lines.data());
    SingleBPool.getBuffer(tool::combine("DLines",id),Buffer_Type::Vertex_Host_Buffer)->map();
}

void DLines::constructD(){
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int model_index ;
    if (m_modelIndex == -1) {
        model_index = ModelSetPool::getPool().malloc();
    } else {
        model_index = m_modelIndex;
    }

    std::vector<std::string> s_vec(1);
    s_vec[0]="Camera";
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(glm::mat4(1.0f),
                         model_index,
                         DrawType::Vertex,
                         0,
                         tool::combine("DLines",id),
                         lines.size() * 2,
                         "",
                         0,
                         0,
                         "Polyline",
                         s_vec);
    insertDObject(d);
    if(updateFunc)
    {
        TaskPool::pushUpdateTask(tool::combine("LinesBasic",id),[&,model_index,d](float _abs_time){
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + model_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
        });
    }
    SingleRender.getNObjects()->push_back(d);
    if (capture) {
        m_captureObject = std::make_shared<CaptureObject>(this, m_boxIndex, d);
        SingleAPP.capturedPush(m_captureObject);
    }
}

void DLines::pushLine(const Line &_line) {
    lines.push_back(_line);
    (*SingleAABBPool)[m_boxIndex]->addInternalPoint(_line.begin.pos);
    (*SingleAABBPool)[m_boxIndex]->addInternalPoint(_line.end.pos);
}

void DLines::pushLines(const std::vector<Line>& _lines) {
    lines.insert(lines.end(),_lines.begin(),_lines.end());
    for(auto& line : _lines)
    {
        (*SingleAABBPool)[m_boxIndex]->addInternalPoint(line.begin.pos);
        (*SingleAABBPool)[m_boxIndex]->addInternalPoint(line.end.pos);
    }
}

void DLines::hide() {
    if (showed) {
        SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Normal);
        SingleRender.normalChanged = true;
        showed = false;
    }
}

void DLines::show() {
    if (!showed) {
        SingleRender.getNObjects()->push_back(m_dobjs[0]);
        SingleRender.normalChanged = true;
        showed = true;
    }
}

DLinePool::DLinePool(const std::vector<Line>& _lines,
                     int _coordinate,
                     bool _updateFunc,std::string _pipeline,
                     std::vector<std::string> _sets):
                     updateFunc(_updateFunc),
                     m_pipeline(std::move(_pipeline)),
                     m_sets(std::move(_sets)){
    m_localCoordiante = _coordinate;
    m_lines = _lines;
    id = mallocId();
    lineResolveCount = Config::getConfig("LinePoolInitialCount");
    lineCount = _lines.size();
    poolSize = lineCount * LineSize;
    init();
}

void DLinePool::constructG(){
    SingleBPool.createVertexHostBuffer(tool::combine("DLinePool",id), LineSize * lineResolveCount, m_lines.data());
    SingleBPool.getBuffer(tool::combine("DLinePool",id), Buffer_Type::Vertex_Host_Buffer)->map();
}

void DLinePool::constructD(){
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int ms_index = ModelSetPool::getPool().malloc();
    auto buffer = SingleBPool.getBuffer(tool::combine("DLinePool",id),Buffer_Type::Vertex_Host_Buffer);

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
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(mat,
                         ms_index,
                         DrawType::Vertex,
                         0,
                         tool::combine("DLinePool",id),
                         lineResolveCount * 2,
                         "",
                         0,
                         0,
                         m_pipeline,
                         m_sets);
    insertDObject(d);
    if(updateFunc)
    {
        TaskPool::pushUpdateTask(tool::combine("LinePoolBasic", id),[&, ms_index, d, buffer](float _abs_time){
            memcpy(buffer->mapped, m_lines.data(), poolSize);
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + ms_index,&(*SingleDPool)[d]->m_matrix,one_matrix);
        });
    }
    SingleRender.getNObjects()->push_back(d);
}

DLinePool::~DLinePool(){
    TaskPool::popUpdateTask(tool::combine("LinePoolBasic", id));
}

void DLinePool::pushLine(const Line &_line) {
    m_lines.push_back(_line);
    poolSize += LineSize;
    if(++lineCount >= lineResolveCount)
    {
        reallocated();
    }
}

void DLinePool::pushLines(const std::vector<Line>& _lines) {
    m_lines.insert(m_lines.end(),_lines.begin(),_lines.end());
    lineCount += _lines.size();
    poolSize += _lines.size() * LineSize;
    if(lineCount >= lineResolveCount)
    {
        reallocated();
    }
}

void DLinePool::reallocated(){
    SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Normal);
    Object::release();
    if(updateFunc)
    {
        TaskPool::popUpdateTask(tool::combine("LinePoolBasic",id));
    }
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer(tool::combine("DLinePool",id), Buffer_Type::Vertex_Host_Buffer);
    lineResolveCount *= 2;
    init();
    SingleRender.normalChanged = true;
}

LineHandle::LineHandle() {
    m_pipeline = "Polyline";
    m_sets = {"Camera"};
    m_listener = new DrawLineHandle(this);
    m_lines = std::make_unique<DLinePool>(std::vector<Line>(), m_localCoordiante, true, m_pipeline, m_sets);
    pushUI();
}

LineHandle::~LineHandle() {
    GUI::popUI("LineHandle");
}

void LineHandle::pushLines(const std::vector<std::pair<glm::vec3, glm::vec3>>& _lines) {
    std::vector<Line> lines(_lines.size());
    for (int i = 0; i < lines.size(); ++i) {
        new ((Line*)&lines[i]) Line{Point{_lines[i].first, m_color},
                                    Point{_lines[i].second, m_color}};
    }
    return m_lines->pushLines(lines);
}

Line &LineHandle::operator[](size_t _index) {
    return m_lines->getLines()[_index];
}

void LineHandle::pushLine(const glm::vec3& _begin, const glm::vec3& _end) {
    pushLine(Line{
        Point{_begin, m_color},
        Point{_end, m_color}
    });
}

void LineHandle::pushLine(const Line& _line) {
    return m_lines->pushLine(_line);
}

void LineHandle::pushUI() {
    GUI::pushUI("LineHandle", [&](){
        ImGui::Begin("LineHandleSetting");

        static char buf[32] = "default";
        ImGui::InputText("filename", buf, IM_ARRAYSIZE(buf));
        if(ImGui::Button("LoadFile"))
        {
            loadFile(std::string(buf) + ".gltf");
        }

        if(ImGui::Button("drawLine"))
        {
            drawLine();
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

int LineHandle::getLineCount() {
    return m_lines->getLineCount();
}

void LineHandle::loadFile(const std::string& _filename) {
    size_t count = 2 * m_lines->getLineCount();
    std::vector<glm::vec3> pos_vec(count);
    std::vector<glm::vec3> color_vec(count);

    for(size_t index = 0; index < m_lines->getLineCount(); index++)
    {
        pos_vec[index * 2] = m_lines->getLines()[index].begin.pos;
        color_vec[index * 2] = m_lines->getLines()[index].begin.color;
        pos_vec[index * 2 + 1] = m_lines->getLines()[index].end.pos;
        color_vec[index * 2 + 1] = m_lines->getLines()[index].end.color;
    }
    std::vector<void*> data_vec{pos_vec.data(), color_vec.data()};
    vkglTF::Model::writeGeometryListToFile(_filename,
                                           count,
                                           data_vec,
                                           std::vector<vkglTF::VertexComponent>{vkglTF::VertexComponent::Position,
                                                                                vkglTF::VertexComponent::Color}
    );
}

void LineHandle::drawLine() {
    if (appendState) {
        SingleAPP.appendListener("drawLinePool", m_listener);
        appendState = false;
    } else {
        SingleAPP.removeListener("drawLinePool");
        appendState = true;
    }
}

void LineHandle::destroy() const {
    if (!appendState){
        TaskPool::pushTask([=](){
            SingleAPP.deleteListener("drawLinePool");
        });
    } else {
        TaskPool::pushTask([=](){
            delete m_listener;
        });
    }
}

DrawLineHandle::DrawLineHandle(LineHandle* _handle):
handle(_handle)
{
    m_action[Event::MouseClick] = [&]() {
        static bool draw = false;
        static glm::vec3 dis;
        static glm::vec3 local;

        if (draw) {
            TaskPool::popUpdateTask("DrawLineHandleUpdate");
            draw = false;
        } else {
            glm::vec3& pre_pos = input::getCursor();
            dis = input::getCursor() - handle->getWorkCenter();
            local = {glm::dot(dis, handle->getTargetPlane().x_coordinate),
                               glm::dot(dis, handle->getTargetPlane().y_coordinate),
                               glm::dot(dis, handle->getTargetPlane().z_coordinate)};
            handle->pushLine(local, local);
            TaskPool::pushUpdateTask("DrawLineHandleUpdate", [&](float _abs_time){
                glm::vec3& realPos = input::getCursor();
                dis = input::getCursor() - handle->getWorkCenter();
                local = {glm::dot(dis, handle->getTargetPlane().x_coordinate),
                                   glm::dot(dis, handle->getTargetPlane().y_coordinate),
                                   glm::dot(dis, handle->getTargetPlane().z_coordinate)};
                (*handle)[handle->getLineCount()-1].end.pos = local;
            });
            draw = true;
        };
    };
}

DrawLineHandle::~DrawLineHandle() {
    TaskPool::popUpdateTask("DrawLineHandleUpdate");
    delete handle;
}

AABBLine::AABBLine(int _aabbIndex, int _captureIndex, glm::vec3 _color) : aabbIndex(_aabbIndex), captureIndex(_captureIndex){
    std::vector<glm::vec3> points;
    genLineVertexBufferFromAABB(*(*SingleAABBPool)[_aabbIndex], points);
    std::vector<Line> lines(points.size() / 2);
    for (size_t i = 0, size = lines.size(); i < size; i++) {
        new ((Point*)&lines[i].begin) Point{points[i * 2], _color};
        new ((Point*)&lines[i].end) Point{points[i * 2 + 1], _color};
    }
    line = std::make_unique<DLines>(lines, false, (*SingleAABBPool)[_aabbIndex]->m_model_index, false);
    line->init();
}

AABBLine::~AABBLine() {
    if (!Config::getConfig("RendererReleased")) {
        (*SingleDPool)[line->m_dobjs[0]]->m_model_index = -1;
        line->destroy();
    }
}

void AABBLine::show() const {
    line->show();
}

void AABBLine::hide() const {
    line->hide();
}

CaptureObjectListener::CaptureObjectListener() {
    m_action[Event::SingleClick] = [this](){
        glm::uvec2& coordinate = input::getMousePressCoordiante();

        uint32_t object_id = ((VulkanFrameBuffer*)SingleRender.getCaptureFrameBuffer())->capture(coordinate.x, coordinate.y, 0);
        input::captureObject(object_id, STATE_IN);
        if (object_id != 0 && object_id != preCaptureId) {
            preCaptureId = object_id;
            if (captureObject) captureObject->hide();
            captureObject = SingleAPP.getCaptureById(object_id);
            captureObject->drawBox();
            pushUI();
            SingleRender.normalChanged = true;
        } else {
            GUI::popUI("CapturedObject");
        }
        std::cout << "Capture Object Id: " << object_id << std::endl;
    };
}

CaptureObjectListener::~CaptureObjectListener() {
    GUI::popUI("CapturedObject");
}

void CaptureObjectListener::pushUI() {
    GUI::popUI("CapturedObject");
    GUI::pushUI("CapturedObject", [this](){
        ImGui::Begin("LineHandleSetting");

        static char buf[32] = "default";
        ImGui::Text("Captured object Name: %s", captureObject->getParent()->getName().c_str());

        ImGui::End();// End setting
    });
}
