//
// Created by jonnxie on 2022/1/13.
//

#ifndef SHATTER_ENGINE_UNIFORM_H
#define SHATTER_ENGINE_UNIFORM_H

#include <glm.hpp>

struct PassLight {
    glm::vec4 position;
    glm::vec3 color;
    float radius;
};

struct CascadeShadowStruct{
    glm::mat4 model;
    uint32_t cascadeIndex;
};

#endif //SHATTER_ENGINE_UNIFORM_H
