//
// Created by AnWell on 2022/2/15.
//

#ifndef PARTICLE_POINT_H
#define PARTICLE_POINT_H

#include "Engine/Item/shatter_enum.h"
#include "mesh.h"
#include PrecompiledCatalog"Engine/Object/precompiled.h"

class DPoints;


class GPoint :public SpatialItem{
public:
    GPoint(const size_t id, const glm::vec3 & v)
    {
        this->v   = v;
        this->id  = id;
        item_type = SpatialType::Point;
        aabb.addInternalPoint(v);
    }

    ~GPoint() {}

    // Implement SpatialDataStructureItem interface ::::::::::::::::::::::::::

    glm::vec3    pointClosestTo(const glm::vec3 & p) const override;
    bool     intersectsRay(const glm::vec3 & p, const glm::vec3 & dir, double & t, glm::vec3 & pos) const override;
    void     barycentricCoordinates(const glm::vec3 & p, double bc[]) const override;
    bool     contains(const glm::vec3 & p, const bool strict) const override;
    bool     intersectsTriangle(const glm::vec3   t[], const bool ignore_if_valid_complex) const override;
    void     draw() override;
    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    glm::vec3 v;
    std::unique_ptr<DPoints> point{nullptr};
};


#endif //PARTICLE_POINT_H
