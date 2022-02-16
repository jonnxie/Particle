//
// Created by AnWell on 2022/2/15.
//

#include "Point.h"
#include "predicates.h"
#include DPointCatalog"Engine/Base/points.h"

glm::vec3 GPoint::pointClosestTo(const glm::vec3 &p) const {
    return v;
}

bool GPoint::intersectsRay(const glm::vec3 &p, const glm::vec3 &dir, double &t, glm::vec3 &pos) const {
    assert(false && "TODO");
    return true;
}

void GPoint::barycentricCoordinates(const glm::vec3 &p, double *bc) const {
    assert(false && "NONSENSE");
}

bool GPoint::contains(const glm::vec3 &p, const bool strict) const {
    return glm::distance(p,v) <= float_limit;
}

bool GPoint::intersectsTriangle(const glm::vec3 *t, const bool ignore_if_valid_complex) const {
    auto res = pointInTriangle3d(v, t[0], t[1], t[2]);
    if(ignore_if_valid_complex) return (res==GeoRelation::STRICTLY_INSIDE || res>=GeoRelation::ON_EDGE0);
    return (res!=GeoRelation::STRICTLY_OUTSIDE);
}

void GPoint::draw() {
    std::vector<Point3dColorSize> p{
            {
                    v,
                    RED_COLOR
            }
    };

    point = std::make_unique<DPoints>(p);
    point->init();
}


