//
// Created by jonnxie on 2021/11/19.
//
#include "precompiledhead.h"
#include "listener.h"
#include CameraCatalog
#include "inputaction.h"
#include "Engine/Renderer/renderer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"

namespace Shatter{
    DeleteObject::DeleteObject() {
        m_action[Event::DeletePress] = [](){
            SingleRender.cleanupObject();
        };
    }

    CaptureObject::CaptureObject() {
        m_action[Event::SingleClick] = [](){
            glm::uvec2& coordinate = input::getMousePressCoordiante();

            uint32_t object_id = ((VulkanFrameBuffer*)SingleRender.getCaptureFrameBuffer())->capture(coordinate.x, coordinate.y, 0);
            input::captureObject(object_id, STATE_IN);
            printf("Capture Object Id: %u.\n", object_id);
        };
    }

    OutputPoint::OutputPoint() {
        m_action[Event::SingleClick] = [](){
//            glm::vec4 center = SingleCamera.m_camera.proj * SingleCamera.m_camera.view * glm::vec4(SingleCamera.center,1.0f);
//            float depth = center.z / center.w;
//            input::targetDepth(depth, STATE_IN);
////        glm::mat4 p = SingleCamera.m_camera.proj;
//            glm::vec4 view = glm::inverse(SingleCamera.m_camera.proj) * glm::vec4(getCursorPressPos(),depth,1.0f);
//            view /= view.w;
            glm::vec3& world = input::getCursor();
//            world = glm::inverse(SingleCamera.m_camera.view) * view;
//            glm::vec3 world = glm::inverse(SingleCamera.m_camera.view) * view;
//            input::cursor(world, STATE_IN);

            glm::vec3 mouse_ray = glm::normalize(world - SingleCamera.eye - SingleCamera.center);
            input::mouseRay(mouse_ray, STATE_IN);
            printf("mouse position:(%f, %f, %f)\n",world.x,world.y,world.z);
        };
        m_action[Event::DeletePress] = [](){
            SingleRender.cleanupObject();
        };
    }


    OutputLine::OutputLine() = default;
}


