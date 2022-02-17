//
// Created by AnWell on 2022/2/8.
//
#include "precompiledhead.h"

#include "mesh.h"

double SpatialItem::dist(const glm::vec3 &p) const
{
    return glm::distance(p,pointClosestTo(p));
}

double SpatialItem::distSqrd(const glm::vec3 &p) const
{
    return glm::distance(p,pointClosestTo(p)) * glm::distance(p,pointClosestTo(p));
}

