//
// Created by AnWell on 2022/3/21.
//

#ifndef MAIN_WORKPLANE_H
#define MAIN_WORKPLANE_H

#include "Engine/Object/object.h"
#include <glm.hpp>
#include "Engine/Item/shatter_item.h"

class WorkPlane : public Object {
public:
    WorkPlane(const Line& _x, const Line& _y, const Line& _z, glm::vec3 _center):m_center(_center){
        m_axis[0] = _x;
        m_axis[1] = _y;
        m_axis[2] = _z;
        init();
    };
    void constructG() override;
    void constructD() override;
    ~WorkPlane() override;
private:
    /*
     * index of line3d
     */
    glm::vec3 m_center{};
    std::array<Line,3> m_axis{};
};



#endif //MAIN_WORKPLANE_H
