//
// Created by AnWell on 2022/3/12.
//

#include "CrossTree.h"

CrossTree::CrossTree(const glm::dvec3 &_begin, const glm::dvec3 &_end, char _localIndex):
m_begin(_begin),
m_end(_end),
m_localIndex(_localIndex)
{
    auto dis = m_end - m_begin;
    m_width = dis.x;
    m_height = dis.y;
    m_depth = dis.z;
//    if(check()) split();
}

/*
 *
 */
void CrossTree::split() {
    uint32_t index = 0;
    glm::dvec3 dis = (m_end - m_begin) / 2.0;
    glm::dvec3 begin = m_begin;
    glm::dvec3 end = m_begin + dis;
    glm::dvec3 move;
    for(size_t i = 0; i < TreeSize; i++)
    {
        move = glm::dvec3(i & 1 ? dis.x : 0, i & (1 << 1) ? dis.y : 0, i & (1 << 2) ? dis.z : 0);
        m_child[i] = std::make_unique<CrossTree>(begin + move, end + move, i);
    }
}

bool CrossTree::check() {
    return false;
}

ParticleNode::ParticleNode(const glm::dvec3& _begin, const glm::dvec3& _end, char _localIndex, ParticleGroup* _group):
m_begin(_begin),
m_end(_end),
m_localIndex(_localIndex),
m_delegate(_group)
{
    auto dis = m_end - m_begin;
    m_width = dis.x;
    m_height = dis.y;
    m_depth = dis.z;
}

void ParticleNode::split() {
    uint32_t index = 0;
    glm::dvec3 dis = (m_end - m_begin) / 2.0;
    glm::dvec3 begin = m_begin;
    glm::dvec3 end = m_begin + dis;
    glm::dvec3 move;
    for(size_t i = 0; i < TreeSize; i++)
    {
        move = glm::dvec3(i & 1 ? dis.x : 0, i & (1 << 1) ? dis.y : 0, i & (1 << 2) ? dis.z : 0);
        m_child[i] = std::make_unique<ParticleNode>(begin + move, end + move, i, m_delegate);
    }
    std::vector<DParticle>& particles = m_delegate->getGroupRef();

    index= 0;
    for(auto& p : m_groups)
    {
        move = (particles[p].pos - m_begin);
        if(move.x > dis.x) index |= 1;
        if(move.y > dis.y) index |= (1 << 1);
        if(move.z > dis.z) index |= (1 << 2);
        m_child[index]->getGroupRef().push_back(p);
    }

    for(size_t i = 0; i < TreeSize; i++)
    {
        if(m_child[i]->check())
        {
            m_child[i]->split();
        }
    }
}

bool ParticleNode::check(){
    if(m_groups.size() > m_delegate->getLeafSize())
    {
        m_leaf = false;
        return true;
    }else{
        m_leaf = true;
        return false;
    }
}
