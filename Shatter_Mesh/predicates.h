//
// Created by AnWell on 2022/2/15.
//

#ifndef PARTICLE_PREDICATES_H
#define PARTICLE_PREDICATES_H


#include "../Shatter_Item/shatter_enum.h"

class predicates {

};

GeoRelation pointInTriangle3d(const glm::vec3 & p,const glm::vec3 & t0,const glm::vec3 & t1,const glm::vec3 & t2);

GeoRelation pointInTriangle2d(const glm::vec2 & p, const glm::vec2 & t0, const glm::vec2 & t1, const glm::vec2 & t2);


GeoRelation pointInSegment3d(const glm::vec3 & p,
                                const glm::vec3 & s0,
                                const glm::vec3 & s1);

bool pointsAreColinear3d(const glm::vec3 & p0,
                            const glm::vec3 & p1,
                            const glm::vec3 & p2);

bool pointsAreColinear2d(const glm::vec2 & p0,
                         const glm::vec2 & p1,
                         const glm::vec2 & p2);

bool vecEquals3d(const glm::vec3& v0,const glm::vec3& v1);

bool vecEquals2d(const glm::vec2 & v0,const glm::vec2 & v1);

double orient2d(const glm::vec2& pa,const glm::vec2& pb,const glm::vec2& pc);

#endif //PARTICLE_PREDICATES_H
