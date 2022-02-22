//
// Created by AnWell on 2022/2/21.
//

#ifndef MAIN_VULKANRENDER_H
#define MAIN_VULKANRENDER_H

#include "Engine/Core/Render.h"

class VulkanRender : public Render{
public:
    void init() final {

    };

    void update() final {

    };

    void cleanup() final {

    };

    void cleanupObject() final {

    };
private:
    std::vector<VkCommandBuffer> pre_offscreen_buffers;
    std::vector<VkCommandBuffer> pre_shadow_buffers;
    std::vector<VkCommandBuffer> pre_g_buffers{};
    std::vector<VkCommandBuffer> pre_norm_buffers{};
    std::vector<VkCommandBuffer> pre_trans_buffers{};
    std::vector<VkCommandBuffer> pre_compute_buffers;

    VkSemaphore imageAvailableSemaphore{};
    VkSemaphore renderFinishedSemaphore{};
    VkSemaphore computeFinishedSemaphore{};
    VkSemaphore computeReadySemaphore{};
    std::vector<VkClearValue> clearValues{};
    VkSubmitInfo computeSubmitInfo{};
    VkSubmitInfo graphicsSubmitInfo{};
    VkPresentInfoKHR presentInfo{};
};


#endif //MAIN_VULKANRENDER_H
