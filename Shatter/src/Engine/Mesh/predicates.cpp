//
// Created by AnWell on 2022/2/15.
//
#include "precompiledhead.h"

#include "predicates.h"


GeoRelation pointInTriangle3d(const glm::vec3 & p,const glm::vec3 & t0,const glm::vec3 & t1,const glm::vec3 & t2)
{
    // test for point in vert
    if(vecEquals3d(p, t0)) return GeoRelation::ON_VERT0;
    if(vecEquals3d(p, t1)) return GeoRelation::ON_VERT1;
    if(vecEquals3d(p, t2)) return GeoRelation::ON_VERT2;

    // test for point in edge in 3D
    if(pointInSegment3d(p, t0, t1) == GeoRelation::STRICTLY_INSIDE) return GeoRelation::ON_EDGE0;
    if(pointInSegment3d(p, t1, t2) == GeoRelation::STRICTLY_INSIDE) return GeoRelation::ON_EDGE1;
    if(pointInSegment3d(p, t2, t0) == GeoRelation::STRICTLY_INSIDE) return GeoRelation::ON_EDGE2;

    // test for the interior: project t on XYZ and, if the check is never false in
    // any of the projections, then p must be inside t

    glm::vec2 p_dropX = { p[1],  p[2]};
    glm::vec2 t0_dropX = {t0[1], t0[2]}, t1_dropX = {t1[1], t1[2]}, t2_dropX = {t2[1], t2[2]};

    if(pointInTriangle2d(p_dropX, t0_dropX, t1_dropX, t2_dropX) == GeoRelation::STRICTLY_OUTSIDE) return GeoRelation::STRICTLY_OUTSIDE;

    glm::vec2 p_dropY = { p[0],  p[2]};
    glm::vec2 t0_dropY = {t0[0], t0[2]}, t1_dropY = {t1[0], t1[2]}, t2_dropY = {t2[0], t2[2]};

    if(pointInTriangle2d(p_dropY, t0_dropY, t1_dropY, t2_dropY) == GeoRelation::STRICTLY_OUTSIDE) return GeoRelation::STRICTLY_OUTSIDE;

    glm::vec2 p_dropZ  = { p[0],  p[1]};
    glm::vec2 t0_dropZ = {t0[0], t0[1]}, t1_dropZ = {t1[0], t1[1]}, t2_dropZ = {t2[0], t2[1]};

    if(pointInTriangle2d(p_dropZ, t0_dropZ, t1_dropZ, t2_dropZ) == GeoRelation::STRICTLY_OUTSIDE) return GeoRelation::STRICTLY_OUTSIDE;

    return GeoRelation::STRICTLY_INSIDE;
}

GeoRelation pointInTriangle2d(const glm::vec2 & p, const glm::vec2 & t0, const glm::vec2 & t1, const glm::vec2 & t2)
{
    if(vecEquals2d(p, t0)) return GeoRelation::ON_VERT0;
    if(vecEquals2d(p, t1)) return GeoRelation::ON_VERT1;
    if(vecEquals2d(p, t2)) return GeoRelation::ON_VERT2;

    double e0p_area = orient2d(t0, t1, p);
    double e1p_area = orient2d(t1, t2, p);
    double e2p_area = orient2d(t2, t0, p);

    bool hit = (e0p_area >= 0 && e1p_area >= 0 && e2p_area >= 0) ||
               (e0p_area <= 0 && e1p_area <= 0 && e2p_area <= 0);

    if(hit)
    {
        if(e0p_area == 0) return GeoRelation::ON_EDGE0;
        if(e1p_area == 0) return GeoRelation::ON_EDGE1;
        if(e2p_area == 0) return GeoRelation::ON_EDGE2;

        return GeoRelation::STRICTLY_INSIDE;
    }

    return GeoRelation::STRICTLY_OUTSIDE;
}

bool vecEquals3d(const glm::vec3 &v0, const glm::vec3 &v1) {
    return ((v0[0] == v1[0]) &&
            (v0[1] == v1[1]) &&
            (v0[2] == v1[2]));
}

bool vecEquals2d(const glm::vec2 & v0,const glm::vec2 & v1){
    return ((v0[0] == v1[0]) &&
            (v0[1] == v1[1]));
}

GeoRelation pointInSegment3d(const glm::vec3 &p, const glm::vec3 &s0, const glm::vec3 &s1) {
    if(vecEquals3d(p, s0)) return GeoRelation::ON_VERT0;
    if(vecEquals3d(p, s1)) return GeoRelation::ON_VERT1;

    if(!pointsAreColinear3d(s0, s1, p)) return GeoRelation::STRICTLY_OUTSIDE;

    if((p[0] > std::min(s0[0], s1[0]) && p[0] < std::max(s0[0], s1[0])) ||
       (p[1] > std::min(s0[1], s1[1]) && p[1] < std::max(s0[1], s1[1])) ||
       (p[2] > std::min(s0[2], s1[2]) && p[2] < std::max(s0[2], s1[2])))
    {
        return GeoRelation::STRICTLY_INSIDE;
    }

    return GeoRelation::STRICTLY_OUTSIDE;
}

bool pointsAreColinear3d(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) {
    glm::vec2 p0_dropX = { p0[1], p0[2] };
    glm::vec2 p0_dropY = { p0[0], p0[2] };
    glm::vec2 p0_dropZ = { p0[0], p0[1] };
    glm::vec2 p1_dropX = { p1[1], p1[2] };
    glm::vec2 p1_dropY = { p1[0], p1[2] };
    glm::vec2 p1_dropZ = { p1[0], p1[1] };
    glm::vec2 p2_dropX = { p2[1], p2[2] };
    glm::vec2 p2_dropY = { p2[0], p2[2] };
    glm::vec2 p2_dropZ = { p2[0], p2[1] };

    // check if all the 2d orthogonal projections of p are colinear
    if(pointsAreColinear2d(p0_dropX, p1_dropX, p2_dropX) &&
       pointsAreColinear2d(p0_dropY, p1_dropY, p2_dropY) &&
       pointsAreColinear2d(p0_dropZ, p1_dropZ, p2_dropZ))
    {
        return true;
    }
    return false;
}

bool pointsAreColinear2d(const glm::vec2 &p0, const glm::vec2 &p1, const glm::vec2 &p2) {
    return (orient2d(p0,p1,p2)==0);
}

double orient2d(const glm::vec2 &pa, const glm::vec2 &pb, const glm::vec2 &pc) {
    float acx = pa[0] - pc[0];
    float bcx = pb[0] - pc[0];
    float acy = pa[1] - pc[1];
    float bcy = pb[1] - pc[1];

    return acx * bcy - acy * bcx;
}
