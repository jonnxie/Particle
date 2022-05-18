//
// Created by jonnxie on 2022/5/18.
//

#ifndef MAIN_OPERATOR_H
#define MAIN_OPERATOR_H

#include "Engine/Item/shatter_macro.h"
#include <mutex>
#include <string>
#include <algorithm>
#include <memory>

class Object;

void pushObject2MainScene(SP(Object) _obj);


#endif //MAIN_OPERATOR_H
