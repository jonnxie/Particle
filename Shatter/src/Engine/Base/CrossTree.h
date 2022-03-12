//
// Created by AnWell on 2022/3/12.
//

#ifndef MAIN_CROSSTREE_H
#define MAIN_CROSSTREE_H

#include <glm.hpp>
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_enum.h"
#include <array>
#include <memory>

class CrossTree {
public:
    CrossTree(const glm::vec3& _begin, const glm::vec3& _end,
              char _localIndex);
    DefineUnCopy(CrossTree);
public:
    void split();
    virtual bool check();

    ClassElement(m_volume, double, Volume);

private:
    std::array<std::unique_ptr<CrossTree>, TreeSize> m_child;
    glm::dvec3   m_begin;
    glm::dvec3   m_end;
    double      m_width;
    double      m_height;
    double      m_depth;
    char        m_localIndex;
};


#endif //MAIN_CROSSTREE_H
