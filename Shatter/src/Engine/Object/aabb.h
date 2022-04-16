//
// Created by maybe on 2021/5/26.
//

#ifndef SHAfloatfloatER_ENGINE_AABB_H
#define SHAfloatfloatER_ENGINE_AABB_H

#include <glm.hpp>
#include <limits>
#include <vulkan/vulkan.h>
#include "Engine/Item/shatter_math.h"

struct Line3d;

struct AABB{
public:
//    AABB():m_min_edgy(-1,-1,-1),m_max_edgy(1,1,1){}
    AABB():m_min_edgy(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()),
    m_max_edgy(std::numeric_limits<float>::min(),std::numeric_limits<float>::min(),std::numeric_limits<float>::min()){}

    explicit AABB(const glm::vec3& _in):m_min_edgy(_in),m_max_edgy(_in){}

    AABB(const glm::vec3& _min,const glm::vec3& _max):m_min_edgy(_min),m_max_edgy(_max){}

    AABB(float _minx,float _miny,float _minz,float _maxx,float _maxy,float _maxz):
    m_min_edgy(_minx,_miny,_minz),m_max_edgy(_maxx,_maxy,_maxz){}

    bool operator == (const AABB& _other) const {return (m_min_edgy == _other.m_min_edgy && m_max_edgy == _other.m_max_edgy);}

    void addInternalPoint(const glm::vec3& _other);

    void addInternalPoint(float _x,float _y,float _z);

    [[nodiscard]] glm::vec3 getCenter() const;

    [[nodiscard]] glm::vec3 getExtent() const;

    [[nodiscard]] float getVolume() const;

    [[nodiscard]] float getArea() const;

    void repair();

    [[nodiscard]] bool isPointInside(const glm::vec3& _p) const;

    [[nodiscard]] bool isPointFloatTotalInside(const glm::vec3& _p)const;

    bool isFullInside(const AABB& _other);

    bool intersectsWithBox(const AABB& _other);

    [[nodiscard]] bool intersectsWithLine(const Line3d& _line) const;

    [[nodiscard]] bool intersectsWithLine(const glm::vec3& _line_middle,const glm::vec3& _line_vect,float _half_length)const;

    glm::vec3 m_min_edgy;
    glm::vec3 m_max_edgy;
    uint32_t m_model_index;
    VkDescriptorSet m_capture_set;
};


#endif //SHAfloatfloatER_ENGINE_AABB_H
