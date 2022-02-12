//
// Created by jonnxie on 2022/1/28.
//

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

Tris::Tris(size_t _initCount,MeshDrawType _type):type(_type){
    tris = std::vector<Tri>(_initCount);
    id = mallocTriId();
}

Tris::Tris(const std::vector<Tri>& _tris,MeshDrawType _type):type(_type){
    tris = _tris;
    id = mallocTriId();
}

void Tris::constructG(){
    SingleBPool.createVertexHostBuffer(tool::combine("Tris",id),TriSize * tris.size(),tris.data());
    SingleBPool.getBuffer(tool::combine("Tris",id),Buffer_Type::Vertex_Host_Buffer)->map();
}

void Tris::constructD(){
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
                         tool::combine("Tris",id),
                         tris.size() * 3,
                         "",
                         0,
                         0,
                         type == MeshDrawType::Face ? "TriangleFace":"TriangleLine",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask(tool::combine("TrisBasic",id),[&,ms_index,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index,&(*SingleDPool)[d]->m_matrix,one_matrix);
    });
//    shatter::app::ShatterApp::getApp().getNObjects()->push_back(d);
    SingleRender.getNObjects()->push_back(d);
}

void Tris::pushTri(const Tri &_tri) {
    tris.push_back(_tri);
}

void Tris::pushTris(const std::vector<Tri>& _tris) {
    tris.insert(tris.end(),tris.begin(),tris.end());
}

