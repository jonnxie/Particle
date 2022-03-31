//
// Created by AnWell on 2021/6/21.
//

#ifndef SHATTER_ENGINE_CAMERA_H
#define SHATTER_ENGINE_CAMERA_H

#include <glm.hpp>
#include <GLFW/glfw3.h>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Pool/bpool.h"

class Camera {
public:
    static Camera& getCamera();
    Camera(const Camera&) = delete;
    Camera& operator= (const Camera&) = delete;
    Camera() = default;
public:
    void generateLookAt();
    void generateProj();
    void update(bool& cameraChanged);
    void reset();
    void flush();
    void init();

    [[nodiscard]] glm::mat4 getView() const ;
    [[nodiscard]] glm::mat4 getProj() const ;

    void alphaPlus(float _num = 1.0f);
    void gammaPlus(float _num = 1.0f);

    ClassElement(m_dlineId, int, LineId);

    float m_camera_radius = 4.0f;

    float m_fovy = glm::radians(45.0f);
    float m_aspect = 600.0f / 800.0f;
    float m_zNear = 0.1f;
//    float m_zFar = 100.0f;
    float m_zFar = 256.0f;

    //与xy平面夹角
    float m_alpha = 45.0f * pai / 180.0f;
    float m_pre_alpha = 45.0f * pai / 180.0f;
    //与x轴夹角
    float m_gamma = 45.0f * pai / 180.0f;
    float m_pre_gamma = 45.0f * pai / 180.0f;

    glm::vec3 eye{};
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 pre_center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

    CameraBuffer m_camera{};

    Frustum m_frustum{};

    ComputeCullUniform m_cull{};

    TargetPlane m_targetPlane{};

    glm::mat4 m_vp{};
};

#define SingleCamera Camera::getCamera()

#endif //SHATTER_ENGINE_CAMERA_H
