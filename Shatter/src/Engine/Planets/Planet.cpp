//
// Created by AnWell on 2022/2/26.
//

#include "Planet.h"

Planet::Planet(uint32_t _resolution, glm::vec3 _pos, glm::vec3 _rotationAxis, float _angle, glm::vec3 _scale,
               uint32_t _id,
               std::string _pipeline, std::vector<std::string> _sets) {
    m_scale = glm::scale(glm::mat4(1.0f),_scale);
    m_rotate = glm::rotate(glm::mat4(1.0f),_angle,_rotationAxis);
    m_translation = glm::translate(glm::mat4(1.0f), _pos);
    m_world = m_translation * m_scale * m_rotate;
    m_id = _id;
    init();
}

Planet::~Planet() {

}

void Planet::generateMesh() {

}

void Planet::constructG() {
    Object::constructG();
}

void Planet::constructD() {
    Object::constructD();
}

void Planet::constructC() {

}
