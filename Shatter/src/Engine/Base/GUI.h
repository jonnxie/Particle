//
// Created by jonnxie on 2021/9/29.
//

#ifndef SHATTER_ENGINE_GUI_H
#define SHATTER_ENGINE_GUI_H

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Render/shatter_render_include.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <array>
#include <queue>
#include <unordered_map>

#include BufferCatalog
#include DeviceCatalog
#include ShaderCatalog

using namespace Shatter::buffer;

using namespace Shatter::render;

static struct UISettings {
    bool displayModels = true;
    bool displayLogos = true;
    bool displayBackground = true;
    bool animateLight = false;
    float lightSpeed = 0.25f;
    std::array<float, 50> frameTimes{};
    float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
    float lightTimer = 0.0f;
} uiSettings;

static struct ShatterBase {
    bool multiThread = true;
} shatterBase;


class GUI{
private:
    VkSampler sampler = VK_NULL_HANDLE;
    ShatterBuffer vertexBuffer;
    ShatterBuffer indexBuffer;
    int32_t vertexCount = 0;
    int32_t indexCount = 0;
    VkDeviceMemory fontMemory = VK_NULL_HANDLE;
    VkImage fontImage = VK_NULL_HANDLE;
    VkImageView fontView = VK_NULL_HANDLE;
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    Device* device = nullptr;
    std::unordered_map<Task_id,std::function<void()>> m_tasks;
public:
    struct PushConstBlock{
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock{};

    GUI()
    {
        ImGui::CreateContext();
    }

    GUI(const GUI& ) = delete;

    GUI& operator=(const GUI&) = delete;

    ~GUI();

    void init(float width, float height);

    void newFrame(bool updateFrameGraph);

    static void pushUI(Task_id _id,std::function<void()> _func);

    static void popUI(const Task_id& _id);

    static void text(const char *formatstr, ...)
    {
        va_list args;
        va_start(args, formatstr);
        ImGui::TextV(formatstr, args);
        va_end(args);
    }

    void updateBuffers();

    void drawFrame(VkCommandBuffer commandBuffer);

    void initResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath);

    static GUI *getGUI();
public:
    static GUI *gui;
};


#endif //SHATTER_ENGINE_GUI_H
