//
// Created by maybe on 2021/7/9.
//
#include "precompiledhead.h"

#include "gcoor.h"
#include "Engine/Object/line3d.h"
#include "Engine/Object/dobject.h"
#include "Engine/Object/device.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Pool/mpool.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Renderer/shatter_render_include.h"
#include <string>

GCoor* GCoor::createGCoor(const std::vector<glm::vec3>& _in)
{
	return new GCoor(_in);
}

void GCoor::constructG() {
    auto gpool = MPool<GObject>::getPool();
    int g = gpool->malloc();

    Point_PC vertex[6];
    (*gpool)[g]->m_vertex = vertex;
    (*gpool)[g]->m_vertex_size = sizeof(Point_PC) * 6;
    {
        vertex[0].pos = lines[0];
        vertex[0].color = RED_COLOR;
        vertex[1].pos = lines[1];
        vertex[1].color = RED_COLOR;

        vertex[2].pos = lines[2];
        vertex[2].color = BLUE_COLOR;
        vertex[3].pos = lines[3];
        vertex[3].color = BLUE_COLOR;

        vertex[4].pos = lines[4];
        vertex[4].color = GOLD_COLOR;
        vertex[5].pos = lines[5];
        vertex[5].color = GOLD_COLOR;
    }
    (*gpool)[g]->genVBuffer("coordinate");
    insertGObject(g);

    auto aabbPool = MPool<AABB>::getPool();
    for(auto & i : vertex)
    {
        (*aabbPool)[m_boxIndex]->addInternalPoint(i.pos);
    }
}

void GCoor::constructD() {
    auto dpool = MPool<DObject>::getPool();
    int d = dpool->malloc();

    std::vector<std::string> s_vec{std::string("Camera")};
    auto modelIndex = ModelSetPool::getPool().malloc();
    (*dpool)[d]->m_type = DType::Normal;

    (*dpool)[d]->prepare(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)),
                         modelIndex,
                         DrawType::Vertex,
                         0,
                         "coordinate",
                         6,
                         "",
                         0,
                         0,
                         "Polyline",
                         s_vec);
    insertDObject(d);
    TaskPool::pushUpdateTask("GCoor",[&,modelIndex,d](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + modelIndex, &(*SingleDPool)[d]->m_matrix, one_matrix);
    });
    SingleRender.pushNObjects(d);
    setCapture(std::make_shared<CaptureObject>(this, m_boxIndex, d, modelIndex, "Coordinate"));
    SingleAPP.capturedPush(getCapture());
}

GCoor::GCoor(const std::vector<glm::vec3>& _in):
        lines(_in)
{
    m_boxIndex = MPool<AABB>::getPool()->malloc();
    init();
};



