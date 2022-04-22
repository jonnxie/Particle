//
// Created by jonnxie on 2022/4/22.
//

#ifndef MAIN_COPYABLE_H
#define MAIN_COPYABLE_H

#include <glm.hpp>
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_enum.h"

class CopyAble {
    virtual void copy(const glm::vec3& _pos) = 0;
};


#endif //MAIN_COPYABLE_H
