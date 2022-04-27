//
// Created by jonnxie on 2021/9/29.
//

#ifndef SHATTER_ENGINE_GUI_H
#define SHATTER_ENGINE_GUI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <glm.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <array>
#include <queue>
#include <unordered_map>
#include "Engine/Buffer/shatterbuffer.h"
#include "Engine/Item/shatter_enum.h"

class Device;

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

class GUI{
private:
    VkSampler sampler = VK_NULL_HANDLE;
    Shatter::buffer::ShatterBuffer vertexBuffer;
    Shatter::buffer::ShatterBuffer indexBuffer;
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
    bool anyItemActive = false;
public:
    bool getItemState() const {
        return anyItemActive;
    }
public:

    std::unordered_map<Task_id,std::function<void()>> m_tasks;
public:
    struct PushConstBlock {
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

    void updateUI();

    static void pushUI(Task_id _id,std::function<void()> _func);

    static void popUI(const Task_id& _id);

    void updateBuffers();

    void drawFrame(VkCommandBuffer commandBuffer);

    void initResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath);

    static GUI *getGUI();
private:
    static GUI *gui;
};

void ShowHelpMarker(const char* desc);


#endif //SHATTER_ENGINE_GUI_H
