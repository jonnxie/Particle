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

DLines::DLines(const std::vector<Line>& _lines, bool _updateFunc):updateFunc(_updateFunc){
    lines = _lines;
    id = mallocId();
    for(auto& line : lines)
    {
        (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(line.begin.pos);
        (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(line.end.pos);
    }
}

DLines::~DLines(){
    TaskPool::popUpdateTask(tool::combine("LinesBasic",id));
}

void DLines::constructG(){
    SingleBPool.createVertexHostBuffer(tool::combine("DLines",id),LineSize * lines.size(),lines.data());
    SingleBPool.getBuffer(tool::combine("DLines",id),Buffer_Type::Vertex_Host_Buffer)->map();
}

void DLines::constructD(){
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
        TaskPool::pushUpdateTask(tool::combine("LinesBasic",id),[&,ms_index,d](float _abs_time){
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + ms_index,&(*SingleDPool)[d]->m_matrix,one_matrix);
        });
    }
    SingleRender.getNObjects()->push_back(d);
    addGPUCaptureComponent((*SingleAABBPool)[m_aabbIndex]->m_min_edgy, (*SingleAABBPool)[m_aabbIndex]->m_max_edgy, d);
}

void DLines::pushLine(const Line &_line) {
    lines.push_back(_line);
    (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(_line.begin.pos);
    (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(_line.end.pos);
}

void DLines::pushLines(const std::vector<Line>& _lines) {
    lines.insert(lines.end(),_lines.begin(),_lines.end());
    for(auto& line : _lines)
    {
        (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(line.begin.pos);
        (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(line.end.pos);
    }
}

DLinePool::DLinePool(const std::vector<Line>& _lines, bool _updateFunc): updateFunc(_updateFunc){
    lines = _lines;
    id = mallocId();
    lineResolveCount = Config::getConfig("LinePoolInitialCount");
    lineCount = _lines.size();
    poolSize = lineCount * LineSize;
    init();
}

void DLinePool::constructG(){
    SingleBPool.createVertexHostBuffer(tool::combine("DLinePool",id),LineSize * lineResolveCount,lines.data());
    SingleBPool.getBuffer(tool::combine("DLinePool",id),Buffer_Type::Vertex_Host_Buffer)->map();
}

void DLinePool::constructD(){
    auto dpool = MPool<DObject>::getPool();
    auto d = dpool->malloc();
    int ms_index = ModelSetPool::getPool().malloc();
    auto buffer = SingleBPool.getBuffer(tool::combine("DLinePool",id),Buffer_Type::Vertex_Host_Buffer);

    std::vector<std::string> s_vec(1);
    s_vec[0]="Camera";
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(glm::mat4(1.0f),
                         ms_index,
                         DrawType::Vertex,
                         0,
                         tool::combine("DLinePool",id),
                         lineResolveCount * 2,
                         "",
                         0,
                         0,
                         "Polyline",
                         s_vec);
    insertDObject(d);
    if(updateFunc)
    {
        TaskPool::pushUpdateTask(tool::combine("LinePoolBasic", id),[&, ms_index, d, buffer](float _abs_time){
            memcpy(buffer->mapped, lines.data(), poolSize);
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
    lines.push_back(_line);
    poolSize += LineSize;
    if(++lineCount >= lineResolveCount)
    {
        reallocated();
    }
}

void DLinePool::pushLines(const std::vector<Line>& _lines) {
    lines.insert(lines.end(),_lines.begin(),_lines.end());
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













