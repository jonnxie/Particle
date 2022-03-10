//
// Created by jonnxie on 2022/1/28.
//
#include "precompiledhead.h"

#include "line.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Base/lines.h"
#include "Engine/Renderer/shatter_render_include.h"
#include "Engine/Object/camera.h"
#include "Engine/Object/inputaction.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Buffer/shatterbufferinclude.h"

GLine::GLine(const
vec3 &_p0, const
vec3 &_p1):p0(_p0),p1(_p1) {
//    assert(distance(_p0,_p1) > 0);
}

GLine::~GLine() {
    delete line;
}


vec3 GLine::dir() const {

    vec3 d = p0 - p1;
    return normalize(d);
}

std::vector<Plane> GLine::toPlanes() const {

    vec3 d = dir();


    vec3 n0(-d.y, d.x, 0);

    vec3 n1(-d.z, 0, d.x);

    vec3 n2(0,-d.z,d.y);

    std::vector<Plane> planes;

    return planes;
}

void GLine::draw() {
    std::vector<Line> lines{
            {
                    {p0,
                     color},
                    {p1,
                     color}
            }
    };
    line = new DLines(lines);
    line->init();
}


//DrawLine::DrawLine() {
//    m_action[Event::SingleClick] = [&]() {
//        static bool draw = false;
//        static glm::vec3 pre_pos;
//        if (draw) {
//            glm::vec3 pos;
//            input::cursor(pos, STATE_OUT);
//            auto line = std::make_unique<GLine>(pre_pos,pos);
//            line->draw();
//            lines.push_back(std::move(line));
//            draw = false;
//            SingleRender.normalChanged = true;
//        } else {
//            input::cursor(pre_pos, STATE_OUT);
//            draw = true;
//        };
//    };
//}

DrawLine::DrawLine() {
    m_action[Event::SingleClick] = [&]() {
        static bool draw = false;
        static glm::vec3 pre_pos;
        static glm::vec3 realPos;
        static VkDevice localDevice = SingleDevice();
        if (draw) {
            TaskPool::popUpdateTask("DrawLineUpdate");
            draw = false;
        } else {
            input::cursor(pre_pos, STATE_OUT);
            auto line = std::make_unique<GLine>(pre_pos,pre_pos);
            line->draw();
            lines.push_back(std::move(line));
            TaskPool::pushUpdateTask("DrawLineUpdate",[&](float _abs_time){
                auto localLine = std::move(lines.back());
                lines.pop_back();
                int id = localLine->line->id;

                glm::vec4 center = SingleCamera.m_camera.proj * SingleCamera.m_camera.view * glm::vec4(SingleCamera.center,1.0f);
                float depth = center.z / center.w;

                glm::vec4 view = glm::inverse(SingleCamera.m_camera.proj) * glm::vec4(getCursorPos(),depth,1.0f);
                view /= view.w;
                realPos = glm::inverse(SingleCamera.m_camera.view) * view;

                auto buffer = SingleBPool.getBuffer(tool::combine("DLines", id), Buffer_Type::Vertex_Host_Buffer);
                Point point{};
                point.pos = realPos;
                point.color = RED_COLOR;
                Point* ptr = (Point*)buffer->mapped;
                ptr++;
                memcpy(ptr, &point, PointSize);
//                vkUnmapMemory(localDevice, buffer->getMemory());
                lines.push_back(std::move(localLine));
            });
            SingleRender.normalChanged = true;
            draw = true;
        };
    };
}

DrawLinePool::DrawLinePool() {
    pool = std::make_unique<DLinePool>(std::vector<Line>{}, true);
    m_action[Event::SingleClick] = [&]() {
        static bool draw = false;
        static glm::vec3 pre_pos;
        static glm::vec3 realPos;
        static VkDevice localDevice = SingleDevice();
        if (draw) {
            TaskPool::popUpdateTask("DrawLinePoolUpdate");
            draw = false;
        } else {
            input::cursor(pre_pos, STATE_OUT);
            auto line = makeLine(pre_pos);
            pool->pushLine(line);
            TaskPool::pushUpdateTask("DrawLinePoolUpdate",[&](float _abs_time){
                glm::vec4 center = SingleCamera.m_camera.proj * SingleCamera.m_camera.view * glm::vec4(SingleCamera.center,1.0f);
                float depth = center.z / center.w;

                glm::vec4 view = glm::inverse(SingleCamera.m_camera.proj) * glm::vec4(getCursorPos(),depth,1.0f);
                view /= view.w;
                realPos = glm::inverse(SingleCamera.m_camera.view) * view;

                auto buffer = SingleBPool.getBuffer(tool::combine("DLinePool", pool->id), Buffer_Type::Vertex_Host_Buffer);
                Point point{};
                point.pos = realPos;
                input::LineColor(point.color, STATE_OUT);
                Line* ptr = (Line*)buffer->mapped;
                ptr += pool->lineCount-1;
                Point* pointPtr = (Point*)ptr;
                pointPtr++;
                memcpy(pointPtr, &point, PointSize);
            });
            SingleRender.normalChanged = true;
            draw = true;
        };
    };
}
