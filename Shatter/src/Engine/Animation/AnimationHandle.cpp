//
// Created by AnWell on 2022/3/11.
//

#include "AnimationHandle.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/Base/GUI.h"
#include PPoolCatalog
#include SetPoolCatalog
#include "Engine/Object/inputaction.h"

static int mallocId()
{
    static int initIdVal = 0;
    static std::mutex idLock;

    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

static void ShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

AnimationHandle::AnimationHandle() {

}

void AnimationHandle::pushUI() {
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
                            std::numeric_limits<float>::min() / 2.0f,
                            std::numeric_limits<float>::max() / 2.0f);
        ImGui::SliderFloat3("RotationAxis",
                            reinterpret_cast<float *>(&m_rotationAxis),
                            std::numeric_limits<float>::min() / 2.0f,
                            std::numeric_limits<float>::max() / 2.0f);
        ImGui::SliderFloat("Angle",
                           reinterpret_cast<float *>(&m_angle),
                           -pai,
                           pai);
        ImGui::SliderFloat3("Scale",
                            reinterpret_cast<float *>(&m_scale),
                            std::numeric_limits<float>::min() / 2.0f,
                            std::numeric_limits<float>::max() / 2.0f);

        if (ImGui::TreeNode("Select Pipeline"))
        {
            static int selected = -1;
            int num = 0;
            for(auto& [id, val] : SinglePPool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selected == num))
                {
                    selected = num;
                    m_pipeline = id;
                }
                num++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Animation Index"))
        {
            static int selected = -1;
            int num = 0;
            for(auto& a : m_animation->getModel()->animations)
            {
                char buf[32];
                sprintf(buf, a.name.c_str());
                if (ImGui::Selectable(buf, selected == num))
                {
                    selected = num;
                    m_animation->setAnimationIndex(num);
                    m_animation->getModel()->resetAnimation();
                }
                num++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Select Descriptor Sets"))
        {
            ShowHelpMarker("Hold CTRL and click to select multiple items.");
            auto count = SingleSetPool.m_map.size();
            auto static selection = std::vector<int>(count);
//            static bool selection[count] = { false, false, false, false, false };
            int num = 0;
            for(auto& [id, val] : SingleSetPool.m_map)
            {
                char buf[32];
                sprintf(buf, id.c_str());
                if (ImGui::Selectable(buf, selection[num] == 1))
                {
                    if (!checkKey(GLFW_KEY_LEFT_CONTROL) && !checkKey(GLFW_KEY_RIGHT_CONTROL))
                    {
                        for(auto& s : selection)
                        {
                            s = 0;
                        }
                        m_sets.clear();
                    }// Clear selection when CTRL is not held
                    selection[num] ^= 1;
                    m_sets.push_back(id);
                }
                num++;
            }
            ImGui::TreePop();
        }

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
                                           m_sets,
                                           m_drawType);
    pushUI();
    SingleRender.normalChanged = true;
}

AnimationHandle::~AnimationHandle() {
    GUI::popUI("popUI");
}

