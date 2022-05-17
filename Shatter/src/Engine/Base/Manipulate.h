//
// Created by jonnxie on 2022/4/25.
//

#ifndef MAIN_MANIPULATE_H
#define MAIN_MANIPULATE_H

#include <Engine/Object/CaptureObject.h>
#include "Engine/Item/shatter_enum.h"
#include ItemCatalog
#include MacroCatalog
#include GLTFCatalog

class Manipulate {
public:
    void move(const glm::vec3& _deta);
    void addElevation(float _deta);
    void addRotation(float _deta);
public:
    ClassElementInitial(m_rotation, float, Rotation, 0);
    ClassElementInitial(m_elevation, float, Elevation, 0);
    ClassElementInitial(m_scale, glm::vec3, Scale, 1.0f);
    ClassProtectedElement(m_localCoordiante, int, Coordinate);
    ClassProtectedElement(m_animationIndex, int, AnimationIndex);
    ClassProtectedElement(m_localTime, float, Time);
private:
    glm::mat4 m_matrix{};
public:
    glm::mat4 &getMatrix() {
        if (changed) updateMatrix();
        return m_matrix;
    }
    void generateAnimationMatrix();
private:
    void updateMatrix();
private:
    bool changed = false;
    vkglTF::Model* model;
};

#endif //MAIN_MANIPULATE_H
