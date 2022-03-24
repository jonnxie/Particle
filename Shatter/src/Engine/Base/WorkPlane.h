//
// Created by AnWell on 2022/3/21.
//

#ifndef MAIN_WORKPLANE_H
#define MAIN_WORKPLANE_H

#include "Engine/Object/object.h"
#include <glm.hpp>
#include "Engine/Item/shatter_item.h"
#include ListenerCatalog

class WorkPlane : public Object {
public:
    WorkPlane(TargetPlane& _coordinate, const glm::vec3& _center):
    m_center(_center),
    m_coordinate(_coordinate)
    {
        init();
    };
    void regenerate(TargetPlane& _coordinate, const glm::vec3& _center);
    void constructG() override;
    void constructD() override;
    ~WorkPlane() override;

    ClassReferenceElement(m_center, glm::vec3, Center);
    ClassReferenceElement(m_coordinate, TargetPlane, Coordinate);
private:
    /*
     * index of line3d
     */
    std::array<Line,3> m_axis{};
};

class ChooseWorkPlane : public Shatter::Listener{
public:
    ChooseWorkPlane();
    ~ChooseWorkPlane() override;
private:
    WorkPlane* plane;
};

#endif //MAIN_WORKPLANE_H
