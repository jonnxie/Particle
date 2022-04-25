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
    ClassElementInitial(m_rotation, float, Rotation, 0);
    ClassElementInitial(m_elevation, float, Elevation, 0);
    ClassProtectedElement(m_localCoordiante, int, Coordinate);
    ClassProtectedElement(m_animationIndex, int, AnimationIndex);
    ClassProtectedElement(m_localTime, float, Time);
    ClassReferenceElement(m_matrix, glm::mat4, Matrix);
    void generateAnimationMatrix();
private:
    vkglTF::Model* model;
};

#endif //MAIN_MANIPULATE_H
