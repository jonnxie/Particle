//
// Created by jonnxie on 2022/1/28.
//

#ifndef GAME_PLANE_H
#define GAME_PLANE_H

#include <glm.hpp>
#include <vector>
#include <iostream>

using namespace glm;

class DLines;

class DTris;

class Plane {
public:
    vec3 n;//plane normal
    float d;//distance
    vec3  p;//any point in the plane
    vec3   color{0.0f,0.5f,0.0f};


    explicit Plane(const vec3& _point = vec3{0,0,0},
                   const vec3& _normal = vec3{0,0,1});

    explicit Plane(const vec3& _p0,
                   const vec3& _p1,
                   const vec3& _p2);

    explicit Plane(const std::vector<vec3>& _samples);
    ~Plane();

    void draw();

    [[nodiscard]] float a() const { return n.x;}
    [[nodiscard]] float b() const { return n.y;}
    [[nodiscard]] float c() const { return n.z;}

    void setPlane(const vec3& _point,
                  const vec3& _normal);

    float operator[](const vec3& _p) const;

    [[nodiscard]] float pointPlaneDistSigned(const vec3& _p)const;
    [[nodiscard]] float pointPlaneDist(const vec3& _p)const;
    [[nodiscard]] vec3 projectOnto(const vec3& _p)const;

    DLines* line{nullptr};
    DTris*  tri{nullptr};
};

std::ostream & operator<<(std::ostream& _in,const Plane& _plane);

#include <deque>
#include "Engine/Object/listener.h"
#include "Engine/Object/inputaction.h"
#include "Engine/Renderer/shatter_render_include.h"

class DrawPlane : public Shatter::Listener{
public:
    DrawPlane();
    ~DrawPlane(){};
private:
    std::deque<std::unique_ptr<Plane>> planes;
};


#endif //GAME_PLANE_H
