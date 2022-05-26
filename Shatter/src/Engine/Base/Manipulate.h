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
    Manipulate();
    Manipulate(const Manipulate& _in);
    Manipulate& operator=(const Manipulate& _in);
    ~Manipulate();
public:
    void move(const glm::vec3& _deta);
    void addElevation(float _deta);
    void addRotation(float _deta);
public:
    ClassElementInitial(m_rotation, float, Rotation, 0);
    ClassElementInitial(m_elevation, float, Elevation, 0);
    ClassElementInitial(m_scale, glm::vec3, Scale, 1.0f);
    ClassElementInitial(m_animationIndex, int, AnimationIndex, 0);
    ClassElementInitial(m_localTime, float, Time, 0);
    ClassElementInitial(changed, bool, Changed, false);
    ClassProtectedElement(m_localCoordiante, int, Coordinate);
    void setMatrix(const glm::mat4& _mat);
    void setPosition(const glm::vec3& _pos);
    [[nodiscard]] int getModelId() const {return modelIndex;};
private:
    glm::mat4 m_matrix{};
public:
    glm::vec3 _rotationAxis{0,0,1};
    float _angle{0};
    glm::mat4 &getMatrix() {
        if (changed) {
            updateMatrix();
        };
        return m_matrix;
    }
    void generateAnimationMatrix();
private:
    void updateMatrix();
private:
    int modelIndex;
    vkglTF::Model* model{nullptr};
};

#endif //MAIN_MANIPULATE_H
