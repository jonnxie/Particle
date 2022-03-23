//
// Created by AnWell on 2022/3/12.
//
#include "precompiledhead.h"
#include "CrossTree.h"
#include "Engine/Item/configs.h"
#include "Engine/Object/inputaction.h"

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
    if(!check())
    {
        std::vector<glm::vec3> cubeBuffer{};
        genLineVertexBuffer(m_begin, m_end, cubeBuffer);
        std::vector<Line> line(cubeBuffer.size() / 2);
        glm::vec3 color;
        input::LineColor(color, STATE_OUT);
        for (int i = 0; i < line.size(); ++i) {
            line[i].begin = {cubeBuffer[2 * i],
                             color};
            line[i].end = {cubeBuffer[2 * i + 1],
                           color};
        }
        m_delegate->getLines()->pushLines(line);
    }
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

    for(auto& p : m_groups)
    {
        index= 0;
        move = (particles[p].pos - m_begin);
        if(move.x > dis.x) index |= 1;
        if(move.y > dis.y) index |= (1 << 1);
        if(move.z > dis.z) index |= (1 << 2);
        m_child[index]->getGroupRef().push_back(p);
//        std::cout << index << std::endl;
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

ParticleGroup::ParticleGroup(size_t _size, glm::dvec3 _begin, glm::dvec3 _end, bool _random, size_t _leafSize):
m_leafSize(_leafSize),
m_begin(_begin),
m_end(_end)
{
    m_groups = std::vector<DParticle>(_size);
    if(_random)
    {
        std::vector<glm::vec3> colors =
                {
                        glm::vec3(1.0f, 1.0f, 1.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f),
                        glm::vec3(1.0f, 1.0f, 0.0f),
                };

        std::default_random_engine rndGen( (unsigned)time(nullptr));
        std::uniform_real_distribution<double> rndXDist(m_begin.x, m_end.x);
        std::uniform_real_distribution<double> rndYDist(m_begin.y, m_end.y);
        std::uniform_real_distribution<double> rndZDist(m_begin.z, m_end.z);
        std::uniform_real_distribution<double> rndCDist(0, 1);

        for(auto& p : m_groups)
        {
            p.pos.x = rndXDist(rndGen);
            p.pos.y = rndYDist(rndGen);
            p.pos.z = rndZDist(rndGen);
//            printPoint(p.pos);
            p.color.x = rndCDist(rndGen);
            p.color.y = rndCDist(rndGen);
            p.color.z = rndCDist(rndGen);
        }
    }
    m_lines = std::make_unique<DLines>(std::vector<Line>{});
    m_tree = std::make_unique<ParticleNode>(m_begin, m_end, 0, this);
    m_tree->getGroupRef() = std::vector<size_t>(_size);
    for (int i = 0; i < _size; ++i)
    {
        m_tree->getGroupRef()[i] = i;
    }
    if(m_tree->check())
    {
        m_tree->split();
    }
    m_lines->init();
}
