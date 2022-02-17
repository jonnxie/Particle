//
// Created by maybe on 2021/5/26.
//
#include "precompiledhead.h"

#include "aabb.h"
#include "line3d.h"

bool AABB::intersectsWithLine(const Line3d& _line) const{
    return intersectsWithLine(_line.getMiddle(),glm::normalize(_line.getVector()),_line.getLength() * 0.5f);
}

glm::vec3 AABB::getCenter() const {
    return (m_min_edgy + m_max_edgy) / 2.0f;
}

glm::vec3 AABB::getExtent() const {
    return m_max_edgy - m_min_edgy;
}

float AABB::getVolume() const {
    const glm::vec3 e = getExtent();
    return e.x*e.y*e.z;
}

float AABB::getArea() const {
    const glm::vec3 e = getExtent();
    return 2.0f*(e.x*e.y + e.x*e.z + e.y*e.z);
}

void AABB::repair() {
    if(m_min_edgy.x > m_max_edgy.x){
//        swap<float>(m_min_edgy.x,m_max_edgy.x);
    }
    if(m_min_edgy.y > m_max_edgy.y){
//        swap<float>(m_min_edgy.y,m_max_edgy.y);
    }
    if(m_min_edgy.z > m_max_edgy.z){
//        swap<float>(m_min_edgy.z,m_max_edgy.z);
    }
}

bool AABB::isPointInside(const glm::vec3 &_p) const {
    return (_p.x >= m_min_edgy.x && _p.x <= m_max_edgy.x &&
            _p.y >= m_min_edgy.y && _p.y <= m_max_edgy.y &&
            _p.z >= m_min_edgy.z && _p.z <= m_max_edgy.z);
}

bool AABB::isPointFloatTotalInside(const glm::vec3 &_p) const {
    return (_p.x > m_min_edgy.x && _p.x < m_max_edgy.x &&
            _p.y > m_min_edgy.y && _p.y < m_max_edgy.y &&
            _p.z > m_min_edgy.z && _p.z < m_max_edgy.z);
}

bool AABB::isFullInside(const AABB &_other) {
    return (isPointInside(_other.m_min_edgy) && isPointInside(_other.m_max_edgy));
}

bool AABB::intersectsWithBox(const AABB &_other) {
    return (m_min_edgy.x <=_other.m_max_edgy.x && m_min_edgy.y <=_other.m_max_edgy.y && m_min_edgy.z <=_other.m_max_edgy.z &&
            m_max_edgy.x >=_other.m_min_edgy.x && m_max_edgy.y >=_other.m_min_edgy.y && m_max_edgy.z >=_other.m_min_edgy.z);
}

bool AABB::intersectsWithLine(const glm::vec3 &_line_middle, const glm::vec3 &_line_vect, float _half_length) const {
    const glm::vec3 e = getExtent() * 0.5f;
    const glm::vec3 t = getCenter() - _line_middle;

    if ((fabs(t.x) > e.x + _half_length * fabs(_line_vect.x)) ||
        (fabs(t.y) > e.y + _half_length * fabs(_line_vect.y)) ||
        (fabs(t.z) > e.z + _half_length * fabs(_line_vect.z)) )
        return false;

    float r = e.y * (float)fabs(_line_vect.z) + e.z * (float)fabs(_line_vect.y);
    if (fabs(t.y*_line_vect.z - t.z*_line_vect.y) > r )
        return false;

    r = e.x * (float)fabs(_line_vect.z) + e.z * (float)fabs(_line_vect.x);
    if (fabs(t.z*_line_vect.x - t.x*_line_vect.z) > r )
        return false;

    r = e.x * (float)fabs(_line_vect.y) + e.y * (float)fabs(_line_vect.x);
    if (fabs(t.x*_line_vect.y - t.y*_line_vect.x) > r)
        return false;

    return true;
}

void AABB::addInternalPoint(float _x, float _y, float _z) {
    if(_x > m_max_edgy.x){m_max_edgy.x = _x;};
    if(_y > m_max_edgy.y){m_max_edgy.y = _y;};
    if(_z > m_max_edgy.z){m_max_edgy.z = _z;};

    if(_x < m_min_edgy.x){m_min_edgy.x = _x;};
    if(_y < m_min_edgy.y){m_min_edgy.y = _y;};
    if(_z < m_min_edgy.z){m_min_edgy.z = _z;};
}

void AABB::addInternalPoint(const glm::vec3 &_other) {
    addInternalPoint(_other.x,_other.y,_other.z);
}

