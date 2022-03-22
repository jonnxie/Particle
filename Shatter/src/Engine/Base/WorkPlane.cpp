//
// Created by AnWell on 2022/3/21.
//

#include "WorkPlane.h"

void WorkPlane::constructG() {
    SingleBPool.createVertexHostBuffer("WorkPlane", TargetPlaneDoubleCoordinateSize, m_axis.data());
    SingleBPool.getBuffer("WorkPlane",Buffer_Type::Vertex_Host_Buffer)->map();
    for(auto& line : m_axis)
    {
        (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(line.begin.pos);
        (*SingleAABBPool)[m_aabbIndex]->addInternalPoint(line.end.pos);
    }
}

void WorkPlane::constructD() {
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
                         "WorkPlane",
                         6,
                         "",
                         0,
                         0,
                         "Polyline",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask("WorkPlane", [&,ms_index,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + ms_index,&(*SingleDPool)[d]->m_matrix,one_matrix);
    });
    SingleRender.getNObjects()->push_back(d);
    addGPUCaptureComponent((*SingleAABBPool)[m_aabbIndex]->m_min_edgy, (*SingleAABBPool)[m_aabbIndex]->m_max_edgy, d);
}

WorkPlane::~WorkPlane()
{
    SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Normal);
    TaskPool::popUpdateTask("WorkPlane");
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer("WorkPlane", Buffer_Type::Vertex_Host_Buffer);
    SingleRender.releaseObject(int(m_capture_id),DrawObjectType::AABB);
    SingleRender.normalChanged = true;
}
