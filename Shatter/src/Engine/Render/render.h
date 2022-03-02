//
// Created by maybe on 2020/11/21.
//

#ifndef VULKAN_TOTURIOL_SHATTER_RENDER_H
#define VULKAN_TOTURIOL_SHATTER_RENDER_H

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#ifndef  GLFW_INCLUDE_VULKAN
          #define GLFW_INCLUDE_VULKAN
#endif
#include <iostream>
#include <GLFW/glfw3.h>
#include <gtc/matrix_transform.hpp>
//#include <gtx/hash.hpp>
#include <vector>
#include <fstream>
#include <glm.hpp>
#include <array>
#include <unordered_map>
#include "Engine/App/shatterappinclude.h"
#include "Engine/Item/shatter_item.h"
#include "imgui.h"
#include "Engine/Item/shatter_enum.h"

#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

namespace Shatter::buffer{
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

            ShatterRender(const ShatterRender&) = delete;
            ShatterRender& operator = (const ShatterRender&) = delete;
            ShatterRender(ShatterRender&&) = delete;
            ShatterRender& operator = (ShatterRender&&) = delete;

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

            void mouseEventCallback(int button, int action, double xpos, double ypos);

            void cleanupSwapChain();

            void draw();

            void updateUI();

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
            void endSingleTimeCommands(VkCommandBuffer commandBuffer) ;
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) ;
            [[nodiscard]] VkRenderPass getRenderPass() const {return renderPass;};
            [[nodiscard]] VkRenderPass getNewRenderPass() const {return newRenderPass;};
            [[nodiscard]] VkExtent2D getExtent2D() const {return swapchain_extent;};
            void allocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& des_set_layout,
                                        VkDescriptorSet* set);
            VkDevice* getDevice(){return &device;};

            [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const{return physicalDevice;};

            [[maybe_unused]] void addDObject(int _drawId);
            void addCObject(int _computeId);
            void createCommandBuffer();
            [[nodiscard]] GLFWwindow* getWindow() const{return window;};
        public:
            static VKAPI_ATTR VkBool32 VKAPI_CALL
            debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
                          size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData) {
                std::cerr << "validation layer: " << msg << std::endl;

                return VK_FALSE;
            }

        public:
            static void onWindowResized(GLFWwindow *window, int width, int height);
            static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
            static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
            static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
            static void keyTypeCallback(GLFWwindow* window,unsigned int code);
            std::vector<int>* getDObjects();
            std::vector<int>* getCObjects();
            std::vector<int>* getTObjects();
            std::vector<int>* getNObjects();
            std::vector<int>* getOffDObjects();
        public:
            GUI *imGui = nullptr;
            std::vector<int> offdrawid_vec;
            std::vector<int> drawid_vec;
            std::vector<int> transparency_vec;
            std::vector<int> normal_vec;//Dont output g attachment
            std::vector<int> computeid_vec;
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
//            std::vector<VkImageView> swapChainImageviews;
//            std::vector<VkFramebuffer> swapChainFramebuffers;

            VkRenderPass renderPass{};

            FrameBufferAttachment* positionAttachment{nullptr};
            FrameBufferAttachment* normalAttachment{nullptr};
            FrameBufferAttachment* albedoAttachment{nullptr};
            FrameBufferAttachment* depthAttachment{nullptr};

            VkRenderPass newRenderPass = VK_NULL_HANDLE;
            std::vector<VkImage> new_swapchain_images;
            std::vector<VkImageView> new_swapChainImageviews;
            std::vector<VkFramebuffer> new_swapChainFramebuffers;
            std::vector<VkCommandBuffer> g_buffers;
            std::vector<VkCommandBuffer> composite_buffers;

            VkCommandPool graphic_commandPool{};
            VkCommandPool compute_commandPool{};
            VkCommandPool transfer_commandPool{};

            VkFormat depthFormat;
            VkImage depthImage{};
            VkDeviceMemory depthImageMemory{};
            VkImageView depthImageView{};

            VkDescriptorPool descriptorPool{};

            std::vector<VkCommandBuffer> graphics_buffers;
            VkCommandBuffer gui_buffer{};
            VkCommandBuffer compute_buffer{};
            std::vector<VkCommandBuffer> offscreen_buffers;

            bool guiChanged = false;
            bool offChanged = false;
            bool drawChanged = false;
            bool normalChanged = false;
            bool transChanged = false;
            bool windowStill = true;
            std::vector<VkCommandBuffer> pre_offscreen_buffers;
            std::vector<VkCommandBuffer> pre_shadow_buffers;
            std::vector<VkCommandBuffer> pre_buffers;
            std::vector<VkCommandBuffer> pre_g_buffers{};
            std::vector<VkCommandBuffer> pre_n_buffers{};
            std::vector<VkCommandBuffer> pre_trans_buffers{};
            std::vector<VkCommandBuffer> pre_compute_buffers;

            std::vector<std::vector<VkCommandBuffer>> pre_offscreen_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_shadow_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_g_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_norm_buffer{};
            std::vector<std::vector<VkCommandBuffer>> pre_trans_buffer{};

            std::vector<std::vector<VkCommandBuffer>> pre_compute_buffer{};

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

