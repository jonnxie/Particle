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


DLines::DLines(size_t _initCount){
    lines = std::vector<Line>(_initCount);
    id = mallocId();
}

DLines::DLines(const std::vector<Line>& _lines){
    lines = _lines;
    id = mallocId();
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
    TaskPool::pushUpdateTask(tool::combine("LinesBasic",id),[&,ms_index,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index,&(*SingleDPool)[d]->m_matrix,one_matrix);
    });
//    shatter::app::ShatterApp::getApp().getNObjects()->push_back(d);
    SingleRender.getNObjects()->push_back(d);
//    SingleRender.normalChanged = true;
}

void DLines::pushLine(const Line &_line) {
    lines.push_back(_line);
}

void DLines::pushLines(const std::vector<Line>& _lines) {
    lines.insert(lines.end(),_lines.begin(),_lines.end());
}















