//
// Created by maybe on 2021/6/2.
//

#ifndef SHAfloatfloatER_ENGINE_LINE_H
#define SHAfloatfloatER_ENGINE_LINE_H

#include <glm.hpp>
#include "gobject.h"

class Line3d  {
public:
    Line3d() : m_begin(0,0,0), m_end(1,1,1) {}
    Line3d(float _xbegin, float _ybegin, float _zbegin, float _xend, float _yend, float _zend) : m_begin(_xbegin, _ybegin, _zbegin),m_end(_xend, _yend, _zend) {}
    Line3d(const glm::vec3& _begin, const glm::vec3& _end) : m_begin(_begin), m_end(_end) {}
    ~Line3d() = default;

    Line3d operator+(const glm::vec3& _point) const { return Line3d(m_begin + _point, m_end + _point); }
    Line3d& operator+=(const glm::vec3& _point) { m_begin += _point; m_end += _point; return *this; }

    Line3d operator-(const glm::vec3& _point) const { return Line3d(m_begin - _point, m_end - _point); }
    Line3d& operator-=(const glm::vec3& _point) { m_begin -= _point; m_end -= _point; return *this; }

    bool operator==(const Line3d& _other) const
    { return (m_begin == _other.m_begin && m_end == _other.m_end) || (m_end == _other.m_begin && m_begin == _other.m_end);}
    bool operator!=(const Line3d& _other) const
    { return !(m_begin == _other.m_begin && m_end == _other.m_end) || (m_end == _other.m_begin && m_begin == _other.m_end);}

    void setLine(const float& _xbegin, const float& _ybegin, const float& _zbegin, const float& _xend, const float& _yend, const float& _zend);
    void setLine(const glm::vec3& _begin, const glm::vec3& _end);
    void setLine(const Line3d& line);


    [[nodiscard]] bool isPointBetweenStartAndEnd(const glm::vec3 &_point) const;

    /*
     * get the closed _point in the line
     */
    [[nodiscard]] glm::vec3 getClosestPoint(const glm::vec3& _point) const;

    [[nodiscard]] glm::vec3 getMiddle() const;
    [[nodiscard]] glm::vec3 getVector() const;
    [[nodiscard]] float getLength() const;

    glm::vec3 m_begin;
    glm::vec3 m_end;
};


#endif //SHAfloatfloatER_ENGINE_LINE_H
