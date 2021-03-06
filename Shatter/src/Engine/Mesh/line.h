//
// Created by jonnxie on 2022/1/28.
//

#ifndef GAME_LINE_H
#define GAME_LINE_H

#include <glm.hpp>
#include <vector>
#include <deque>
#include "plane.h"
#include "Engine/Object/listener.h"

class DLines;
using namespace glm;

class DLinePool;

using namespace Shatter;

class GLine
{
public:
    explicit GLine(const
    vec3& _p0,const
    vec3 & _p1);
    ~GLine();
    GLine(const GLine&) = delete;
    GLine(GLine&&) = delete;
    GLine& operator=(const GLine&) = delete;
    GLine& operator=(GLine&&)=delete;

    void draw();
    [[nodiscard]] std::vector<Plane> toPlanes() const;
    [[nodiscard]]
    vec3 dir() const;

public:
    DLines* line{nullptr};

    vec3   p0, p1;
    vec3   color{0.0f,0.5f,0.0f};
};

class DrawLine : public Shatter::Listener{
public:
    DrawLine();
    ~DrawLine() override;
private:
    std::deque<std::unique_ptr<GLine>> lines;
};

class DrawLinePool : public Shatter::Listener{
public:
    DrawLinePool();
    ~DrawLinePool() override;
private:
    std::unique_ptr<DLinePool> pool;
};


#endif //GAME_LINE_H
