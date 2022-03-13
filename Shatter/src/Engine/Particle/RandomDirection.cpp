//
// Created by AnWell on 2022/2/10.
//
#include "precompiledhead.h"

#include "RandomDirection.h"

glm::vec3 randomDirection() {
    float theta = genRandomInRange<float>(-pai, pai);
    float y = genRandomInRange<float>(-1.0f, 1.0f);
    float r = std::sqrt(1 - y * y);
    return glm::vec3{r * std::cos(theta), y, -r * std::sin(theta)};
}

glm::vec3 randomDirectionInRange(const glm::vec3 &_average, float _angle) {
    assert(glm::length(_average) >= float_limit);
    glm::vec3 x, y, z;
    genLocalCoordinateFromZ(_average, x, y, z);
    glm::mat3 localCoordinate = glm::inverse(glm::mat3(x, y, z));//column major
    float f = genRandomInRange<float>(0.0f, 1.0f);
    float phi = glm::sqrt(f) * _angle;
    float theta = genRandomInRange<float>(-pai, pai);
    glm::vec3 localDirection{
            glm::cos(theta) * glm::sin(phi),
            glm::sin(theta) * glm::sin(phi),
            glm::cos(phi)
    };
    return localCoordinate * localDirection;
}

glm::vec3 randomPosition(const glm::vec3 &_center, const glm::vec3 &_normal, float _radius) {
    glm::vec3 x,y,z;
    genLocalCoordinateFromZ(_normal,x,y,z);
    glm::mat3 localCoordinate = glm::inverse(glm::mat3(x,y,z));//column major
    float f = genRandomInRange<float>(0.0f,1.0f);
    float phi = glm::sqrt(f) * _radius;
    float theta = genRandomInRange<float>(-pai, pai);
    glm::vec3 localDirection{
            glm::sin(theta) * _radius,
            glm::cos(theta) * _radius,
            0
    };
    return _center + localCoordinate * localDirection;
}
