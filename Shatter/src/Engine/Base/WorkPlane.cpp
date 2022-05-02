//
// Created by AnWell on 2022/3/21.
//
#include "precompiledhead.h"
#include "WorkPlane.h"
#include "Engine/Object/inputaction.h"
#include "Engine/App/shatterapp.h"
#include "Engine/Object/camera.h"
#include CaptureCatalog


void WorkPlane::regenerate(TargetPlane& _coordinate, const glm::vec3& _center){
    m_coordinate = _coordinate;
    m_center = _center;
    new ((Line*)&m_axis[0]) Line(makeLine(m_coordinate.x_coordinate, m_center));
    new ((Line*)&m_axis[1]) Line(makeLine(m_coordinate.y_coordinate, m_center));
    new ((Line*)&m_axis[2]) Line(makeLine(m_coordinate.z_coordinate, m_center));
}

void WorkPlane::constructG() {
    new ((Line*)&m_axis[0]) Line(makeLine(m_coordinate.x_coordinate, m_center));
    new ((Line*)&m_axis[1]) Line(makeLine(m_coordinate.y_coordinate, m_center));
    new ((Line*)&m_axis[2]) Line(makeLine(m_coordinate.z_coordinate, m_center));

    SingleBPool.createVertexHostBuffer("WorkPlane", TargetPlaneDoubleCoordinateSize, m_axis.data());
    SingleBPool.getBuffer("WorkPlane",Buffer_Type::Vertex_Host_Buffer)->map();
    for(auto& line : m_axis)
    {
        (*SingleBoxPool)[m_boxIndex]->addInternalPoint(line.begin.pos);
        (*SingleBoxPool)[m_boxIndex]->addInternalPoint(line.end.pos);
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
        memcpy(ptr + ms_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
        memcpy(SingleBPool.getBuffer("WorkPlane",Buffer_Type::Vertex_Host_Buffer)->mapped, m_axis.data(), TargetPlaneDoubleCoordinateSize);
    });
    SingleRender.pushNObjects(d);
    m_captureObject = std::make_shared<CaptureObject>(this, m_boxIndex, d, "WorkPlane");
    SingleAPP.capturedPush(m_captureObject);
}

WorkPlane::~WorkPlane()
{
    SingleRender.releaseObject(m_dobjs[0], DrawObjectType::Normal);
    TaskPool::popUpdateTask("WorkPlane");
    vkQueueWaitIdle(SingleRender.graphics_queue);
    SingleBPool.freeBuffer("WorkPlane", Buffer_Type::Vertex_Host_Buffer);
    SingleRender.releaseObject(int(m_capture_id), DrawObjectType::AABB);
    SingleRender.normalChanged = true;
}

WorkPlane::WorkPlane(TargetPlane &_coordinate, const glm::vec3 &_center):
        m_center(_center),
        m_coordinate(_coordinate)
{
    m_boxIndex = MPool<AABB>::getPool()->malloc();
    init();
}

ChooseWorkPlane::ChooseWorkPlane() {
    m_action[Event::MouseClick] = [&]() {
        static bool draw_work_plane = false;
        static glm::vec2 pre_pos;
        static glm::vec2 realPos;
        static TargetPlane tar_plane;
        if(draw_work_plane)
        {
            TaskPool::popUpdateTask("ChooseWorkPlaneUpdate");
            draw_work_plane = false;
        } else {
            plane = SingleAPP.generateWorkPlane(SingleAPP.getWorkTargetPlane(), input::getCursor());
            SingleAPP.setWork(true);
            TaskPool::pushUpdateTask("ChooseWorkPlaneUpdate", [&](float _abs_time){
                tar_plane.z_coordinate = glm::normalize(SingleCamera.center + SingleCamera.eye - plane->getCenter());
                tar_plane.x_coordinate = glm::normalize(input::getCursor() - plane->getCenter());
                tar_plane.y_coordinate = glm::normalize(glm::cross(tar_plane.z_coordinate, tar_plane.x_coordinate));
                tar_plane.x_coordinate = glm::normalize(glm::cross(tar_plane.y_coordinate, tar_plane.z_coordinate));

                plane->regenerate(tar_plane, plane->getCenter());
            });
            SingleRender.normalChanged = true;
            draw_work_plane = true;
        };
    };
}

ChooseWorkPlane::~ChooseWorkPlane() {
    TaskPool::popUpdateTask("ChooseWorkPlaneUpdate");
}
