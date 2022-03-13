//
// Created by AnWell on 2022/3/12.
//

#ifndef MAIN_CROSSTREE_H
#define MAIN_CROSSTREE_H

#include <glm.hpp>
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include <array>
#include <memory>
#include "Engine/Object/object.h"
#include "Engine/Particle/RandomDirection.h"
#include "lines.h"

class CrossTree {
public:
    CrossTree(const glm::dvec3& _begin, const glm::dvec3& _end,
              char _localIndex);
    DefineUnCopy(CrossTree);
public:
    virtual void split();
    virtual bool check();

    ClassElement(m_volume, double, Volume);

private:
    std::array<std::unique_ptr<CrossTree>, TreeSize> m_child;
    glm::dvec3   m_begin;
    glm::dvec3   m_end;
    double       m_width;
    double       m_height;
    double       m_depth;
    char         m_localIndex;
};

//八叉树剔除算法
struct DParticle{
    glm::dvec3 pos;
    glm::dvec3 color;
//    glm::dvec3 acceleration;
//    glm::dvec3 velocity;
//    glm::dvec3 force;
//    glm::dvec3 mass;
};

class ParticleGroup;

class ParticleNode {
public:
    ParticleNode(const glm::dvec3& _begin, const glm::dvec3& _end,
                 char _localIndex, ParticleGroup* _group);
public:
    void split();
    bool check();
    ClassElement(m_volume, double, Volume);
    ClassElement(m_leaf, bool, LeafAble);
    ClassReferenceElement(m_groups, std::vector<size_t>, GroupRef);

private:
    glm::dvec3              m_begin;
    glm::dvec3              m_end;
    double                  m_width;
    double                  m_height;
    double                  m_depth;
    char                    m_localIndex;
private:
    std::array<std::unique_ptr<ParticleNode>, TreeSize> m_child;

    ParticleGroup*          m_delegate{};
};

class ParticleGroup{
public:
    explicit ParticleGroup(size_t _size,
                           glm::dvec3 _begin,
    glm::dvec3 _end, bool _random = true, size_t _leafSize = 10);
    DefineUnCopy(ParticleGroup);

    ClassElement(m_leafSize, size_t, LeafSize);
    ClassReferenceElement(m_groups, std::vector<DParticle>, GroupRef);
    ClassReferenceElement(m_lines, std::unique_ptr<DLines>, Lines);
private:
    glm::dvec3                      m_begin;
    glm::dvec3                      m_end;
    std::unique_ptr<ParticleNode>   m_tree{};
};

#endif //MAIN_CROSSTREE_H
