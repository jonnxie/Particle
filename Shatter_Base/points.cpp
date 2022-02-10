//
// Created by AnWell on 2022/2/10.
//

#include "points.h"

Points::Points(size_t _initCount) {

}

Points::Points(const std::vector<Point3dColorSize> &_points) {

}

void Points::constructG() {
    Object::constructG();
}

void Points::constructD() {
    Object::constructD();
}

void Points::pushPoint(const Point3dColorSize &_point) {
    points.push_back(_point);
}

void Points::pushPoint(const std::vector<Point3dColorSize> &_points) {
    points.insert(points.end(),_points.begin(),_points.end());
}
