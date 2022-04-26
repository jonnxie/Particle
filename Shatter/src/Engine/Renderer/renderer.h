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

struct GLFWwindow;

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

            /*
             * new render pass
             */
            void createRenderPass();

            void createMSAARenderPass();

            VkCommandBuffer m_capture_buffer = VK_NULL_HANDLE;
            VkFormat m_captureFormat = VK_FORMAT_R32_UINT;
            VkRenderPass m_captureRenderPass = VK_NULL_HANDLE;
            FrameBuffer* m_frameBuffers{nullptr};
            std::vector<std::vector<VkCommandBuffer>> pre_capture_buffers{};

            void createCaptureRenderPass();

            void createCaptureFramebuffers();

            void createCaptureCommandBuffers(VkCommandBuffer _cb, int _imageIndex);

            void updateCaptureCommandBuffers(VkCommandBuffer _cb,int _imageIndex);

            void createFramebuffers();

            void clearAttachment(FrameBufferAttachment* _attachment);

            void createAttachment(VkFormat _format, VkImageUsageFlags _usage, FrameBufferAttachment* _attachment);

            void createAttachmentResources();

            void createGraphicsCommandPool();

            void createComputeCommandPool();

            void createTransferCommandPool();

            void createDepthResources();

            void createDescriptorPool();

            void createPrimaryCommandBuffers();

            void createSecondaryCommandBuffers();

            void prepareMultipleThreadDate();

            void createGraphicsCommandBuffersMultiple();

            void updateGraphicsCommandBuffersMultiple(int _index);

            void updateOffscreenBufferAsync(VkCommandBuffer _cb,int _imageIndex) const;

            void createOffScreenBuffers(VkCommandBuffer _cb, int _imageIndex);

            void createShadowGraphicsBuffers(VkCommandBuffer _cb, int _imageIndex);

            void updateShadowGraphicsAsync(VkCommandBuffer _cb,int _imageIndex);

            void createComputeCommandBuffer();

            void createComputeCommandBuffersMultiple();

            void updateComputeCommandBufferAsync();

            void createSemaphores();

            void recreateSwapChain();

            void keyEventCallback(int key, int action);

            void mouseEventCallback(int button, int action, int xpos, int ypos);

            void cleanupSwapChain();

            void draw();

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

            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) ;
            [[nodiscard]] VkRenderPass getDefaultRenderPass() const {return m_renderPass;};
            VkRenderPass getCaptureRenderPass(){
                return m_captureRenderPass;
            };
            FrameBuffer* getCaptureFrameBuffer(){
                return m_frameBuffers;
            };
            [[nodiscard]] VkExtent2D getExtent2D() const {return swapchain_extent;};
            void allocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& des_set_layout,
                                        VkDescriptorSet* set);
            VkDevice* getDevice(){return &device;};
            [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const{return physicalDevice;};
            void createCommandBuffer();
            [[nodiscard]] GLFWwindow* getWindow() const{return window;};
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

        public:
            static void onWindowResized(GLFWwindow *window, int width, int height);
            static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
            static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
            static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
            static void keyTypeCallback(GLFWwindow* window,unsigned int code);
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
            std::vector<int> offdrawid_vec;
            std::vector<int> drawid_vec;
            std::vector<int> transparency_vec;
            ExchangeVector<int> normalIdVec;
            std::vector<int> computeid_vec;
            std::unordered_map<int, int> aabb_map;//capture id , aabb index
            std::vector<buffer::ShatterTexture*> tex_vec;
            GLFWwindow *window{};

            VkInstance instance{};
            VkDebugReportCallbackEXT callback{};
            /*
            * 表示一个可以用来展示然后渲染图形数据的对象（即表面）的句柄
            */
            VkSurfaceKHR surface{};

            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device{};

            VkQueue graphics_queue{};
            VkQueue present_queue{};
            VkQueue compute_queue{};
            VkQueue transfer_queue{};

            VkSwapchainKHR swapchain{};
            /*
            * 图像代表交换链中的项
            */
            VkFormat swapchain_image_format;
            std::vector<VkImage> swapchain_images;
            VkExtent2D swapchain_extent{};

            FrameBufferAttachment* positionAttachment{nullptr};
            FrameBufferAttachment* normalAttachment{nullptr};
            FrameBufferAttachment* albedoAttachment{nullptr};
            FrameBufferAttachment* depthAttachment{nullptr};

            VkRenderPass m_renderPass = VK_NULL_HANDLE;
            std::vector<VkImage> m_swapchainImages;
            std::vector<VkImageView> m_swapChainImageviews;
            std::vector<VkSampler> m_swapChainSamplers;
            std::vector<VkFramebuffer> m_swapChainFramebuffers;
            std::vector<VkDescriptorSet> m_swapChainSets;
            std::vector<VkCommandBuffer> composite_buffers;

            VkCommandPool graphic_commandPool{};
            VkCommandPool compute_commandPool{};
            VkCommandPool transfer_commandPool{};

            VkFormat m_depthFormat;
            VkImage m_depthImage{};
            VkDeviceMemory m_depthImageMemory{};
            VkImageView m_depthImageView{};

            VkDescriptorPool descriptorPool{};

            std::vector<VkCommandBuffer> graphics_buffers;
            std::vector<VkCommandBuffer> gui_buffer{};
            VkCommandBuffer compute_buffer{};
            std::vector<VkCommandBuffer> offscreen_buffers;

            bool guiChanged = false;
            bool offChanged = false;
            bool drawChanged = false;
            bool normalChanged = false;
            bool transChanged = false;
            bool aabbChanged = false;
            bool windowStill = true;
            std::vector<VkCommandBuffer> pre_compute_buffers;

            std::vector<std::vector<VkCommandBuffer>> pre_offscreen_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_shadow_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_g_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_norm_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_trans_buffer{};

            VkSemaphore imageAvailableSemaphore{};
            VkSemaphore renderFinishedSemaphore{};
            VkSemaphore computeFinishedSemaphore{};
            VkSemaphore computeReadySemaphore{};
            std::vector<VkClearValue> clearValues{};
            VkSubmitInfo computeSubmitInfo{};
            VkSubmitInfo graphicsSubmitInfo{};
            VkPresentInfoKHR presentInfo{};
        };
    };
};

#define SingleRender Shatter::render::ShatterRender::getRender()

#endif //VULKAN_TOTURIOL_SHATTER_RENDER_H

