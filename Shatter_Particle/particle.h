//
// Created by AnWell on 2022/2/10.
//

#ifndef PARTICLE_PARTICLE_H
#define PARTICLE_PARTICLE_H

#include <glm.hpp>
#include <vector>
#include "../Shatter_Base/points.h"
#include "../Shatter_Object/listener.h"

class DrawPoint : public Listener{
public:
    DrawPoint();
private:
    std::deque<std::unique_ptr<Points>> points;
};

struct Particle{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 acceleration;
    glm::vec3 velocity;
    glm::vec3 force;
    glm::vec3 mass;
};

class ParticleList{
public:
    std::vector<Particle> particles;
};

class ParticleGenerator{
public:
    virtual void generateParticles(ParticleList& _particleList,float _absTime) = 0;
};

class Omnidirectional : public ParticleGenerator{
public:
    void generateParticles(ParticleList& _particleList,float _absTime) final;

public:
    glm::vec3 center;
};

class Directed : public ParticleGenerator{
public:
    void generateParticles(ParticleList& _particleList,float _absTime) final;
};

class FDisk : public ParticleGenerator{
public:
    void generateParticles(ParticleList &_particleList,float _absTime) final;
};

class FTriangle : public ParticleGenerator{
public:
    void generateParticles(ParticleList& _particleList,float _absTime) final;
};

class FSphere : public ParticleGenerator{
public:
    void generateParticles(ParticleList& _particleList,float _absTime) final;
};

#endif //PARTICLE_PARTICLE_H
