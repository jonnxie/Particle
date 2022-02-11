//
// Created by maybe on 2021/6/2.
//

#ifndef SHATTER_ENGINE_SHATTER_MATH_H
#define SHATTER_ENGINE_SHATTER_MATH_H

#include <iostream>
#include <glm.hpp>
//
//template<class T>
//void swap(T& _left, T _right){
//    T t = _left;
//    _left = _right;
//    _right = _left;
//}

float genRandom(float _range);

void genRotateFromQuaternion(const glm::vec4* _in,glm::mat4* _out);

void genRotateFromEulerAngle(glm::vec3* _angle,glm::mat4* _matrix);

void interpolate(const glm::vec4* _inA,const glm::vec4* _inB,float _alpha,glm::vec4* _out);

void interpolate(const glm::vec3* _inA,const glm::vec3* _inB,float _alpha,glm::vec3* _out);

void interpolate(const glm::vec2* _inA,const glm::vec2* _inB,float _alpha,glm::vec2* _out);

void setTranslate(glm::mat4* _out,glm::vec3* _transition);

void setIdentity(glm::mat4* _matrix);

void setFromEulerAngleToQuaternion(const glm::vec3& _angle,glm::vec4& _quaternion);

void invTransformAndRotate(const glm::mat4* _mat,const glm::vec3* _in,glm::vec3* _out);

/*
 * Right hand coordinate system
 */
void genLocalCoordinateFromZ(const glm::vec3& _f, glm::vec3& _x, glm::vec3& _y, glm::vec3& _z);

#endif //SHATTER_ENGINE_SHATTER_MATH_H
