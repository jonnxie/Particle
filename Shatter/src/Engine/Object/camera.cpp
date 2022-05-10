//
// Created by AnWell on 2021/6/21.
//
#include "precompiledhead.h"

#include "camera.h"
#include "Engine/Renderer/shatter_render_include.h"
#include "Engine/Buffer/shatterbufferinclude.h"
#include "inputaction.h"
#include "Engine/Pool/setpool.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Pool/bpool.h"
#include "device.h"
#include <mutex>

using namespace Shatter::App;
using namespace Shatter::render;

static std::mutex static_mutex;
static bool if_get = false;


Camera &Camera::getCamera() {
    static Camera camera;
    std::lock_guard<std::mutex> guard_mutex(static_mutex);
    if(!if_get){
        if_get = true;
        camera.init();
    }
    return camera;
}

void Camera::generateLookAt(){
    m_camera.view = glm::lookAt(eye + center, center, up);
}

void Camera::generateProj(){
    m_aspect = SingleAPP.getPresentViewPort().view.width / SingleAPP.getPresentViewPort().view.height;
    m_reverse_aspect = 1.0f / m_aspect;
    m_camera.proj = glm::perspective(m_fovy, m_aspect, m_zNear, m_zFar);
    m_camera.proj[1][1] *= -1;
}

void Camera::init() {
    auto buffer = BPool::getPool().getBuffer("CameraBuffer",Buffer_Type::Uniform_Buffer);

    VkDescriptorBufferInfo camera_buffer = {};
    camera_buffer.buffer = (*buffer)();
    camera_buffer.offset = 0;
    camera_buffer.range = sizeof(CameraBuffer);

    VkDescriptorBufferInfo view_space_buffer = {};
    view_space_buffer.buffer = (*BPool::getPool().getBuffer("ViewSpaceDepth",Buffer_Type::Uniform_Buffer))();
    view_space_buffer.offset = 0;
    view_space_buffer.range = 8;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = SetPool::getPool()["Camera"];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &camera_buffer;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = SetPool::getPool()["ViewSpaceDepth"];
    descriptorWrites[1].dstBinding = 0;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &view_space_buffer;

    vkUpdateDescriptorSets(Device::getDevice()(),
                           descriptorWrites.size(),
                           descriptorWrites.data(),
                           0,
                           nullptr);
    generateProj();
}

glm::mat4 Camera::getView() const
{
    return m_camera.view;
};

glm::mat4 Camera::getProj() const
{
    return m_camera.proj;
};


void Camera::gammaPlus(float _num) {
    m_gamma += _num * degree;
}

void Camera::alphaPlus(float _num) {
    m_alpha += _num * degree;
}

void Camera::update(bool& cameraChanged) {
    glm::vec2& cursor_pos = getCursorPos();
    if(cameraChanged)
    {
        if(checkMouse(GLFW_MOUSE_BUTTON_MIDDLE))
        {
//            getCursor(cursor_pos);
            glm::vec2 dis = cursor_pos - getCursorPressPos();
            m_alpha = static_cast<float>(m_pre_alpha) + (dis.y);
            m_gamma = static_cast<float>(m_pre_gamma) - (dis.x);
        }

        /*
         *
         */
        {
//        if(checkKey(GLFW_KEY_UP)){
//            alphaPlus(1.0f);
//            cameraChanged = false;
//        }else if(checkKey(GLFW_KEY_DOWN)){
//            alphaPlus(-1.0f);
//            cameraChanged = false;
//        }else if(checkKey(GLFW_KEY_LEFT)){
//            gammaPlus(1.0f);
//            cameraChanged = false;
//        }else if(checkKey(GLFW_KEY_RIGHT)){
//            gammaPlus(-1.0f);
//            cameraChanged = false;
//        }
        }

        up = glm::vec3(0.0f,0.0f,1.0f);
        if(m_alpha > half_pai || m_alpha < -half_pai){
            up = glm::vec3(0.0f,0.0f,-1.0f);
            if(m_alpha > pai){
                m_alpha = m_alpha - two_pai;
            }else if(m_alpha < -pai){
                m_alpha = m_alpha + two_pai;
            }
        }
    }

    if(!checkMouse(GLFW_MOUSE_BUTTON_MIDDLE))
    {
        m_pre_alpha = m_alpha;
        m_pre_gamma = m_gamma;
    }

    if(cameraChanged) {
        m_camera_radius -= getScrollPos().y;
        m_camera_radius = glm::clamp(m_camera_radius,0.0f,std::numeric_limits<float>::max());
        float cos_alpha = std::cos(m_alpha);
        eye = glm::vec3(m_camera_radius * cos_alpha * std::cos(m_gamma),
                        m_camera_radius * cos_alpha * std::sin(m_gamma),
                        m_camera_radius * std::sin(m_alpha));
        {
            m_targetPlane.z_coordinate = glm::normalize(eye);
            m_targetPlane.x_coordinate = glm::normalize(glm::cross(up,m_targetPlane.z_coordinate));
            m_targetPlane.y_coordinate = glm::normalize(glm::cross(m_targetPlane.z_coordinate,m_targetPlane.x_coordinate));
        }
    }

    if(checkMouse(GLFW_MOUSE_BUTTON_RIGHT))
    {
        center = pre_center - m_targetPlane.x_coordinate * (cursor_pos.x - getCursorPressPos().x) * m_camera_radius * 0.5f +
                 m_targetPlane.y_coordinate * (cursor_pos.y - getCursorPressPos().y) * m_camera_radius * m_reverse_aspect * 0.5f;
    }else{
        pre_center = center;
    }

    if(cameraChanged) {
        m_camera.view = glm::lookAt(eye + center, center, up);
//        vkQueueWaitIdle(SingleRender.graphics_queue);
        generateProj();
        flush();
    }
}

void Camera::updateCursorRay() const {
    glm::vec3& ray = input::getCursorRay();
    glm::vec3& target = input::getCursor();
    ray = glm::normalize(target - (center + eye));
}

glm::vec3 Camera::getPos() const {
    return center + eye;
}

void Camera::reset(){
    m_camera_radius = 4.0f;

    center = glm::vec3(0,0,0);

    m_alpha = 45.0f * pai / 180.0f;
    m_pre_alpha = 45.0f * pai / 180.0f;

    m_gamma = 45.0f * pai / 180.0f;
    m_pre_gamma = 45.0f * pai / 180.0f;
}

void Camera::flush() {
    auto buffer = BPool::getPool().getBuffer("CameraBuffer",Buffer_Type::Uniform_Buffer);
    VkDeviceMemory camera_buffer = buffer->getMemory();
    VkDevice device = SingleDevice();

    void *data;
    vkMapMemory(device,
                camera_buffer,
                0,
                CameraBufferSize,
                0, &data);
    memcpy(data, &m_camera, CameraBufferSize);
    vkUnmapMemory(device, camera_buffer);

    m_cull.projection = m_camera.proj;
    m_cull.modelview = m_camera.view;

    glm::vec3 cameraPos = center + eye;
    m_cull.cameraPos = glm::vec4(cameraPos, 1.0f) * -1.0f;

    m_vp = m_cull.projection * m_cull.modelview;
    m_frustum.update(m_vp);

    memcpy(m_cull.frustumPlanes, m_frustum.planes.data(), sizeof(glm::vec4) * 6);

    buffer = SingleBPool.getBuffer("CameraPos",Buffer_Type::Uniform_Buffer);
    memcpy(buffer->mapped, &cameraPos, one_vec3);
    buffer->flush();

    buffer = SingleBPool.getBuffer("ViewSpaceDepth",Buffer_Type::Uniform_Buffer);
    float depth[2]{m_zNear, m_zFar};
    memcpy(buffer->mapped, depth, 8);

    buffer = SingleBPool.getBuffer(tool::combine("DLines",m_dlineId),Buffer_Type::Vertex_Host_Buffer);
    std::array<Line,3> lines{
            Line{
                    Point{center,
                          PURPLE_COLOR},
                    Point{center + m_targetPlane.x_coordinate,
                          RED_COLOR}
            },
            Line{
                    Point{center,
                          PURPLE_COLOR},
                    Point{center + m_targetPlane.y_coordinate,
                          CYAN_COLOR}
            },
            Line {
                    Point{center,
                          PURPLE_COLOR},
                    Point{center + m_targetPlane.z_coordinate,
                          GREEN_COLOR}
            },
    };
    memcpy(buffer->mapped,lines.data(),TargetPlaneDoubleCoordinateSize);

}





