//
// Created by jonnxie on 2022/1/28.
//

#ifndef GAME_PLANE_H
#define GAME_PLANE_H

#include <glm.hpp>
#include <vector>
#include <iostream>

using namespace glm;

class Lines;

class Tris;

class Plane {
public:
    dvec3 n;//plane normal
    double d;//distance
    dvec3  p;//any point in the plane
    vec3   color{0.0f,0.5f,0.0f};


    explicit Plane(const dvec3& _point = dvec3{0,0,0},
                   const dvec3& _normal = dvec3{0,0,1});

    explicit Plane(const dvec3& _p0,
                   const dvec3& _p1,
                   const dvec3& _p2);

    explicit Plane(const std::vector<dvec3>& _samples);
    ~Plane(){
        delete line;
        delete tri;
    }


    void draw();

    [[nodiscard]] double a() const { return n.x;}
    [[nodiscard]] double b() const { return n.y;}
    [[nodiscard]] double c() const { return n.z;}

    void setPlane(const dvec3& _point,
                  const dvec3& _normal);

    double operator[](const dvec3& _p) const;

    [[nodiscard]] double pointPlaneDistSigned(const dvec3& _p)const;
    [[nodiscard]] double pointPlaneDist(const dvec3& _p)const;
    [[nodiscard]] dvec3 projectOnto(const dvec3& _p)const;

    Lines* line{nullptr};
    Tris*  tri{nullptr};
};

std::ostream & operator<<(std::ostream& _in,const Plane& _plane);

#include <deque>
#include "../Shatter_Object/listener.h"
#include "../Shatter_Object/inputaction.h"
#include "../Shatter_Render/shatter_render_include.h"

class DrawPlane : public Listener{
public:
    DrawPlane();
private:
    std::deque<std::unique_ptr<Plane>> planes;
};


#endif //GAME_PLANE_H
