//
// Created by hasee on 2022/6/11.
//

#include "precompiledhead.h"
#include "DebugGeo.h"
#include "Engine/Base/points.h"
#include "Engine/Object/inputaction.h"

DPoints* DebugGeo::DebugPoint(const glm::vec3& _position, float _radius) {
    glm::vec3 color;
    input::LineColor(color, STATE_OUT);
    Point3dColorSize info{_position, color, _radius};
    return new DPoints(std::vector<Point3dColorSize>{info});
}

DPoints* DebugGeo::DebugPoint(const std::vector<Point3dColorSize> &_points) {
    return new DPoints(_points);
}


