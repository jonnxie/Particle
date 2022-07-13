//
// Created by maybe on 2020/11/21.
//

#ifndef VULKAN_TOTURIOL_SHATTER_RENDER_H
#define VULKAN_TOTURIOL_SHATTER_RENDER_H

#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#ifndef  GLFW_INCLUDE_VULKAN
  #define GLFW_INCLUDE_VULKAN
#endif
#include <iostream>
// #include <GLFW/glfw3.h>
#include <gtc/matrix_transform.hpp>
#include <vector>
#include <fstream>
#include <glm.hpp>
#include <array>
#include <unordered_map>
#include "Engine/App/shatterappinclude.h"
#include "Engine/Item/shatter_item.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "FrameBuffer.h"
#include "Platform/Vulkan/VulkanFrameBuffer.h"
#include <memory>
#include "Engine/Base/ExchangeVector.h"

namespace Shatter::buffer {
    class ShatterTexture;
}

class GUI;

struct SwapChainSupportDetails;

struct QueueFamilyIndices;

struct FrameBufferAttachment;

namespace Shatter{
    namespace render {

        static void check_vk_result(VkResult err);

        class ShatterRender {
        public:
            static ShatterRender& getRender();

            DefineUnCopy(ShatterRender);
            void init();

            void loop();

            void cleanup();

            void cleanupObject();
        private:
            ShatterRender() = default;
            ~ShatterRender() = default;
            void initWindow();

            void initVulkan();

            void prepareImGui();

            void createInstance();

            void setupDebugCallback();

            void createSurface();

            void pickPhysicalDevice();

            static bool enableFeatures();

            void createLogicalDevice();

            void createSwapChain();

            VkRenderPass m_colorRenderPass{VK_NULL_HANDLE};
            void createColorRenderPass();
            void createColorFramebuffers();

            uint32_t m_presentImageCount{};
            VkRenderPass m_presentRenderPass{VK_NULL_HANDLE};
            struct VkPresent {
                VkImage image;
                VkImageView imageView;
                VkSampler sampler;
                VkFramebuffer framebuffer;
            };
            void createPresentRenderPass();
            void createPresentFramebuffers();

            VkFormat m_captureFormat = VK_FORMAT_R32_UINT;
            VkRenderPass m_captureRenderPass = VK_NULL_HANDLE;
            FrameBuffer* m_captureFrameBuffer{nullptr};
            std::vector<std::vector<VkCommandBuffer>> pre_capture_buffers{};

            void createCaptureRenderPass();

            void createCaptureFramebuffers();

            void createNewCaptureCommandBuffers();

            void createGraphicsCommandPool();

            void createComputeCommandPool();

            void createTransferCommandPool();

            void createDescriptorPool();

            void freeGraphicsPrimaryCB();

            void allocateGraphicsPrimaryCB();

            void allocateComputePrimaryCB();

            void createSecondaryCommandBuffers();

            void prepareMultipleThreadDate();

            void createColorGraphicsCommandBuffersMultiple();

            void createPresentGraphicsCommandBuffers();

            void updatePresentCommandBuffers(int _index);

            void createGraphicsCommandBuffers();

            void updateOffscreenBufferAsync(VkCommandBuffer _cb,int _imageIndex) const;

            void createOffScreenBuffers(VkCommandBuffer _cb, int _imageIndex);

            void createShadowGraphicsBuffers(VkCommandBuffer _cb, int _imageIndex);

            void updateShadowGraphicsAsync(VkCommandBuffer _cb,int _imageIndex);

            void createComputeCommandBuffer();

            void createComputeCommandBuffersMultiple();

            void updateComputeCommandBufferAsync();

            void createSemaphores();

            void recreateSwapChain();

            void cleanupSwapChain();

            void newDraw();

            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

            VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

            VkFormat findDepthFormat();

            bool isDeviceSuitable(VkPhysicalDevice device);

            bool checkDeviceExtensionSupport(VkPhysicalDevice device);

            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

            std::vector<const char *> getRequiredExtensions();

            bool checkValidationLayerSupport();

        public:
            VkCommandBuffer beginSingleTimeCommands() ;
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);

            VkCommandBuffer beginSingleTimeTransCommands() ;
            void endSingleTimeTransCommands(VkCommandBuffer commandBuffer);

            void windowResize();

            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) ;
            [[nodiscard]] VkRenderPass getPresentRenderPass() const {return m_presentRenderPass;};
            [[nodiscard]] VkRenderPass getColorRenderPass() const {return m_colorRenderPass;};
            [[nodiscard]] VkRenderPass getCaptureRenderPass() { return m_captureRenderPass;};
            FrameBuffer* getCaptureFrameBuffer() {return m_captureFrameBuffer;};
            [[nodiscard]] VkExtent2D getExtent2D() const {return presentExtent;};
            VkDevice* getDevice(){return &device;};
            VkPhysicalDevice getPhysicalDevice() {return physicalDevice;};
            [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const{return physicalDevice;};
            void createCommandBuffer();
            static void updateColorSet();
        public:
            void releaseObject(int _id, DrawObjectType _type);
            void releaseComputeObject(int _id);
            void exchangeObjects();
            void pushDObjects(int _element);
            void pushCObjects(int _element);
            void pushTObjects(int _element);
            void pushNObjects(int _element);
            void pushOObjects(int _element);
            std::unordered_map<int, int>* getAABBMap();
        public:
            GUI *imGui = nullptr;
            ExchangeVector<int> drawIdVec;
            ExchangeVector<int> normalIdVec;
            std::vector<int> offdrawid_vec;
            std::vector<int> transparency_vec;
            std::vector<int> computeid_vec;
            std::unordered_map<int, int> aabb_map;//capture id , aabb index

            VkQueue graphics_queue{}, present_queue{}, compute_queue{}, transfer_queue{};

            bool resized = false;
            bool initialed = false;
            /*
            * 图像代表交换链中的项
            */
            VkFormat m_presentFormat;
//            std::vector<VkImage> m_presentImages;
            std::vector<VkPresent> m_presents{};
            VkExtent2D presentExtent{};
            VkFormat m_depthFormat;

            uint32_t m_swapChainImageCount;
            VkDescriptorSet m_colorSet;

            FrameBuffer* colorFrameBuffers{nullptr};

            bool guiChanged = false, offChanged = false, drawChanged = false, normalChanged = false, transChanged = false, aabbChanged = false, windowStill = true;
        private:
            std::vector<VkCommandBuffer> gui_buffer{}, offscreen_buffers;
            VkCommandBuffer computeCB{};
            VkCommandBuffer colorCB{};
            VkCommandBuffer captureCB{};
            std::vector<VkCommandBuffer> presentCB{};

        private:
            std::vector<VkCommandBuffer> pre_compute_buffers;
            std::vector<std::vector<VkCommandBuffer>> pre_offscreen_buffer{}, pre_shadow_buffer{};

            VkSemaphore imageAvailableSemaphore{}, renderFinishedSemaphore{}, computeFinishedSemaphore{}, computeReadySemaphore{};
            VkFence renderFence{};

            std::vector<VkClearValue> clearValues{};
            VkSubmitInfo computeSubmitInfo{}, graphicsSubmitInfo{};
            VkPresentInfoKHR presentInfo{};
            VkCommandPool graphic_commandPool{}, compute_commandPool{}, transfer_commandPool{};
        private:
            VkSwapchainKHR swapchain{VK_NULL_HANDLE};
            VkDescriptorPool descriptorPool{};
            VkInstance instance{};
            VkDebugReportCallbackEXT callback{};
            /*
            * 表示一个可以用来展示然后渲染图形数据的对象（即表面）的句柄
            */
            VkSurfaceKHR surface{};

            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device{};
        public:
            static VKAPI_ATTR VkBool32 VKAPI_CALL
            debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
                          size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData) {
                std::cerr << "validation layer: " << msg << std::endl;
                return VK_FALSE;
            }

            static void check_vk_result(VkResult err)
            {
                if (err == 0)
                    return;
                fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
                if (err < 0)
                    abort();
            }
        };
    };
};

#define SingleRender Shatter::render::ShatterRender::getRender()

#endif //VULKAN_TOTURIOL_SHATTER_RENDER_H

