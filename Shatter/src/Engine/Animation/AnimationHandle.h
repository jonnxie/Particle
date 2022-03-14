//
// Created by AnWell on 2022/3/11.
//

#ifndef MAIN_ANIMATIONHANDLE_H
#define MAIN_ANIMATIONHANDLE_H

#include "abasic.h"
#include "Engine/Item/shatter_macro.h"

class AnimationHandle {
public:
    AnimationHandle();
    ~AnimationHandle();
    DefineUnCopy(AnimationHandle);
    void loadAnimation(const std::string& _files);
    ClassElement(m_pos, glm::vec3, Pos);
    ClassElement(m_rotationAxis, glm::vec3, RotationAxis);
    ClassElement(m_angle, float, Angle);
    ClassElement(m_scale, glm::vec3, Scale);
    ClassElement(m_id, int, Id);
    ClassElement(m_pipeline, std::string, Pipeline);
    ClassElement(m_sets, std::vector<std::string>, Sets);

private:
    std::unique_ptr<ABasic> m_animation{nullptr};
};


#endif //MAIN_ANIMATIONHANDLE_H
