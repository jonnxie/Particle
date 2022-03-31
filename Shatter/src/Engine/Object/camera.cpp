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

using namespace Shatter::app;
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
    m_aspect = getViewPort().view.width / getViewPort().view.height;
    m_camera.proj = glm::perspective(m_fovy, m_aspect, m_zNear, m_zFar);
    m_camera.proj[1][1] *= -1;
}

void Camera::init() {
    auto buffer = BPool::getPool().getBuffer("CameraBuffer",Buffer_Type::Uniform_Buffer);

    VkDescriptorBufferInfo camera_buffer = {};
    camera_buffer.buffer = (*buffer)();
    camera_buffer.offset = 0;
    camera_buffer.range = sizeof(CameraBuffer);

    VkWriteDescriptorSet descriptorWrites = {};

    descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites.dstSet = SetPool::getPool()["Camera"];
    descriptorWrites.dstBinding = 0;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites.descriptorCount = 1;
    descriptorWrites.pBufferInfo = &camera_buffer;

    vkUpdateDescriptorSets(Device::getDevice()(),
                           1,
                           &descriptorWrites,
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
        if(checkMouse(GLFW_MOUSE_BUTTON_LEFT))
        {
//            getCursor(cursor_pos);
            glm::vec2 dis = cursor_pos - getCursorPressPos();
            m_alpha = static_cast<float>(m_pre_alpha) + (dis.y);
            m_gamma = static_cast<float>(m_pre_gamma) - (dis.x);
        }

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

    if(!checkMouse(GLFW_MOUSE_BUTTON_LEFT))
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
//        std::cout << std::fixed << glm::dot(m_targetPlane.x_coordinate,m_targetPlane.y_coordinate) << std::endl;
    }

    if(checkMouse(GLFW_MOUSE_BUTTON_RIGHT))
    {
//        getCursor(cursor_pos);
        glm::vec3 dis = input::getCursor() - input::getCursorPress();
//        center = pre_center - dis;
//        static bool p = true;
//        float time = timer::getTime();
//        if(time - int(time) > 0.5f && p)
//        {
//            p = false;
//            printPoint(dis);
//        }else if(time - int(time) <= 0.5f && !p){
//            p = true;
//        }
//        m_camera.view = glm::lookAt(eye + center, center, up);
        float x_times = m_camera_radius;
        float y_times = (m_camera_radius / getViewPort().view.width) * getViewPort().view.height;
        center = pre_center - m_targetPlane.x_coordinate * (cursor_pos.x - getCursorPressPos().x) * x_times +
                 m_targetPlane.y_coordinate * (cursor_pos.y - getCursorPressPos().y) * y_times;
    }else{
        pre_center = center;
    }

    if(cameraChanged) {
        m_camera.view = glm::lookAt(eye + center, center, up);
        vkQueueWaitIdle(SingleRender.graphics_queue);
        flush();
    }
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





