//
// Created by AnWell on 2022/2/8.
//

#ifndef CAD_MESH_H
#define CAD_MESH_H

#include "Engine/Item/shatter_enum.h"
#include PrecompiledCatalog"Engine/Object/precompiled.h"
#include AABBCatalog"Engine/Object/aabb.h"

class SpatialItem
{
public:
    explicit SpatialItem() {}
    virtual ~SpatialItem() {}
    size_t      id{};
    AABB        aabb{};
    SpatialType item_type = SpatialType::Abstract;

    // given a point in space P, finds the point in the item that is closest to P
    virtual glm::vec3 pointClosestTo(const glm::vec3 & p) const = 0;

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    virtual void barycentricCoordinates(const glm::vec3 & p, double bc[]) const = 0;

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    virtual void draw() = 0;

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    double dist     (const glm::vec3 & p) const;
    double distSqrd(const glm::vec3 & p) const;

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    virtual bool contains           (const glm::vec3 & p, const bool strict) const = 0;
    virtual bool intersectsTriangle(const glm::vec3 t[], const bool ignore_if_valid_complex) const = 0;
    virtual bool intersectsRay     (const glm::vec3 & p, const glm::vec3 & dir, double & t, glm::vec3 & pos) const = 0;
};




#endif //CAD_MESH_H
