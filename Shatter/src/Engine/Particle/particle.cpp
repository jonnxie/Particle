//
// Created by AnWell on 2022/2/10.
//
#include "precompiledhead.h"

#include "particle.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Render/shatter_render_include.h"
#include "Engine/Object/camera.h"
#include "Engine/Object/inputaction.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Buffer/shatterbufferinclude.h"

DrawPoint::DrawPoint() {
    m_action[Event::SingleClick] = [&]() {
        static glm::vec3 realPos;
        input::cursor(realPos, STATE_OUT);
        auto point = std::make_unique<DPoints>(std::vector{
                                                     Point3dColorSize{
                                                             realPos,
                                                             RED_COLOR,
                                                             4}
                                             }
        );
        point->init();
        points.push_back(std::move(point));
        SingleRender.normalChanged = true;
    };
}


void Omnidirectional::generateParticles(ParticleList &_particleList,float _absTime) {

}

void Directed::generateParticles(ParticleList &_particleList,float _absTime) {

}

void FDisk::generateParticles(ParticleList &_particleList,float _absTime) {

}

void FTriangle::generateParticles(ParticleList &_particleList,float _absTime) {

}

void FSphere::generateParticles(ParticleList &_particleList,float _absTime) {

}
