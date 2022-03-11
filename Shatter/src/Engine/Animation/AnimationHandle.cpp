//
// Created by AnWell on 2022/3/11.
//

#include "AnimationHandle.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/Base/GUI.h"

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;

    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

AnimationHandle::AnimationHandle() {
    GUI::pushUI("AnimationHandle",[&](){
        ImGui::Begin("AnimationSetting");
        static char buf[32] = "default";
        ImGui::InputText("filename", buf, IM_ARRAYSIZE(buf));
        if(ImGui::Button("LoadAnimation"))
        {
            loadAnimation(tool::combineAnimation(buf));
        }
        ImGui::SliderFloat3("Position",
                            reinterpret_cast<float *>(&m_pos),
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max());
        ImGui::SliderFloat3("RotationAxis",
                            reinterpret_cast<float *>(&m_rotationAxis),
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max());
        ImGui::SliderFloat("Angle",
                           reinterpret_cast<float *>(&m_angle),
                           -pai,
                           pai);
        ImGui::SliderFloat3("Scale",
                            reinterpret_cast<float *>(&m_scale),
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max());
//        m_pipeline
//        m_sets

        ImGui::End();// End setting
    });
}

void AnimationHandle::loadAnimation(const std::string &_files) {
    m_animation = std::make_unique<ABasic>(_files,
                                           m_pos,
                                           m_rotationAxis,
                                           m_angle,
                                           m_scale,
                                           mallocId(),
                                           m_pipeline,
                                           m_sets);
    SingleRender.normalChanged = true;
}

