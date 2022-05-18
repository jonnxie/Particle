//
// Created by jonnxie on 2022/4/25.
//

#include "Manipulate.h"
#include "Engine/Pool/mpool.h"

void Manipulate::updateMatrix() {
    Target* target = (*MPool<Target>::getPool())[m_localCoordiante];

    TargetPlane localPlane{};
    localPlane.x_coordinate = {glm::cos(m_rotation),glm::sin(m_rotation),glm::sin(m_elevation)};
    glm::vec3 up = glm::vec3{0,0,1};
    if (m_elevation > half_pai || m_elevation < -half_pai) {
        up = glm::vec3{0,0,-1};
    }
    localPlane.y_coordinate = glm::cross(up, localPlane.x_coordinate);
    localPlane.z_coordinate = glm::cross(localPlane.x_coordinate, localPlane.y_coordinate);

    glm::mat4 rotate = glm::mat4 {
        glm::vec4(localPlane.x_coordinate,0.0f),
        glm::vec4(localPlane.y_coordinate,0.0f),
        glm::vec4(localPlane.z_coordinate,0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    };

    rotate = glm::rotate(glm::mat4(1.0f),_angle,_rotationAxis);

    glm::mat4 tran = glm::mat4 {
            glm::vec4{target->plane.x_coordinate, 0.0f},
            glm::vec4{target->plane.y_coordinate, 0.0f},
            glm::vec4{target->plane.z_coordinate, 0.0f},
            glm::vec4{target->center, 1.0f}};

    new ((glm::mat4*)&m_matrix) glm::mat4(tran * rotate * glm::scale(glm::mat4(1.0f), m_scale));
}

void Manipulate::move(const glm::vec3 & _deta) {
    Target* target = (*MPool<Target>::getPool())[m_localCoordiante];
    target->center += _deta;
    changed = true;
}

void Manipulate::addElevation(float _deta) {
    m_elevation += _deta;
    if (m_elevation > pai || m_elevation < -pai) {
        m_elevation -= int(m_elevation / pai) * two_pai;
    }
    changed = true;
}

void Manipulate::addRotation(float _deta) {
    m_rotation += _deta;
    if (m_rotation > two_pai || m_rotation < -two_pai) {
        m_rotation -= int(m_rotation / two_pai) * two_pai;
    }
    changed = true;
}

void Manipulate::generateAnimationMatrix() {

}

Manipulate& Manipulate::operator=(const Manipulate &_in) {
    this->m_rotation = _in.m_rotation;
    this->m_elevation = _in.m_elevation;
    this->m_scale = _in.m_scale;
    this->m_animationIndex = _in.m_animationIndex;
    this->m_localTime = _in.m_localTime;
    updateMatrix();
    return *this;
}

Manipulate::Manipulate() {
    m_localCoordiante = MPool<Target>::getPool()->malloc();
}

Manipulate::~Manipulate() {
    MPool<Target>::getPool()->free(m_localCoordiante);
}

Manipulate::Manipulate(const Manipulate& _in) {
    *this = _in;
}

