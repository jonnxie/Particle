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
#include "Engine/Item/shatter_math.h"

#define RandomSend 007u

template<class T>
T genRandom(const T& _average,const T& _variance)
{
    std::default_random_engine engine(RandomSend);
    std::normal_distribution<T> normalDistribution(_average, _variance);
    T val = normalDistribution(engine);
    return val;
}

template<class T>
T genRandomInRange(const T& _left,const T& _right)
{
    return genRandom<T>(_left, _right);
}

/*
 * return unit render direction
 */
glm::vec3 randomDirection();

/*
 * return a vector uniformly distributed about w with maximum deflection angle δ
 */
glm::vec3 randomDirectionInRange(const glm::vec3& _average,float _angle);

/*
 * return a position uniformly distributed about a plane with a center point and a normal direction
 */
glm::vec3 randomPosition(const glm::vec3& _center,const glm::vec3& _normal,float _radius);


#endif //PARTICLE_RANDOMDIRECTION_H
