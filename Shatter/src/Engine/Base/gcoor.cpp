//
// Created by maybe on 2021/7/9.
//

#include "gcoor.h"
#include "Engine/Object/line3d.h"
#include "Engine/Object/dobject.h"
#include "Engine/Object/device.h"
#include "Engine/Object/mpool.h"
#include "Engine/Object/setpool.h"
#include "Engine/Object/modelsetpool.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Object/bpool.h"
#include "Engine/Buffer/shatterbufferinclude.h"
#include <string>

GCoor::GCoor(const std::vector<int>& _in):m_x_axis(_in[0]),m_y_axis(_in[1]),m_z_axis(_in[2]){
}


GCoor *GCoor::createGCoor(int _x, int _y, int _z) {
    auto val = new GCoor(_x,_y,_z);
    val->init();
    return val;
}

GCoor *GCoor::createGCoor(const std::vector<int>& _in) {
    auto val = new GCoor(_in);
    val->init();
    return val;
}

void GCoor::constructG() {
    auto gpool = MPool<GObject>::getPool();
    int g = gpool->malloc();

    auto line_pool = MPool<Line3d>::getPool();
    Point_PC vertex[6];
    (*gpool)[g]->m_vertex = vertex;
    (*gpool)[g]->m_vertex_size = sizeof(Point_PC) * 6;
    {
        auto line = (*line_pool)[m_x_axis];
        vertex[0].pos = line->m_begin;
        vertex[0].color = RED_COLOR;
        vertex[1].pos = line->m_end;
        vertex[1].color = RED_COLOR;

        line = (*line_pool)[m_y_axis];
        vertex[2].pos = line->m_begin;
        vertex[2].color = BLUE_COLOR;
        vertex[3].pos = line->m_end;
        vertex[3].color = BLUE_COLOR;

        line = (*line_pool)[m_z_axis];
        vertex[4].pos = line->m_begin;
        vertex[4].color = GOLD_COLOR;
        vertex[5].pos = line->m_end;
        vertex[5].color = GOLD_COLOR;
    }
    (*gpool)[g]->genVBuffer("coordinate");
    insertGObject(g);

    for(auto & i : vertex)
    {
//        printPoint(i.pos);
        m_aabb.addInternalPoint(i.pos);
    }
}

void GCoor::constructD() {
    auto dpool = MPool<DObject>::getPool();
    int d = dpool->malloc();

    std::vector<std::string> s_vec{std::string("Camera")};
//    glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
    (*dpool)[d]->m_type = DType::Normal;
    (*dpool)[d]->prepare(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)),
                          ModelSetPool::getPool().malloc(),
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
}




