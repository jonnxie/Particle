//
// Created by AnWell on 2022/2/10.
//

#ifndef PARTICLE_RANDOMDIRECTION_H
#define PARTICLE_RANDOMDIRECTION_H

#include <glm.hpp>
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Object/shatter_math.h"

#define RandomSend 007u

template<class T>
T genRandom(const T& _average,const T& _variance)
{
    std::default_random_engine engine(RandomSend);
    std::normal_distribution<T> normalDistribution(_average,_variance);
    T val = normalDistribution(engine);
    return val;
}

template<class T>
T genRandomInRange(const T& _left,const T& _right)
{
    return genRandom(_left,_right);
}

/*
 * return unit render direction
 */
glm::vec3 randomDirection()
{
    float theta = genRandomInRange(-pai,pai);
    float y = genRandomInRange(-1.0f,1.0f);
    float r = std::sqrt(1-y*y);
    return glm::vec3{r * std::cos(theta),y,-r * std::sin(theta)};
}

/*
 * return a vector uniformly distributed about w with maximum deflection angle δ
 */
glm::vec3 randomDirectionInRange(const glm::vec3& _average,float _angle)
{
    assert(glm::length(_average) <= float_limit);
    glm::vec3 x,y,z;
    genLocalCoordinateFromZ(_average,x,y,z);
    glm::mat3 localCoordinate = glm::inverse(glm::mat3(x,y,z));//column major
    float f = genRandomInRange(0.0f,1.0f);
    float phi = glm::sqrt(f) * _angle;
    float theta = genRandomInRange(-pai, pai);
    glm::vec3 localDirection{
            glm::cos(theta) * glm::sin(phi),
            glm::sin(theta) * glm::sin(phi),
            glm::cos(phi)
    };
    return localCoordinate * localDirection;
}

/*
 * return a position uniformly distributed about a plane with a center point and a normal direction
 */
glm::vec3 randomPosition(const glm::vec3& _center,const glm::vec3& _normal,float _radius)
{
    glm::vec3 x,y,z;
    genLocalCoordinateFromZ(_normal,x,y,z);
    glm::mat3 localCoordinate = glm::inverse(glm::mat3(x,y,z));//column major
    float f = genRandomInRange(0.0f,1.0f);
    float phi = glm::sqrt(f) * _radius;
    float theta = genRandomInRange(-pai, pai);
    glm::vec3 localDirection{
            glm::sin(theta) * _radius,
            glm::cos(theta) * _radius,
            0
    };
    return _center + localCoordinate * localDirection;
}


#endif //PARTICLE_RANDOMDIRECTION_H
