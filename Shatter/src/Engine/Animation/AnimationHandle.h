//
// Created by AnWell on 2022/3/11.
//

#ifndef MAIN_ANIMATIONHANDLE_H
#define MAIN_ANIMATIONHANDLE_H

#include "abasic.h"
#include <set>
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_enum.h"

class AnimationHandle {
public:
    AnimationHandle();
    ~AnimationHandle();
    DefineUnCopy(AnimationHandle);
    void pushUI();
    void loadAnimation(const std::string& _files, bool _binary = false);
    ClassElement(m_pos, glm::vec3, Pos);
    ClassElement(m_rotationAxis, glm::vec3, RotationAxis);
    ClassElement(m_angle, float, Angle);
    ClassElement(m_scale, glm::vec3, Scale);
    ClassElement(m_id, int, Id);
    ClassElement(m_pipeline, std::string, Pipeline);
    ClassElement(m_sets, std::vector<std::string>, Sets);
    ClassElement(m_drawType, DrawObjectType, DrawType);
private:
    std::unique_ptr<ABasic> m_animation{nullptr};
};


#endif //MAIN_ANIMATIONHANDLE_H
