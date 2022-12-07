//
// Created by hasee on 2022/6/11.
//

#ifndef MAIN_DEBUGGEO_H
#define MAIN_DEBUGGEO_H

#include <iostream>
#include "Engine/Item/shatter_item.h"

class DPoints;

class DebugGeo {
    static DPoints* DebugPoint(const glm::vec3& _position, float _radius);
    static DPoints* DebugPoint(const std::vector<Point3dColorSize>& _points);
};


#endif //MAIN_DEBUGGEO_H
