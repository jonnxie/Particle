//
// Created by jonnxie on 2021/11/19.
//
#include "precompiledhead.h"
#include "listener.h"
#include CameraCatalog
#include "inputaction.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/Base/lines.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"

namespace Shatter{
    DeleteObject::DeleteObject() {
        m_action[Event::DeletePress] = [](){
            SingleRender.cleanupObject();
        };
    }

    OutputPoint::OutputPoint() {
        m_action[Event::SingleClick] = [](){
            glm::vec3& world = input::getCursor();

            glm::vec3 mouse_ray = glm::normalize(world - SingleCamera.eye - SingleCamera.center);
            input::mouseRay(mouse_ray, STATE_IN);
        };
        m_action[Event::DeletePress] = [](){
            SingleRender.cleanupObject();
        };
    }


    OutputLine::OutputLine() = default;
}


