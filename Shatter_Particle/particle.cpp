//
// Created by AnWell on 2022/2/10.
//

#include "particle.h"
#include "../Shatter_Item/shatter_enum.h"
#include "../Shatter_Render/shatter_render_include.h"
#include "../Shatter_Object/camera.h"
#include "../Shatter_Object/inputaction.h"
#include "../Shatter_Object/taskpool.h"
#include "../Shatter_Buffer/shatterbufferinclude.h"

DrawPoint::DrawPoint() {
    m_action[Event::SingleClick] = [&]() {
        static glm::vec3 realPos;
        input::cursor(realPos, STATE_OUT);
        auto point = std::make_unique<Points>(std::vector{
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
