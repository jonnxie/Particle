//
// Created by jonnxie on 2022/1/28.
//

#ifndef GAME_LINE_H
#define GAME_LINE_H

#include <glm.hpp>
#include <vector>
#include <deque>
#include "plane.h"
#include "../Shatter_Object/listener.h"

class Lines;
using namespace glm;

class GLine
{
public:
    explicit GLine(const dvec3& _p0,const dvec3 & _p1);
    ~GLine();
    GLine(const GLine&) = delete;
    GLine(GLine&&) = delete;
    GLine& operator=(const GLine&) = delete;
    GLine& operator=(GLine&&)=delete;

    void draw();
    [[nodiscard]] std::vector<Plane> toPlanes() const;
    [[nodiscard]] dvec3 dir() const;

public:
    Lines* line{nullptr};
    dvec3  p0, p1;
    vec3   color{0.0f,0.5f,0.0f};
};

class DrawLine : public Listener{
public:
    DrawLine();
private:
    std::deque<std::unique_ptr<GLine>> lines;
};


#endif //GAME_LINE_H
