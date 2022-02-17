//
// Created by jonnxie on 2021/9/29.
//

#include "GUI.h"
#include <mutex>

GUI *GUI::gui = new GUI;

static std::mutex lock;

GUI *GUI::getGUI() {
    pushUI("default",[&](){
        static int file_index = 0;
        ImGui::SetNextWindowSize(ImVec2(200, 500), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("Base settings");
        ImGui::SetWindowSize(ImVec2(200, 500),ImGuiCond_FirstUseEver);
        ImGui::Checkbox("MultiThread", &shatterBase.multiThread);
//        static char filenames[64] = ""; ImGui::InputText("filenames", filenames, 64);
//        text("Visible objects: %d", ((IndirectDrawState*)getIndirectDrawState())->drawCount);
//        for (uint32_t i = 0; i < MAX_LOD_LEVEL + 1; i++) {
//            text("LOD %d: %d", i, ((IndirectDrawState*)getIndirectDrawState())->lodCount[i]);
//        }
        if(ImGui::Button("captureScreenShot"))
        {
            tool::saveScreenshot(tool::combine("screenshot",file_index++) + ".ppm");
            std::cout << "action!" << std::endl;
        }
        ImGui::SliderFloat("roughness", &getMaterial().roughness, 0.1f, 1.0f);
        ImGui::SliderFloat("metallic", &getMaterial().metallic, 0.1f, 1.0f);
        ImGui::SliderFloat("R", &getMaterial().r, 0.0f, 1.0f);
        ImGui::SliderFloat("G", &getMaterial().g, 0.0f, 1.0f);
        ImGui::SliderFloat("B", &getMaterial().b, 0.0f, 1.0f);

//        ImGui::Checkbox("Display logos", &uiSettings.displayLogos);
//        ImGui::Checkbox("Display background", &uiSettings.displayBackground);
//        ImGui::Checkbox("Animate light", &uiSettings.animateLight);
        ImGui::End();
    });
    return gui;
}

void GUI::pushUI(Task_id _id, std::function<void()> _func) {
    std::lock_guard<std::mutex> guard_lock(lock);
    if(gui->m_tasks.count(_id) == 0)
    {
        gui->m_tasks[std::move(_id)] = std::move(_func);
    }else{
        WARNING(gui task is already existed!);
    }
}

void GUI::popUI(const Task_id& _id) {
    std::lock_guard<std::mutex> guard_lock(lock);
    if(gui->m_tasks.count(_id) == 0)
    {
        WARNING(gui task is already erased!);
    }else{
        gui->m_tasks.erase(_id);
    }
}
