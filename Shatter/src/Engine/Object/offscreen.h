//
// Created by jonnxie on 2021/10/13.
//

#ifndef SHATTER_ENGINE_OFFSCREEN_H
#define SHATTER_ENGINE_OFFSCREEN_H

#include <vulkan/vulkan.h>
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"


class FrameBuffer;

class OffScreen {
public:
    static OffScreen& getOffScreen();
    DefineUnCopy(OffScreen);

    void release();
    void beginRenderPass(VkCommandBuffer _cb,bool _multiple = false) const;
    void endRenderPass(VkCommandBuffer _cb);
public:
    uint32_t m_width;
    uint32_t m_height;
    VkRenderPass    m_pass;
    VkFramebuffer   m_frame_buffer;

    VkImage         m_depth_image;
    VkImageView     m_depth_image_view;
    VkDeviceMemory  m_depth_memory;
    VkFormat        m_depth_format;

    VkImage         m_color_image;
    VkImageView     m_color_image_view;
    VkDeviceMemory  m_color_memory;
    VkFormat        m_color_format;

    VkSampler       m_sampler;

    VkDescriptorImageInfo m_color_descriptor;
    VkDescriptorImageInfo m_depth_descriptor;

    VkCommandBufferInheritanceInfo m_inherit_info;

    FrameBuffer* m_newframebuffer{nullptr};
private:
    void setup(const VkDevice& _device,
               uint32_t _width,
               uint32_t _height);
    OffScreen() = default;
};

#define SingleOffScreen  OffScreen::getOffScreen()

struct Cascade{
    VkFramebuffer frameBuffer;
    VkDescriptorSet descriptorSet;
    VkImageView view;

    float splitDepth;
    glm::mat4 viewProjMatrix;

    void destroy(VkDevice device) const{
        vkDestroyImageView(device, view, nullptr);
        vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }
};

class CascadeShadow{
public:
    static CascadeShadow& getCascadeShadow();
    CascadeShadow(const CascadeShadow&) = delete;
    CascadeShadow& operator=(const CascadeShadow&) = delete;
    CascadeShadow(CascadeShadow&&) = delete;
    CascadeShadow& operator=(CascadeShadow&&) = delete;

    void release();
    void beginRenderPass(VkCommandBuffer _cb,uint32_t _index,bool _multiple = false);
    void endRenderPass(VkCommandBuffer _cb);

    struct DepthImage {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
        VkSampler sampler;
        VkDescriptorImageInfo descriptor;
        void destroy(VkDevice device) const{
            vkDestroyImageView(device, view, nullptr);
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, mem, nullptr);
            vkDestroySampler(device, sampler, nullptr);
        }
    } depth{};

public:
    struct DepthPass {
        VkRenderPass renderPass;
        VkPipelineLayout  pipelineLayout;
        VkPipeline pipeline;
        B_id uniformBuffer;
        struct UniformBlock{
            std::array<glm::mat4, SHADOW_MAP_CASCADE_COUNT> cascadeViewProjMat;
        } ubo;
        void destroy(VkDevice device) const {
            vkDestroyRenderPass(device, renderPass, nullptr);
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            vkDestroyPipeline(device, pipeline, nullptr);
        }
        VkRenderPass operator()() const{
            return renderPass;
        }
    } depthPass;

private:
    void setup(const VkDevice& _device,
               uint32_t _width,
               uint32_t _height);
    CascadeShadow() = default;

public:
    VkCommandBufferInheritanceInfo inheritInfo{};
private:
    std::array<Cascade,SHADOW_MAP_CASCADE_COUNT> cascades{};
};

#define SingleCascade  CascadeShadow::getCascadeShadow()

#endif //SHATTER_ENGINE_OFFSCREEN_H
