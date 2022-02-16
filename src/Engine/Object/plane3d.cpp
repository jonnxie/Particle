//
// Created by AnWell on 2021/6/28.
//

#include "plane3d.h"

Plane3d::Plane3d():m_normal(0.0f,1.0f,0.0f) {
    recalculateD(glm::vec3(0.0f,0.0f,0.0f));
}

Plane3d::Plane3d(glm::vec3 &_point, const glm::vec3 &_normal) :
        m_normal(_normal)
{
    recalculateD(_point);
}

Plane3d::Plane3d(float _px, float _py, float _pz, float _nx, float _ny, float _nz):
        m_normal(glm::vec3(_nx,_ny,_nz))
{
    recalculateD(glm::vec3(_px,_py,_pz));
}

Plane3d::Plane3d(glm::vec3 &_point1, const glm::vec3 &_point2, const glm::vec3 &_point3) {
    setPlane(_point1,_point2,_point3);
}

Plane3d::Plane3d(glm::vec3 &_normal, const float &_d):m_normal(_normal),m_distance(_d) {

}

float Plane3d::getDistanceTo(const glm::vec3 &_point) const {
    return glm::dot(_point,m_normal) + m_distance;
}

bool Plane3d::isFrontFacing(const glm::vec3 & _direction) const {
    return glm::dot(m_normal,_direction) < 0;
}

bool Plane3d::getIntersectionWithPlane(const Plane3d &_other, glm::vec3 &_outLinePoint, glm::vec3 &_outLineVect) const {
    float fn00 = glm::length(m_normal);
    float fn01 = glm::dot(m_normal,_other.m_normal);
    float fn11 = glm::length(_other.m_normal);
    float det = fn00 * fn11 - fn01*fn01;

    /*
     * 此时两平面平行   det 等同于在两法线夹角cos值为1时
     */
    if(glm::abs(det) < float_limit)
        return false;

    float invdet = 1.0f/det;
    float fc0 = (fn11 * -m_distance + fn01 * _other.m_distance) * invdet;
    float fc1 = (fn00 * -_other.m_distance + fn01*m_distance) * invdet;


    _outLineVect = glm::cross(m_normal,_other.m_normal);
    _outLinePoint = m_normal * fc0 + _other.m_normal * fc1;
    return true;
}

bool Plane3d::existsIntersection(const Plane3d & _other) const {
    glm::vec3 cross = glm::cross(m_normal,_other.m_normal);
    /*
     * 只有平行时sin值为0就会为0，其它情况均相交
     */
    return glm::length(cross) > float_limit;
}

glm::vec3 Plane3d::getMemberPoint() const {
    return m_normal * -m_distance;
}

void Plane3d::recalculateD(const glm::vec3 & _memberPoint) {
    m_distance = - glm::dot(m_normal,_memberPoint);
}

bool Plane3d::getIntersectionWithLimitedLine(const glm::vec3 & _linePoint1, const glm::vec3 &_linePoint2, glm::vec3 & _outIntersection) const {
    return (getIntersectionWithLine(_linePoint1,_linePoint2-_linePoint1,_outIntersection) &&
            glm::abs(
                    glm::distance(_linePoint1,_outIntersection) + glm::distance(_linePoint2,_outIntersection) -
                    glm::distance(_linePoint2,_linePoint1)
                    ) <= float_limit);
}


float Plane3d::getKnownIntersectionWithLine(const glm::vec3 & _linePoint1, const glm::vec3 & _linePoint2) const {
    glm::vec3 vect = _linePoint2 - _linePoint1;
    float t2 = glm::dot(m_normal,vect);
    return -((glm::dot(m_normal,_linePoint1) + m_distance) / t2);
}

bool Plane3d::getIntersectionWithLine(const glm::vec3 &_linePoint, const glm::vec3 & _lineVect, glm::vec3 &_outIntersection) const {
    float t2 = glm::dot(m_normal,_lineVect);
    /*
     * 此时两线平行
     */
    if (glm::abs(t2) < float_limit){
        return false;
    }

    /*
     * t2为cos值，-(glm::dot(m_normal,_linePoint) + m_distance)为垂直距离
     */
    float t = -(glm::dot(m_normal,_linePoint) + m_distance) / t2;
    _outIntersection = _linePoint + (_lineVect * t);
    return true;
}

void Plane3d::setPlane(const glm::vec3 & _point1, const glm::vec3 &_point2, const glm::vec3 &_point3) {
    m_normal = glm::cross(_point2-_point1,_point3 - _point1);
    glm::normalize(m_normal);

    recalculateD(_point1);
}

void Plane3d::setPlane(const glm::vec3 & _nvec, float _d) {
    m_normal = _nvec;
    m_distance = _d;
}

void Plane3d::setPlane(const glm::vec3 & _point, const glm::vec3 & _nvector) {
    m_normal = _nvector;
    recalculateD(_point);
}

bool Plane3d::operator==(const Plane3d & _other) const {
    return _other.m_normal == m_normal && _other.m_distance == m_distance;
}

bool Plane3d::operator!=(const Plane3d & _other) const {
    return !(*this == _other);;
}

Relation3D Plane3d::classifyPointRelation(const glm::vec3 &point) const {
    const float  d = glm::dot(point,m_normal)+m_distance;

    if (d < 0.0f)
        return Relation3D::Back;

    if (d > 0.0f)
        return Relation3D::Front ;

    return Relation3D::Planar;
}
