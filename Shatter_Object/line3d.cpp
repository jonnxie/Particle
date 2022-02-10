//
// Created by maybe on 2021/6/2.
//

#include "line3d.h"

void Line3d::setLine(const float& _xbegin, const float& _ybegin, const float& _zbegin, const float& _xend, const float& _yend, const float& _zend)
{
    m_begin = glm::vec3(_xbegin, _ybegin, _zbegin) ;
    m_end = glm::vec3(_xend, _yend, _zend);
}

void Line3d::setLine(const glm::vec3& _begin, const glm::vec3& _end)
{
    m_begin = _begin;
    m_end = _end;
}

void Line3d::setLine(const Line3d& line){
    m_begin = line.m_begin;
    m_end = line.m_end;
}

bool Line3d::isPointBetweenStartAndEnd(const glm::vec3& _point) const
{
    return (glm::abs(
            - glm::distance(m_end,m_begin) +
            (glm::distance(_point,m_begin) + glm::distance(_point,m_end))
    ) < float_limit);
}


glm::vec3 Line3d::getClosestPoint(const glm::vec3& _point) const
{
    glm::vec3 c = _point - m_begin;
    glm::vec3 v = m_end - m_begin;
    float d = glm::length(v);
    v /= d;
    float t = glm::dot(c,v);

    if (t < (float)0.0)
        return m_begin;
    if (t > d)
        return m_end;

    v *= t;
    return m_begin + v;
}

glm::vec3 Line3d::getMiddle()const{
    return (m_begin + m_end) / 2.0f;
}

glm::vec3 Line3d::getVector()const{
    return m_end - m_begin;
}

float Line3d::getLength()const{
    return glm::length(m_end - m_begin);
}
