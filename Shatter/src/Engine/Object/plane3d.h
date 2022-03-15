//
// Created by AnWell on 2021/6/28.
//

#ifndef SHATTER_ENGINE_PLANE3D_H
#define SHATTER_ENGINE_PLANE3D_H

#include <glm.hpp>
#include "Engine/Item/shatter_enum.h"

class Plane3d {
public:
    Plane3d();

    Plane3d (glm::vec3&, const glm::vec3&);

    Plane3d(float, float, float, float, float, float);

    Plane3d(glm::vec3&, const glm::vec3&, const glm::vec3& );

    Plane3d(glm::vec3&, const float&);

    ~Plane3d() = default;

    inline bool operator==(const Plane3d&) const;

    inline bool operator!=(const Plane3d&) const;

    void setPlane(const glm::vec3&, const glm::vec3&);

    void setPlane(const glm::vec3& , float);

    void setPlane(const glm::vec3& , const glm::vec3& , const glm::vec3& );

    bool getIntersectionWithLine(const glm::vec3& ,
                                 const glm::vec3& ,
                                 glm::vec3& ) const;

    /*
     * 返回p1 p2两点构成线段被平面截取时 p1到平面的交点的长度占整段线段长度的百分比
     */
    float getKnownIntersectionWithLine(const glm::vec3& ,
                                     const glm::vec3& ) const;

    bool getIntersectionWithLimitedLine(
            const glm::vec3&,
            const glm::vec3&,
            glm::vec3&) const;

    Relation3D classifyPointRelation(const glm::vec3& point) const;

    void recalculateD(const glm::vec3&);

    glm::vec3 getMemberPoint() const;

    bool existsIntersection(const Plane3d&) const;

    bool getIntersectionWithPlane(const Plane3d&,
                                  glm::vec3&,
                                  glm::vec3&) const;

    bool isFrontFacing(const glm::vec3&) const;

    float  getDistanceTo(const glm::vec3&) const;


    glm::vec3 m_normal;
    /*
     * distance from origin to plane
     */
    float m_distance;
};


#endif //SHATTER_ENGINE_PLANE3D_H
