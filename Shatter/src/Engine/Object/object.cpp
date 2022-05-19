//
// Created by maybe on 2021/6/3.
//
#include "precompiledhead.h"

#include "object.h"
#include "gobject.h"
#include "CaptureObject.h"

TargetPlane &Object::getTargetPlane() {
    return (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->plane;
}

glm::vec3 &Object::getWorkCenter() {
    return (*MPool<Target>::getPool())[m_manipulate->getCoordinate()]->center;
}

glm::mat4 Object::getRotate() {
    TargetPlane localPlane{};
    localPlane.x_coordinate = {glm::cos(m_manipulate->getRotation()),glm::sin(m_manipulate->getRotation()),glm::sin(m_manipulate->getElevation())};
    glm::vec3 up = glm::vec3{0,0,1};
    if (m_manipulate->getElevation() > half_pai || m_manipulate->getElevation() < -half_pai) {
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
    return rotate;
}

glm::mat4 Object::getScale() {
    return glm::scale(glm::mat4(1.0f), m_manipulate->getScale());
}

Manipulate &Object::getManipulate() {
    return *m_manipulate;
}

Object::~Object() {
    release();
}

SP(Object) Object::copy(const glm::vec3& _position) {
    std::cout << "Target has not realize copy function" << std::endl;
    return nullptr;
}

void Object::move(const glm::vec3 &_move) {
    std::cout << "Target has not realize move function" << std::endl;
}

void Object::update(const glm::vec3& _movement) {
    std::cout << "Target has not realize update function" << std::endl;
}
