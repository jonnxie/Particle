//
// Created by maybe on 2020/11/21.
//
#include "precompiledhead.h"

#include "renderer.h"
#include <GLFW/glfw3.h>

#include <stb_image.h>
#include <stdexcept>
#include <cstring>
#include <set>
#include <chrono>

#include <tiny_obj_loader.h>
#include "Engine/Object/inputaction.h"
#include "Engine/Pool/mpool.h"
#include "Engine/Object/dobject.h"
#include "Engine/Object/cobject.h"
#include "Engine/Event/threadpool.h"
#include "Engine/Event/objecttask.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/configs.h"
#include "Engine/Buffer/shatterbufferinclude.h"
#include "Engine/Base/GUI.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Object/offscreen.h"
#include "Engine/Pool/setpool.h"
#include "Engine/Pool/ppool.h"
#include "pipeline.h"
#include "Engine/Object/aabb.h"
#include "Engine/Object/camera.h"
#include "Engine/Renderer/window.h"

namespace Shatter::render{
    bool render_created = false;

    static int min_image_count = 0;

    void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    const std::vector<const char *> validationLayers = {
//            "VK_LAYER_NV_optimus",
//            "VK_LAYER_AMD_switchable_graphics",
            "VK_LAYER_RENDERDOC_Capture",
//            "VK_LAYER_LUNARG_monitor"
            "VK_LAYER_KHRONOS_validation",
//            "VK_LAYER_LUNARG_api_dump"
//            "VK_LAYER_LUNARG_device_simulation"
    };

    const std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
//            VK_NV_MESH_SHADER_EXTENSION_NAME,
//            VK_NV_TASK_SHADER_EXTENSION_NAME,
//            VK_NV_GLSL_SHADER_EXTENSION_NAME
//            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
//            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
//            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
//            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
//            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
//            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
//            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
    };

    VkResult CreateDebugReportCallbackEXT(VkInstance instance,
                                          const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugReportCallbackEXT *pCallback) {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance,
                                                                               "vkCreateDebugReportCallbackEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugReportCallbackEXT(VkInstance instance,
                                       VkDebugReportCallbackEXT callback,
                                       const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugReportCallbackEXT");
        if (func != nullptr) {
            func(instance, callback, pAllocator);
        }
    }

    ShatterRender& ShatterRender::getRender(){
        static ShatterRender render;
        if(!render_created) {
            render_created = true;
            render.init();
        }
        return render;
    }

    void ShatterRender::init(){
        clearValues.resize(AttachmentCount);
        clearValues[0].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[1].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[2].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[3].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[4].depthStencil = { 1.0f, 0 };
        initWindow();
        initVulkan();
        initialed = true;
    }

    void ShatterRender::loop(){
//        glfwPollEvents();
        if (initialed) {
            if(Config::getConfig("enableScreenGui"))
            {
                GUI::getGUI()->updateUI();
            }
            newDraw();
        }
    }

    void ShatterRender::cleanup(){
        cleanupSwapChain();
        if (m_colorRenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, m_colorRenderPass, nullptr);
            colorFrameBuffers->release();
            colorFrameBuffers->releaseCaptureVals();
            delete colorFrameBuffers;
        }

        if (m_presentRenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, m_presentRenderPass, nullptr);
        }

#ifdef SHATTER_GPU_CAPTURE
        if(m_captureRenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, m_captureRenderPass, nullptr);
        }
        m_captureFrameBuffer->release();
        m_captureFrameBuffer->releaseCaptureVals();
        delete m_captureFrameBuffer;
#endif

        vkDestroySwapchainKHR(device, swapchain, nullptr);

        delete imGui;

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(device, computeFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, computeReadySemaphore, nullptr);

        vkDestroyFence(device, renderFence, nullptr);

        vkDestroyCommandPool(device, graphic_commandPool, nullptr);
        vkDestroyCommandPool(device, compute_commandPool, nullptr);
        vkDestroyCommandPool(device, transfer_commandPool, nullptr);

        releaseThreadObjectPool();
        releaseThreadCommandPool();

        vkDestroyDevice(device, nullptr);
        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        Config::setConfig("RendererReleased", 1);
    }

    void ShatterRender::initWindow(){
        setWindowViewPort(UnionViewPort{
                VkViewport{0,
                           0,
                           float(SingleAPP.getWindowSize().first),
                           float(SingleAPP.getWindowSize().second),
                           0,
                           1.0f}
        });

        setWindowScissor(VkRect2D{
                VkOffset2D{0, 0},
                VkExtent2D{uint32_t(SingleAPP.getWindowSize().first), uint32_t(SingleAPP.getWindowSize().second)}
        });
    }

    void ShatterRender::initVulkan(){
        createInstance();
        setupDebugCallback();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createColorRenderPass();
        createColorFramebuffers();
        createPresentRenderPass();
#ifdef SHATTER_GPU_CAPTURE
        createCaptureRenderPass();
        createCaptureFramebuffers();
#endif
        createGraphicsCommandPool();
        createComputeCommandPool();
        createTransferCommandPool();
        createDescriptorPool();
//        createDepthResources();
//        createFramebuffers();
        createPresentFramebuffers();
        prepareImGui();
        allocateGraphicsPrimaryCB();
        allocateComputePrimaryCB();
        prepareMultipleThreadDate();
        createSemaphores();
    }

    void ShatterRender::createInstance(){
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested,but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "NJUST Project";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Shatter Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void ShatterRender::setupDebugCallback(){
        if (!enableValidationLayers) return;

        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug callback!");
        }
    }

    void ShatterRender::createSurface(){
        if (glfwCreateWindowSurface(instance, ((GLFWWindow*)SingleAPP.getMainWindow())->get(), nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void ShatterRender::windowResize() {
        if (!initialed) {
            return;
        }
        initialed = false;
        resized = true;
        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();

        createPresentFramebuffers();

        prepareImGui();

        freeGraphicsPrimaryCB();

        allocateGraphicsPrimaryCB();

        vkDeviceWaitIdle(device);

        initialed = true;
    }

    void ShatterRender::pickPhysicalDevice(){
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        //选定合适的device
        for (const auto &tmp_device : devices) {
            if (isDeviceSuitable(tmp_device)) {
                physicalDevice = tmp_device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        setVkPhysicalDeviceMemoryProperties(memProperties);
    }

    bool ShatterRender::enableFeatures(){
        bool config = false;
//        if(Config::getConfig("enableRayTracing"))
//        {
//            Raytracing::getTracer()->enableFeatures();
//            config = true;
//        }

        if(Config::getConfig("enableMeshShader"))
        {
            static VkPhysicalDeviceMeshShaderFeaturesNV meshFeatures = {};
            meshFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
            meshFeatures.pNext = getDeviceCreateNextChain();
            meshFeatures.taskShader = VK_TRUE;
            meshFeatures.meshShader = VK_TRUE;
            setDeviceCreateNextChain(&meshFeatures);
            config = true;
        }
        return config;
    }

    void ShatterRender::createLogicalDevice(){
        QueueFamilyIndices indices = getIndices();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int> uniqueQueueFamilies = {indices.graphicsFamily,
                                             indices.presentFamily};

        float queuePriority = 1.0f;
        for (int queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.fillModeNonSolid = VK_TRUE;
        deviceFeatures.multiDrawIndirect = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
        if(enableFeatures())
        {
            physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            physicalDeviceFeatures2.features = deviceFeatures;
            physicalDeviceFeatures2.pNext = getDeviceCreateNextChain();
            createInfo.pEnabledFeatures = nullptr;
            createInfo.pNext = &physicalDeviceFeatures2;
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
//        setIndices(indices);
//        if(Config::getConfig("enableRayTracing"))
//        {
//            Raytracing::getTracer()->prepare();
//        }

        vkGetDeviceQueue(device, indices.computeFamily, 0,&compute_queue);
        vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphics_queue);
        vkGetDeviceQueue(device, indices.presentFamily, 0, &present_queue);
        vkGetDeviceQueue(device, indices.transferFamily, 0, &transfer_queue);
    }
    void ShatterRender::createSwapChain(){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ::chooseSwapExtent(swapChainSupport.capabilities, SingleAPP.getWindowSize().first,
                                               SingleAPP.getWindowSize().second);

        setSwapChainFormat(surfaceFormat);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainKHR oldSwapchain = swapchain;

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.oldSwapchain = oldSwapchain;

        QueueFamilyIndices indices = getIndices();

        std::set<uint32_t> queueFamilyIndicesSets = {(uint32_t) indices.graphicsFamily,
                                                     (uint32_t) indices.presentFamily,
                                                     (uint32_t) indices.transferFamily,
                                                     (uint32_t) indices.computeFamily};

        std::vector<uint32_t> queueFamilyIndices{};
        for(auto& set : queueFamilyIndicesSets)
        {
            queueFamilyIndices.emplace_back(set);
        }

        std::array<uint32_t, 1> presentIndices{uint32_t(indices.graphicsFamily)};
//        if (indices.graphicsFamily != indices.presentFamily) {
//            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//            createInfo.queueFamilyIndexCount = presentIndices.size();
//            createInfo.pQueueFamilyIndices = presentIndices.data();
//        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        Config::setConfig("SwapChainImageCount", imageCount);
        SingleAPP.setSwapChainCount(imageCount);

        if(swapChainSupport.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT){
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if(swapChainSupport.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT){
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));

        if (oldSwapchain != VK_NULL_HANDLE) {
            std::cout << "old swapchain destroy" << std::endl;
            vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
        }

        min_image_count = createInfo.minImageCount;

        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

        pre_offscreen_buffer.resize(imageCount);
        pre_shadow_buffer.resize(imageCount);
        pre_capture_buffers.resize(imageCount);

        m_presentFormat = surfaceFormat.format;
        presentExtent = extent;
        m_depthFormat = findDepthFormat();
        setDepthFormat(m_depthFormat);
    }

    void ShatterRender::createCaptureFramebuffers()
    {
        FrameBufferSpecification spec{};
        spec.Width = SingleAPP.getPresentViewPort().view.width;
        spec.Height = SingleAPP.getPresentViewPort().view.height;
        spec.formats = {{m_captureFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
                        {m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}};
        spec.Samples = 1;
        spec.SwapChainTarget = false;
        spec.RenderPass = m_captureRenderPass;

        m_captureFrameBuffer = FrameBuffer::createFramebuffer(spec);
    }

    void ShatterRender::createGraphicsCommandPool(){
        QueueFamilyIndices queueFamilyIndices = getIndices();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphic_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command Pool!");
        }
    }

    void ShatterRender::createComputeCommandPool() {
        QueueFamilyIndices queueFamilyIndices = getIndices();

        VkCommandPoolCreateInfo poolInfo = {};
        if(!Config::getConfig("enableMultipleComputeQueue"))
        {
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily;
        }else{
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        }

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &compute_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command Pool!");
        }
    }

    void ShatterRender::createTransferCommandPool(){
        QueueFamilyIndices queueFamilyIndices = getIndices();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &transfer_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command Pool!");
        }
    }

    void ShatterRender::createDescriptorPool(){
        VkDescriptorPoolSize pool_sizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        poolInfo.pPoolSizes = pool_sizes;

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor Pool!");
        }
    }

    uint32_t ShatterRender::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
        static bool set = false;
        static VkPhysicalDeviceMemoryProperties memProperties;
        if(!set)
        {
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
            setVkPhysicalDeviceMemoryProperties(memProperties);
            set = true;
        }
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void ShatterRender::createColorRenderPass() {
        std::array<VkAttachmentDescription,AttachmentCount> attachments{};
        {
            //Color attachment
            attachments[AttachmentBack].format = m_presentFormat;
            attachments[AttachmentBack].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentBack].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentBack].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[AttachmentBack].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentBack].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentBack].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentBack].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Position
            attachments[AttachmentPosition].format = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachments[AttachmentPosition].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentPosition].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentPosition].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentPosition].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentPosition].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentPosition].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentPosition].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Normals
            attachments[AttachmentNormal].format = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachments[AttachmentNormal].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentNormal].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentNormal].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentNormal].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentNormal].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentNormal].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentNormal].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Albedo
            attachments[AttachmentAlbedo].format = VK_FORMAT_R8G8B8A8_UNORM;
            attachments[AttachmentAlbedo].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentAlbedo].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentAlbedo].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentAlbedo].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentAlbedo].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentAlbedo].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentAlbedo].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            //Depth attachment
            attachments[AttachmentDepth].format = m_depthFormat;
            attachments[AttachmentDepth].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentDepth].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentDepth].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentDepth].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentDepth].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentDepth].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentDepth].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        std::array<VkSubpassDescription,3> subpassDescriptions{};

        // First subpass: Fill G-Buffer components
        // ----------------------------------------------------------------------------------------

        VkAttachmentReference colorReferences[4];
        colorReferences[0] = { AttachmentBack, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        colorReferences[1] = { AttachmentPosition, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        colorReferences[2] = { AttachmentNormal, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        colorReferences[3] = { AttachmentAlbedo, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        subpassDescriptions[SubpassG].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[SubpassG].colorAttachmentCount = 4;
        subpassDescriptions[SubpassG].pColorAttachments = colorReferences;
        subpassDescriptions[SubpassG].pDepthStencilAttachment = &depthReference;
        // Second subpass: Final composition (using G-Buffer components)
        // ----------------------------------------------------------------------------------------

        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        VkAttachmentReference inputReferences[3];
        inputReferences[0] = { AttachmentPosition, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        inputReferences[1] = { AttachmentNormal, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        inputReferences[2] = { AttachmentAlbedo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

        subpassDescriptions[SubpassLight].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[SubpassLight].colorAttachmentCount = 1;
        subpassDescriptions[SubpassLight].pColorAttachments = &colorReference;
        subpassDescriptions[SubpassLight].pDepthStencilAttachment = &depthReference;
        subpassDescriptions[SubpassLight].inputAttachmentCount = 3;
        subpassDescriptions[SubpassLight].pInputAttachments = inputReferences;

        subpassDescriptions[SubpassTransparency].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[SubpassTransparency].colorAttachmentCount = 1;
        subpassDescriptions[SubpassTransparency].pColorAttachments = &colorReference;
        subpassDescriptions[SubpassTransparency].pDepthStencilAttachment = &depthReference;
        subpassDescriptions[SubpassTransparency].inputAttachmentCount = 1;
        subpassDescriptions[SubpassTransparency].pInputAttachments = inputReferences;

        // Subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 4> dependencies;

        dependencies[ExternalToG].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[ExternalToG].dstSubpass = 0;
        dependencies[ExternalToG].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dependencies[ExternalToG].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[ExternalToG].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[ExternalToG].dstAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[ExternalToG].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // This dependency transitions the input attachment from color attachment to shader read
        dependencies[GtoLight].srcSubpass = 0;
        dependencies[GtoLight].dstSubpass = 1;
        dependencies[GtoLight].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[GtoLight].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[GtoLight].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[GtoLight].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[GtoLight].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[LightToTransparency].srcSubpass = 1;
        dependencies[LightToTransparency].dstSubpass = 2;
        dependencies[LightToTransparency].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[LightToTransparency].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[LightToTransparency].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[LightToTransparency].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[LightToTransparency].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[TransparencyToExternal].srcSubpass = 0;
        dependencies[TransparencyToExternal].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[TransparencyToExternal].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
        dependencies[TransparencyToExternal].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[TransparencyToExternal].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[TransparencyToExternal].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[TransparencyToExternal].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
        renderPassInfo.pSubpasses = subpassDescriptions.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_colorRenderPass));
    }

    void ShatterRender::createColorFramebuffers() {
        FrameBufferSpecification spec{};
        spec.Width = SingleAPP.getPresentViewPort().view.width;
        spec.Height = SingleAPP.getPresentViewPort().view.height;
        spec.formats = {
                {m_presentFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT},
                {VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT},
                {VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT},
                {VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT},
                {m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}
        };
        spec.Samples = 1;
        spec.SwapChainTarget = false;
        spec.RenderPass = m_colorRenderPass;

        colorFrameBuffers = FrameBuffer::createFramebuffer(spec);
    }

    void ShatterRender::createPresentRenderPass() {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_presentFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        std::array<VkSubpassDependency,2> dependency;
        dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency[0].dstSubpass = 0;
        dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT  |
                                     //                                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency[0].srcAccessMask = 0;
        dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependency[1].srcSubpass = 0;
        dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT  |
                                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ;
//                                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency[1].dstAccessMask = 0;
        dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

//        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkAttachmentDescription attachments = colorAttachment;
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachments;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependency.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_presentRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void ShatterRender::createPresentFramebuffers() {
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        m_presents.resize(imageCount);
        m_swapChainImageCount = imageCount;
        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
        VkSamplerCreateInfo samplerInfo = tool::samplerCreateInfo();

        for (int i = 0; i < imageCount; i++)
        {
            m_presents[i].image = images[i];

            m_presents[i].imageView = buffer::ShatterTexture::Create_ImageView(&device, images[i], m_presentFormat,
                                                                                VK_IMAGE_ASPECT_COLOR_BIT);
            vkCreateSampler(device, &samplerInfo, nullptr, &m_presents[i].sampler);

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_presentRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &m_presents[i].imageView;
            framebufferInfo.width = presentExtent.width;
            framebufferInfo.height = presentExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_presents[i].framebuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void ShatterRender::createCaptureRenderPass(){
        std::array<VkAttachmentDescription,2> attachments{};
        //Color attachment
        attachments[0].format = m_captureFormat;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // Position
        attachments[1].format = m_depthFormat;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpassDescriptions{};

        VkAttachmentReference colorReferences = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        subpassDescriptions.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions.colorAttachmentCount = 1;
        subpassDescriptions.pColorAttachments = &colorReferences;
        subpassDescriptions.pDepthStencilAttachment = &depthReference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_HOST_READ_BIT;
        dependencies[0].dstAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescriptions;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_captureRenderPass));
    }

    void ShatterRender::freeGraphicsPrimaryCB() {
        vkFreeCommandBuffers(device, graphic_commandPool, SingleAPP.getSwapChainCount(), presentCB.data());
        vkFreeCommandBuffers(device, graphic_commandPool, 1, &colorCB);
        vkFreeCommandBuffers(device, graphic_commandPool, 1, &captureCB);
    }

    void ShatterRender::allocateGraphicsPrimaryCB() {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
        commandBufferAllocateInfo.commandPool = graphic_commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        commandBufferAllocateInfo.commandBufferCount = m_swapChainImageCount;
        presentCB.resize(m_swapChainImageCount);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, presentCB.data()));

        commandBufferAllocateInfo.commandBufferCount = 1;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &colorCB));
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &captureCB));
    }

    void ShatterRender::allocateComputePrimaryCB() {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.commandPool = compute_commandPool;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &computeCB));
    }

    void ShatterRender::prepareMultipleThreadDate() {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.pNext = VK_NULL_HANDLE;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmdPoolInfo.queueFamilyIndex = getIndices().graphicsFamily;
        /*
         * Init compute multiple thread data
         */
        cmdPoolInfo.queueFamilyIndex = getIndices().computeFamily;
        auto threadCommandPool = getThreadCommandPool();
        threadCommandPool->resize(SingleThreadPool->m_thread_count);
        for(auto& pool: *threadCommandPool)
        {
            VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &pool.computePool));
        }

        cmdPoolInfo.queueFamilyIndex = getIndices().graphicsFamily;
        for(auto& pool: *threadCommandPool)
        {
            VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &pool.graphicsPool));
        }
    }

    void ShatterRender::updateOffscreenBufferAsync(VkCommandBuffer _cb,int _imageIndex) const{
        SingleOffScreen.beginRenderPass(_cb, true);

        if(!offdrawid_vec.empty())
        {
            vkCmdExecuteCommands(_cb, offdrawid_vec.size(), pre_offscreen_buffer[_imageIndex].data());

//            vkCmdExecuteCommands(_cb,offdrawid_vec.size(),&pre_offscreen_buffers[offdrawid_vec.size() * _imageIndex]);
        }
        SingleOffScreen.endRenderPass(_cb);
    }

    void ShatterRender::createOffScreenBuffers(VkCommandBuffer _cb, int _imageIndex){
        auto threadCommandPool = getThreadCommandPool();
        SingleOffScreen.beginRenderPass(_cb, true);
        auto threadPool = ThreadPool::pool();
        int draw_index = 0;
        std::vector<VkCommandBuffer> commandBuffers(offdrawid_vec.size());
        int threadIndex = 0;
        for(size_t off_index = 0; off_index < offdrawid_vec.size(); off_index++)
        {
            (*threadPool)[threadIndex]->addTask([&, threadIndex, off_index] {
                VkCommandBuffer* cb = &commandBuffers[off_index];
                ObjectTask::newGraphicsTask(threadIndex, offdrawid_vec[off_index], SingleOffScreen.m_inherit_info, cb);
            });
            if(threadIndex >= threadPool->m_thread_count){
                threadIndex -= threadPool->m_thread_count;
            }
        }
        threadPool->wait();
        if(!commandBuffers.empty())
        {
            vkCmdExecuteCommands(_cb, commandBuffers.size(), commandBuffers.data());
        }
        pre_offscreen_buffer[_imageIndex].clear();
        pre_offscreen_buffer[_imageIndex].insert(pre_offscreen_buffer[_imageIndex].end(),commandBuffers.begin(),commandBuffers.end());
        SingleOffScreen.endRenderPass(_cb);
    }

    void ShatterRender::createShadowGraphicsBuffers(VkCommandBuffer _cb, int _imageIndex){
        auto threadObjectPool = getThreadCommandPool();
        size_t draw_index;
        auto threadPool = ThreadPool::pool();
        auto dPool = SingleDPool;
        for(size_t index = 0; index < SHADOW_MAP_CASCADE_COUNT; index++)
        {
            SingleCascade.beginRenderPass(_cb, index,true);

            draw_index = 0;
            std::vector<VkCommandBuffer> depthBuffer(drawIdVec.size());
            for (int t = 0; t < std::thread::hardware_concurrency(); t++)
            {
                for (int j = 0; j < numObjectsPerThread; j++)
                {
                    if(draw_index >= drawIdVec.size()) break;
                    (*threadPool).threads[t]->addTask([&,draw_index] {
                        VkCommandBuffer *cb = &depthBuffer[draw_index];
                        ObjectTask::shadowDepthTask(t,drawIdVec[draw_index],SingleCascade.inheritInfo,cb);
                    });
                    draw_index++;
                }
                if(draw_index >= drawIdVec.size()) break;
            }
            (*threadPool).wait();
            vkCmdExecuteCommands(_cb, depthBuffer.size(), depthBuffer.data());
//            pre_shadow_buffers.insert(pre_shadow_buffers.end(),depthBuffer.begin(),depthBuffer.end());
            pre_shadow_buffer[_imageIndex].clear();
            pre_shadow_buffer[_imageIndex].insert(pre_shadow_buffer[_imageIndex].end(), depthBuffer.begin(), depthBuffer.end());
            SingleCascade.endRenderPass(_cb);
        }
    }

    void ShatterRender::updateShadowGraphicsAsync(VkCommandBuffer _cb,int _imageIndex) {
        auto threadObjectPool = getThreadCommandPool();
        size_t draw_index;
        auto threadPool = ThreadPool::pool();
        auto dPool = SingleDPool;
        auto begin = pre_shadow_buffer[_imageIndex].begin();
        auto end = pre_shadow_buffer[_imageIndex].begin();
        std::advance(end,drawIdVec.size());
        for(size_t index = 0; index < SHADOW_MAP_CASCADE_COUNT; index++)
        {
            SingleCascade.beginRenderPass(_cb, index,true);
            std::vector<VkCommandBuffer> depthBuffer(begin,end);
            vkCmdExecuteCommands(_cb, depthBuffer.size(), depthBuffer.data());
            std::advance(begin , drawIdVec.size());
            std::advance(end , drawIdVec.size());
            SingleCascade.endRenderPass(_cb);
        }
    }

    void ShatterRender::createSecondaryCommandBuffers()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = graphic_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        allocInfo.commandBufferCount = m_swapChainImageCount;
        gui_buffer.resize(m_swapChainImageCount);

        if (vkAllocateCommandBuffers(device, &allocInfo, gui_buffer.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate gui command buffer!");
        }

        offscreen_buffers.resize(m_swapChainImageCount);
        if (vkAllocateCommandBuffers(device, &allocInfo, offscreen_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate off screen command buffers!");
        }
    }

    void ShatterRender::cleanupObject()
    {
        pre_compute_buffers.clear();

        SingleDPool->free(offdrawid_vec);
        offdrawid_vec.clear();
        SingleDPool->free(drawIdVec());
        drawIdVec.clear();
        SingleCPool->free(computeid_vec);
        computeid_vec.clear();
    }

    void ShatterRender::createComputeCommandBuffersMultiple()
    {
        std::vector<VkCommandBuffer> commandBuffers(computeid_vec.size());

        VkCommandBufferBeginInfo cmdBufInfo{};
        {
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.pNext = VK_NULL_HANDLE;
            cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmdBufInfo.pInheritanceInfo = VK_NULL_HANDLE;
        }

        VK_CHECK_RESULT(vkBeginCommandBuffer(computeCB, &cmdBufInfo));
        auto threadPool = ThreadPool::pool();
        // Inheritance info for the secondary command buffers
        VkCommandBufferInheritanceInfo inheritanceInfo {};
        {
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.pNext = VK_NULL_HANDLE;
            inheritanceInfo.renderPass = VK_NULL_HANDLE;
            inheritanceInfo.subpass = 0;
            inheritanceInfo.framebuffer = VK_NULL_HANDLE;
            inheritanceInfo.occlusionQueryEnable = false;
        }

//        TaskPool::computeBarrierRequire(compute_buffer);

        int draw_index = 0;

        for(size_t compute_index = 0; compute_index < computeid_vec.size(); compute_index++)
        {
            threadPool->addTask([&,compute_index] {
                VkCommandBuffer* cb = &commandBuffers[compute_index];
                ObjectTask::computeTask(computeid_vec[compute_index], inheritanceInfo, cb);
            });
        }

        (*threadPool).wait();

        pre_compute_buffers = commandBuffers;

        // Execute render commands from the secondary command buffer
        vkCmdExecuteCommands(computeCB, commandBuffers.size(), commandBuffers.data());

//        TaskPool::computeBarrierRelease(compute_buffer);

        VK_CHECK_RESULT(vkEndCommandBuffer(computeCB));
    }

    void ShatterRender::updateComputeCommandBufferAsync()
    {
        std::vector<VkCommandBuffer> commandBuffers(computeid_vec.size());

        VkCommandBufferBeginInfo cmdBufInfo{};

        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufInfo.pNext = VK_NULL_HANDLE;
        cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        cmdBufInfo.pInheritanceInfo = VK_NULL_HANDLE;

//        vkQueueWaitIdle(compute_queue);
        VK_CHECK_RESULT(vkBeginCommandBuffer(computeCB, &cmdBufInfo));
        if(!computeid_vec.empty())
        {
            vkCmdExecuteCommands(computeCB, computeid_vec.size(), pre_compute_buffers.data());
        }

        VK_CHECK_RESULT(vkEndCommandBuffer(computeCB));
    }

    void ShatterRender::createNewCaptureCommandBuffers() {
        VkCommandBufferBeginInfo cmdBufInfo{};
        {
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.pNext = VK_NULL_HANDLE;
            cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmdBufInfo.pInheritanceInfo = VK_NULL_HANDLE;
        }

        if (SingleAPP.viewportChanged) {
            ((VulkanFrameBuffer*)m_captureFrameBuffer)->resize(SingleAPP.getPresentViewPort().view.width, SingleAPP.getPresentViewPort().view.height);
        }

        vkBeginCommandBuffer(captureCB, &cmdBufInfo);

        std::array<VkClearValue,2> clearCaptureValue{};
        clearCaptureValue[0].color = { { uint32_t(0) } };
        clearCaptureValue[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        {
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = VK_NULL_HANDLE;
            renderPassBeginInfo.renderPass = m_captureRenderPass;
            renderPassBeginInfo.framebuffer = ((VulkanFrameBuffer*)m_captureFrameBuffer)->m_frame_buffer;
            renderPassBeginInfo.renderArea.offset = {0, 0};
            renderPassBeginInfo.renderArea.extent = SingleAPP.getPresentViewPort().scissor.extent;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearCaptureValue.data();
        }
//        TaskPool::captureBarrierRequire(m_capture_buffer);
        vkCmdBeginRenderPass(captureCB, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        VkDeviceSize offsets = 0;
        auto set_pool = MPool<VkDescriptorSet>::getPool();
        std::vector<VkCommandBuffer> captureBuffers(aabb_map.size());
        int index = 0;
        VkCommandBufferInheritanceInfo inheritanceInfo{};
        {
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.pNext = VK_NULL_HANDLE;
            inheritanceInfo.renderPass = m_captureRenderPass;
            inheritanceInfo.subpass = 0;
            inheritanceInfo.framebuffer = ((VulkanFrameBuffer*)m_captureFrameBuffer)->m_frame_buffer;
            inheritanceInfo.occlusionQueryEnable = false;
        }

        VkCommandBufferBeginInfo commandBufferBeginInfo {};
        {
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
        }
        VkViewport& viewport = SingleAPP.getPresentViewPort()();
        VkRect2D& scissor = SingleAPP.getPresentViewPort().scissor;

        int threadIndex = 0;
        for(auto& pair: aabb_map)
        {
            int boxId =  pair.second;
            int captureIndex =  pair.first;
            (*SingleThreadPool)[threadIndex]->addTask([&, boxId, threadIndex, captureIndex, index](){
                VkCommandPool pool = getCommandPool(CommandPoolType::GraphicsPool, threadIndex);
                VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
                commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
                commandBufferAllocateInfo.commandPool = pool;
                commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                commandBufferAllocateInfo.commandBufferCount = 1;
                vkAllocateCommandBuffers(SingleDevice.logicalDevice, &commandBufferAllocateInfo, &captureBuffers[index]);

                vkBeginCommandBuffer(captureBuffers[index], &commandBufferBeginInfo);
                vkCmdSetViewport(captureBuffers[index], 0, 1, &viewport);
                vkCmdSetScissor(captureBuffers[index], 0, 1, &scissor);
                vkCmdBindPipeline(captureBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, (*SinglePPool["AABBCapture"])());
                int model_index = (*SingleBoxPool)[boxId]->m_model_index;
                ShatterBuffer* buffer = SingleBPool.getBuffer(tool::combine("Capture", captureIndex), Buffer_Type::Vertex_Buffer);
                vkCmdBindVertexBuffers(captureBuffers[index], 0, 1, &buffer->m_buffer, &offsets);
                std::array<VkDescriptorSet, 3> set_array{};
                set_array[0] = *(*set_pool)[model_index];
                set_array[1] = SingleSetPool["Camera"];
                set_array[2] = (*SingleBoxPool)[boxId]->m_capture_set;
                vkCmdBindDescriptorSets(captureBuffers[index],
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        (*SinglePPool["AABBCapture"]).getPipelineLayout(),
                                        0,
                                        3,
                                        set_array.data(),
                                        0,
                                        nullptr
                );
                vkCmdDraw(captureBuffers[index], CaptureBoxVertexCount, 1, 0, 0);
                vkEndCommandBuffer(captureBuffers[index]);
            });
            if(threadIndex >= SingleThreadPool->m_thread_count){
                threadIndex -= SingleThreadPool->m_thread_count;
            }
            index++;
        }
        SingleThreadPool->wait();
        if(!captureBuffers.empty()) {
            vkCmdExecuteCommands(captureCB, captureBuffers.size(), captureBuffers.data());
        }
        vkCmdEndRenderPass(captureCB);
        vkEndCommandBuffer(captureCB);
    }

    void ShatterRender::createColorGraphicsCommandBuffersMultiple() {
        exchangeObjects();
        VkCommandBufferBeginInfo cmdBufInfo{};
        {
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.pNext = VK_NULL_HANDLE;
            cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmdBufInfo.pInheritanceInfo = VK_NULL_HANDLE;
        }

        VkRenderPassBeginInfo renderPassBeginInfo{};
        {
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = VK_NULL_HANDLE;
            renderPassBeginInfo.renderPass = m_colorRenderPass;
            renderPassBeginInfo.framebuffer = ((VulkanFrameBuffer*)colorFrameBuffers)->get();
            renderPassBeginInfo.renderArea.offset = {0, 0};
            renderPassBeginInfo.renderArea.extent = SingleAPP.getPresentViewPort().scissor.extent;
            renderPassBeginInfo.clearValueCount = AttachmentCount;
            renderPassBeginInfo.pClearValues = clearValues.data();
        }

        if (SingleAPP.viewportChanged) {
            ((VulkanFrameBuffer*)colorFrameBuffers)->resize(SingleAPP.getPresentViewPort().view.width, SingleAPP.getPresentViewPort().view.height);
            updateColorSet();
        }

        vkBeginCommandBuffer(colorCB, &cmdBufInfo);
        auto threadPool = ThreadPool::pool();
        // Inheritance info for the secondary command buffers
        VkCommandBufferInheritanceInfo inheritanceInfo {};
        {
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.pNext = VK_NULL_HANDLE;
            inheritanceInfo.renderPass = m_colorRenderPass;
            inheritanceInfo.subpass = 0;
            inheritanceInfo.framebuffer = ((VulkanFrameBuffer*)colorFrameBuffers)->get();
            inheritanceInfo.occlusionQueryEnable = false;
        }

        auto dPool = SingleDPool;
        /*
         * Use barrier to changed offscreen and shadow image layout to shader resource
         */
        TaskPool::barrierRequire(colorCB);

        vkCmdBeginRenderPass(colorCB, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        int draw_index = 0;
        int threadIndex = 0;

        /*
         * G
         */
        {
            draw_index = 0;
            std::vector<VkCommandBuffer> gBuffers(drawIdVec.size());
            inheritanceInfo.subpass = SubpassG;
            threadIndex = 0;
            for(size_t d_index = 0; d_index < drawIdVec.size(); d_index++)
            {
                (*threadPool)[threadIndex]->addTask([&, d_index, threadIndex](){
                    VkCommandBuffer* cb = &gBuffers[d_index];
                    ObjectTask::gTask(threadIndex, drawIdVec[d_index], inheritanceInfo, cb);
                });
                if( ++threadIndex >= threadPool->m_thread_count){
                    threadIndex -= threadPool->m_thread_count;
                }
            }
            (*threadPool).wait();
            vkCmdExecuteCommands(colorCB, gBuffers.size(), gBuffers.data());
        }
        vkCmdNextSubpass(colorCB, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        /*
         * Light
         */
        {
            inheritanceInfo.subpass = SubpassLight;

            VkCommandBuffer compositeCB;
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = graphic_commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = 1;
            vkAllocateCommandBuffers(device, &allocInfo, &compositeCB);


            VkCommandBufferBeginInfo commandBufferBeginInfo {};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

            vkBeginCommandBuffer(compositeCB, &commandBufferBeginInfo);
            UnionViewPort& tmp = SingleAPP.getPresentViewPort();
            vkCmdSetViewport(compositeCB, 0, 1, &tmp.view);
            VkRect2D& scissor = tmp.scissor;
            vkCmdSetScissor(compositeCB,0,1,&scissor);

            std::array<VkDescriptorSet,2> sets{};
            {
                sets[0] = SingleSetPool["gBuffer"];
                sets[1] = SingleSetPool["MultiLight"];
            }
            vkCmdBindDescriptorSets(compositeCB, VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Composition"]->getPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
            vkCmdBindPipeline(compositeCB, VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Composition"]->getPipeline());
            vkCmdDraw(compositeCB, 3, 1, 0, 0);
            vkEndCommandBuffer(compositeCB);

            vkCmdExecuteCommands(colorCB, 1, &compositeCB);
        }
        vkCmdNextSubpass(colorCB, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        inheritanceInfo.subpass = SubpassTransparency;

        /*
         * normal object pass
         */
        {
            if(!normalIdVec.empty())
            {
                std::vector<VkCommandBuffer> normalBuffers(normalIdVec.size());
                threadIndex = 0;
                for(size_t normal_index = 0; normal_index < normalIdVec.size(); normal_index++)
                {
                    (*threadPool)[threadIndex]->addTask([&, normal_index, threadIndex](){
                        VkCommandBuffer* cb = &normalBuffers[normal_index];
                        ObjectTask::newGraphicsTask(threadIndex, normalIdVec[normal_index], inheritanceInfo, cb);
                    });
                    if( ++threadIndex >= threadPool->m_thread_count){
                        threadIndex -= threadPool->m_thread_count;
                    }
                }
                (*threadPool).wait();
                vkCmdExecuteCommands(colorCB, normalBuffers.size(), normalBuffers.data());
            }
        }

        /*
         * transparency pass
         */
        {
            if(!transparency_vec.empty())
            {
                draw_index = 0;
                std::vector<VkCommandBuffer> transparencyBuffers(transparency_vec.size());
                inheritanceInfo.subpass = SubpassTransparency;
                threadIndex = 0;

                for(size_t transparent_index = 0; transparent_index < transparency_vec.size(); transparent_index++)
                {
                    (*threadPool)[threadIndex]->addTask([&, transparent_index, threadIndex](){
                        VkCommandBuffer* cb = &transparencyBuffers[transparent_index];
                        ObjectTask::newGraphicsTask(threadIndex, transparency_vec[transparent_index], inheritanceInfo, cb);
                    });
                    if( ++threadIndex >= threadPool->m_thread_count){
                        threadIndex -= threadPool->m_thread_count;
                    }
                }
                (*threadPool).wait();
                vkCmdExecuteCommands(colorCB, transparencyBuffers.size(), transparencyBuffers.data());
            }
        }

//        std::thread offscreen([&](bool config){
//            if(config)
//            {
//                VkCommandBufferBeginInfo commandBufferBeginInfo {};
//                commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//                commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
//                commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
//                commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
//
//                vkBeginCommandBuffer(offscreen_buffers[i], &commandBufferBeginInfo);
//                UnionViewPort& tmp = getViewPort();
//                vkCmdSetViewport(offscreen_buffers[i], 0, 1, &tmp.view);
//
//                VkRect2D& scissor = getScissor();
//                vkCmdSetScissor(offscreen_buffers[i],0,1,&scissor);
//                VkDescriptorSet tmp_set = SingleSetPool["OffScreen"];
//                vkCmdBindDescriptorSets(offscreen_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Quad"]->getPipelineLayout(), 0, 1, &tmp_set, 0, nullptr);
//                vkCmdBindPipeline(offscreen_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Quad"]->getPipeline());
//                vkCmdDraw(offscreen_buffers[i], 4, 1, 0, 0);
//                vkEndCommandBuffer(offscreen_buffers[i]);
//            }
//        },Config::getConfig("enableOffscreenDebug"));


//        offscreen.join();
//        if(Config::getConfig("enableOffscreenDebug"))
//        {
//            commandBuffers.push_back(offscreen_buffers[i]);
//        }

//            std::cout << "CommandBuffer: " << m_colorCommandBuffer << std::endl;

        vkCmdEndRenderPass(colorCB);

        TaskPool::barrierRelease(colorCB);

        vkEndCommandBuffer(colorCB);
    }

    void ShatterRender::createPresentGraphicsCommandBuffers() {
        std::array<VkClearValue,2> clearPresentValue{};
        clearPresentValue[0].color = { { 0.92f, 0.92f, 0.92f, 1.0f }  };
        clearPresentValue[1].depthStencil = { 1.0f, 0 };
        if(Config::getConfig("enableScreenGui"))
        {
            imGui->newFrame(false);
            imGui->updateBuffers();
        }
        for (size_t i = 0; i < presentCB.size(); i++) {
            VkCommandBufferBeginInfo cmdBufInfo{};
            {
                cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                cmdBufInfo.pNext = VK_NULL_HANDLE;
                cmdBufInfo.flags =
                        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                cmdBufInfo.pInheritanceInfo = VK_NULL_HANDLE;
            }

            VkRenderPassBeginInfo renderPassBeginInfo{};
            {
                renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassBeginInfo.pNext = VK_NULL_HANDLE;
                renderPassBeginInfo.renderPass = m_presentRenderPass;
                renderPassBeginInfo.framebuffer = m_presents[i].framebuffer;
                renderPassBeginInfo.renderArea.offset = {0, 0};
                renderPassBeginInfo.renderArea.extent = presentExtent;
                renderPassBeginInfo.clearValueCount = 2;
                renderPassBeginInfo.pClearValues = clearPresentValue.data();
            }

            vkBeginCommandBuffer(presentCB[i], &cmdBufInfo);
            auto threadPool = ThreadPool::pool();

            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.layerCount = 1;

            {
                VkImageMemoryBarrier imageMemoryBarrier {};
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageMemoryBarrier.image = ((VulkanFrameBuffer*)colorFrameBuffers)->m_attachments[0].image;
                imageMemoryBarrier.subresourceRange = subresourceRange;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                // Put barrier inside setup command buffer
                vkCmdPipelineBarrier(
                        presentCB[i],
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &imageMemoryBarrier);
            }

            vkCmdBeginRenderPass(presentCB[i], &renderPassBeginInfo,
                                 VK_SUBPASS_CONTENTS_INLINE);

//            UnionViewPort& tmp = getWindowViewPort();
//            vkCmdSetViewport(new_graphics_buffer[i], 0, 1, &tmp.view);
//            VkRect2D& scissor = getWindowScissor();
//            vkCmdSetScissor(new_graphics_buffer[i],0,1, &scissor);
//
//            vkCmdBindDescriptorSets(new_graphics_buffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Present"]->getPipelineLayout(), 0, 1, &m_colorSet, 0, nullptr);
//            vkCmdBindPipeline(new_graphics_buffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Present"]->getPipeline());
//            vkCmdDraw(new_graphics_buffer[i], 3, 1, 0, 0);

            if (Config::getConfig("enableScreenGui")) {
                imGui->drawFrame(presentCB[i]);
            }

            vkCmdEndRenderPass(presentCB[i]);

            {
                VkImageMemoryBarrier imageMemoryBarrier {};
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                imageMemoryBarrier.image = ((VulkanFrameBuffer*)colorFrameBuffers)->m_attachments[0].image;
                imageMemoryBarrier.subresourceRange = subresourceRange;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                // Put barrier inside setup command buffer
                vkCmdPipelineBarrier(
                        presentCB[i],
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &imageMemoryBarrier);
            }

            vkEndCommandBuffer(presentCB[i]);
        }
    }

    void ShatterRender::updatePresentCommandBuffers(int _index) {
        std::array<VkClearValue,2> clearPresentValue{};
        clearPresentValue[0].color = { { 0.92f, 0.92f, 0.92f, 1.0f }  };
        clearPresentValue[1].depthStencil = { 1.0f, 0 };
        VkCommandBufferBeginInfo cmdBufInfo{};
        {
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.pNext = VK_NULL_HANDLE;
            cmdBufInfo.flags =
                    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            cmdBufInfo.pInheritanceInfo = VK_NULL_HANDLE;
        }

        VkRenderPassBeginInfo renderPassBeginInfo{};
        {
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = VK_NULL_HANDLE;
            renderPassBeginInfo.renderPass = m_presentRenderPass;
            renderPassBeginInfo.framebuffer = m_presents[_index].framebuffer;
            renderPassBeginInfo.renderArea.offset = {0, 0};
            renderPassBeginInfo.renderArea.extent = presentExtent;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearPresentValue.data();
        }

        vkQueueWaitIdle(graphics_queue);

        vkBeginCommandBuffer(presentCB[_index], &cmdBufInfo);
        auto threadPool = ThreadPool::pool();

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        {
            VkImageMemoryBarrier imageMemoryBarrier {};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.image = ((VulkanFrameBuffer*)colorFrameBuffers)->m_attachments[0].image;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            // Put barrier inside setup command buffer
            vkCmdPipelineBarrier(
                    presentCB[_index],
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);
        }

        vkCmdBeginRenderPass(presentCB[_index], &renderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);


//        UnionViewPort& tmp = getWindowViewPort();
//        vkCmdSetViewport(new_graphics_buffer[_index], 0, 1, &tmp.view);
//        VkRect2D& scissor = getWindowScissor();
//        vkCmdSetScissor(new_graphics_buffer[_index],0,1, &scissor);
//
//        vkCmdBindDescriptorSets(new_graphics_buffer[_index], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Present"]->getPipelineLayout(), 0, 1, &m_colorSet, 0, nullptr);
//        vkCmdBindPipeline(new_graphics_buffer[_index], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Present"]->getPipeline());
//        vkCmdDraw(new_graphics_buffer[_index], 3, 1, 0, 0);

        if (Config::getConfig("enableScreenGui")) {
            imGui->drawFrame(presentCB[_index]);
        }

        vkCmdEndRenderPass(presentCB[_index]);

        {
            VkImageMemoryBarrier imageMemoryBarrier {};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            imageMemoryBarrier.image = ((VulkanFrameBuffer*)colorFrameBuffers)->m_attachments[0].image;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            // Put barrier inside setup command buffer
            vkCmdPipelineBarrier(
                    presentCB[_index],
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);
        }

        vkEndCommandBuffer(presentCB[_index]);
    }

    void ShatterRender::createGraphicsCommandBuffers() {
        createNewCaptureCommandBuffers();
        createColorGraphicsCommandBuffersMultiple();
        createPresentGraphicsCommandBuffers();
    }

    void ShatterRender::createComputeCommandBuffer(){
        // build command buffer
        VkCommandBufferBeginInfo command_buffer_begin_info
        {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                nullptr,
                VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                nullptr
        };
        if (vkBeginCommandBuffer(computeCB, &command_buffer_begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("command buffer begin failed");
        }

        auto cpool = MPool<CObject>::getPool();
        for(auto& computeobj : computeid_vec){
            (*cpool)[computeobj]->compute(computeCB);
        }

        vkEndCommandBuffer(computeCB);
    }

    void ShatterRender::createSemaphores(){
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeFinishedSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeReadySemaphore) != VK_SUCCESS ) {

            throw std::runtime_error("failed to create semaphores!");
        }

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence);
    }

    void ShatterRender::recreateSwapChain(){
        vkDeviceWaitIdle(device);
        cleanupSwapChain();
        createSwapChain();
//        createRenderPass();
//        createDepthResources();
//        createFramebuffers();
        createCaptureFramebuffers();

        {
            SinglePPool.release();
            SinglePPool.init();
        }
        allocateGraphicsPrimaryCB();
        allocateComputePrimaryCB();
        createComputeCommandBuffer();
        createSecondaryCommandBuffers();
//        createGraphicsCommandBuffers();
//        createGraphicsCommandBuffersMultiple();
        vkDeviceWaitIdle(device);
    }

    void ShatterRender::cleanupSwapChain(){
        {
            for (auto & present : m_presents) {
//                vkDestroyImage(device, present.image, nullptr);
                vkDestroyImageView(device, present.imageView, nullptr);
                vkDestroySampler(device, present.sampler, nullptr);
                vkDestroyFramebuffer(device, present.framebuffer, nullptr);
            }
        }
    }

    void ShatterRender::newDraw() {

        uint32_t imageIndex;
        auto requireImageResult = vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if((requireImageResult == VK_ERROR_OUT_OF_DATE_KHR) || (requireImageResult == VK_SUBOPTIMAL_KHR)){
            windowResize();
        }
        setSwapChainIndex(int(imageIndex));
        std::array<VkCommandBuffer, 3> commands{SingleRender.colorCB, SingleRender.captureCB, SingleRender.presentCB[imageIndex]};

        static VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        graphicsSubmitInfo = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,
                nullptr,
                1,
                &imageAvailableSemaphore,
                waitStages,
                3,
                commands.data(),
                1,
                &renderFinishedSemaphore
        };

        VkResult fenceRes;
        fenceRes = vkWaitForFences(SingleDevice(), 1, &renderFence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        fenceRes = vkResetFences(SingleDevice(), 1, &renderFence);
        VK_CHECK_RESULT(fenceRes);
//        vkResetFences(SingleDevice(), 1, &SingleRender.renderFence);

        if (guiChanged || offChanged || drawChanged || normalChanged || transChanged || aabbChanged || SingleAPP.viewportChanged || SingleRender.resized)
        {
            createGraphicsCommandBuffers();
            guiChanged = offChanged = drawChanged = normalChanged = transChanged = aabbChanged =
                    SingleAPP.viewportChanged = SingleRender.resized =  false;
        } else if (Config::getConfig("enableScreenGui")) {
            if (Config::getConfig("enableScreenGui"))
            {
                imGui->newFrame(false);
                imGui->updateBuffers();
            }
//            if (GUI::getGUI()->getItemState() || SingleAPP.presentReset) {
            updatePresentCommandBuffers((int)imageIndex);
//                SingleAPP.presentReset = false;
//            }
        }

        VK_CHECK_RESULT(vkQueueSubmit(graphics_queue, 1, &graphicsSubmitInfo, renderFence));

        presentInfo = {
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                nullptr,
                1,
                &renderFinishedSemaphore,
                1,
                &swapchain,
                &imageIndex,
                nullptr
        };
        auto result = vkQueuePresentKHR(present_queue, &presentInfo);
        if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                windowResize();
                return;
            } else {
                VK_CHECK_RESULT(result);
            }
        }
        vkQueueWaitIdle(graphics_queue);
        vkQueueWaitIdle(transfer_queue);
    }

    SwapChainSupportDetails ShatterRender::querySwapChainSupport(VkPhysicalDevice device){
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                  surface,
                                                  &presentModeCount,
                                                  nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                      surface,
                                                      &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    VkFormat ShatterRender::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                                VkFormatFeatureFlags features){
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }

            throw std::runtime_error("failed to find supported format1");
        }
        return VkFormat{};
    }

    VkFormat ShatterRender::findDepthFormat(){
        return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool ShatterRender::isDeviceSuitable(VkPhysicalDevice device){
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        setDeviceFeatures(supportedFeatures);
//        printDeviceFeatures();

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device,&deviceProperties);

//
//        VkPhysicalDeviceProperties2 deviceProperties2;
//        (*vkGetPhysicalDeviceProperties2)(device,&deviceProperties2);

        bool val = indices.isComplete() &&
                   extensionsSupported &&
                   supportedFeatures.samplerAnisotropy &&
                   deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        if(val)
        {
            uint32_t count;
            VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts & deviceProperties.limits.framebufferDepthSampleCounts;
            count = VK_SAMPLE_COUNT_1_BIT;
            if (counts & VK_SAMPLE_COUNT_2_BIT) { count = VK_SAMPLE_COUNT_2_BIT; }
            if (counts & VK_SAMPLE_COUNT_4_BIT) { count = VK_SAMPLE_COUNT_4_BIT; }
            if (counts & VK_SAMPLE_COUNT_8_BIT) { count = VK_SAMPLE_COUNT_8_BIT; }
            if (counts & VK_SAMPLE_COUNT_16_BIT) { count = VK_SAMPLE_COUNT_16_BIT; }
            if (counts & VK_SAMPLE_COUNT_32_BIT) { count = VK_SAMPLE_COUNT_32_BIT; }
            if (counts & VK_SAMPLE_COUNT_64_BIT) { count = VK_SAMPLE_COUNT_64_BIT; }
            Config::setConfig("FramebufferSampleCount", count);

            setIndices(indices);
        }

        return val;
    }

    bool ShatterRender::checkDeviceExtensionSupport(VkPhysicalDevice device){
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        for(auto &i : availableExtensions)
        {
            std::cout << i.extensionName << std::endl;
        }

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices ShatterRender::findQueueFamilies(VkPhysicalDevice device){
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && indices.graphicsFamily == -1) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (queueFamily.queueCount > 0 && presentSupport && i != indices.graphicsFamily) {
                indices.presentFamily = i;
            }else if(queueFamily.queueCount > 0 && presentSupport && indices.presentFamily == -1){
                indices.presentFamily = i;
            }

            if(queueFamily.queueCount > 0 && queueFamily.queueFlags &
            VK_QUEUE_COMPUTE_BIT && (i != indices.presentFamily || i != indices.graphicsFamily)){
                indices.computeFamily = i;
            }
            else if(queueFamily.queueCount > 0 && queueFamily.queueFlags &VK_QUEUE_COMPUTE_BIT && indices.computeFamily == -1)
            {
                indices.computeFamily = i;
            }

            if(queueFamily.queueCount > 0 && queueFamily.queueFlags &
            VK_QUEUE_TRANSFER_BIT && (i != indices.graphicsFamily || i != indices.computeFamily)){
                indices.transferFamily = i;
            }
            else if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && indices.computeFamily == -1)
            {
                indices.computeFamily = i;
            }
            i++;
        }

        return indices;
    }

    std::vector<const char *> ShatterRender::getRequiredExtensions(){
        std::vector<const char *> extensions;

        unsigned int glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (unsigned int i = 0; i < glfwExtensionCount; i++) {
            extensions.push_back(glfwExtensions[i]);
        }

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
//            extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
//            extensions.push_back(VK_NV_MESH_SHADER_EXTENSION_NAME);
        }

        return extensions;
    }

    VkCommandBuffer ShatterRender::beginSingleTimeCommands(){
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = graphic_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    VkCommandBuffer ShatterRender::beginSingleTimeTransCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = transfer_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void ShatterRender::endSingleTimeTransCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(transfer_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(transfer_queue);

        vkFreeCommandBuffers(device, transfer_commandPool, 1, &commandBuffer);
    }


    void ShatterRender::endSingleTimeCommands(VkCommandBuffer commandBuffer){
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphics_queue);

        vkFreeCommandBuffers(device, graphic_commandPool, 1, &commandBuffer);
    }

    bool ShatterRender::checkValidationLayerSupport(){
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const auto &layer : availableLayers) {
            std::cout << std::string("availableLayers: ") + layer.layerName <<std::endl;
        }

        for (const char *layerName : validationLayers) {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void ShatterRender::createCommandBuffer(){
        if (Config::getConfig("enableMultipleComputeQueue"))
        {
            createComputeCommandBuffersMultiple();
        } else {
            createComputeCommandBuffer();
        }
        createSecondaryCommandBuffers();
        createGraphicsCommandBuffers();
//        createGraphicsCommandBuffersMultiple();
    }

    void ShatterRender::exchangeObjects()
    {
        drawIdVec.flush();
        normalIdVec.flush();
    }

    void ShatterRender::pushDObjects(int _element)
    {
        drawIdVec.push_back(_element);
    }

    void ShatterRender::pushCObjects(int _element)
    {
        computeid_vec.push_back(_element);
    }

    void ShatterRender::pushTObjects(int _element)
    {
        transparency_vec.push_back(_element);
    }

    void ShatterRender::pushNObjects(int _element)
    {
        normalIdVec.push_back(_element);
    }

    void ShatterRender::pushOObjects(int _element)
    {
        offdrawid_vec.push_back(_element);
    }

    void ShatterRender::releaseObject(int _id, DrawObjectType _type)
    {
        switch (_type) {
            case DrawObjectType::Default:
            {
                drawIdVec.erase(_id);
                break;
            }
            case DrawObjectType::OffScreen:
            {
                auto iterator = std::find(offdrawid_vec.begin(), offdrawid_vec.end(), _id);
                if(iterator != offdrawid_vec.end())
                {
                    offdrawid_vec.erase(iterator);
                }
                break;
            }
            case DrawObjectType::Transparency:
            {
                auto iterator = std::find(transparency_vec.begin(), transparency_vec.end(), _id);
                if(iterator != transparency_vec.end())
                {
                    transparency_vec.erase(iterator);
                }
                break;
            }
            case DrawObjectType::Normal:
            {
                normalIdVec.erase(_id);
                break;
            }
            case DrawObjectType::AABB:{
                if(aabb_map.count(_id) != 0){
                    aabb_map.erase(_id);
                }
                break;
            }
            default:{
                break;
            }
        }
    }

    void ShatterRender::releaseComputeObject(int _id)
    {
        auto iterator = std::find(computeid_vec.begin(), computeid_vec.end(), _id);
        if(iterator != computeid_vec.end())
        {
            computeid_vec.erase(iterator);
        }
    }

    std::unordered_map<int, int>* ShatterRender::getAABBMap()
    {
        return &aabb_map;
    }
    void ShatterRender::prepareImGui() {
        imGui = GUI::getGUI();

        imGui->init((float)presentExtent.width, (float)presentExtent.height);
        imGui->initResources(m_presentRenderPass, graphics_queue, "../shaders/");

        if(Config::getConfig("enableDockSpace"))
        {
            ImGui_ImplVulkan_InitInfo initInfo{};
            initInfo.Instance = instance;
            initInfo.PhysicalDevice = physicalDevice;
            initInfo.Device = device;
            initInfo.QueueFamily = getIndices().graphicsFamily;
            initInfo.Queue = graphics_queue;
            initInfo.PipelineCache = VK_NULL_HANDLE;
            initInfo.DescriptorPool = descriptorPool;
            initInfo.Subpass = 0;
            initInfo.MinImageCount =  min_image_count;
            initInfo.ImageCount = m_swapChainImageCount;
            initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            initInfo.Allocator = nullptr;
            initInfo.CheckVkResultFn = check_vk_result;

            ImGui_ImplGlfw_InitForVulkan(((GLFWWindow*)SingleAPP.getMainWindow())->get(), true);
            ImGui_ImplVulkan_Init(&initInfo, m_presentRenderPass);
        }
    }

    void ShatterRender::updateColorSet() {
        std::array<VkDescriptorImageInfo, 3> descriptorImageInfos = {
                tool::descriptorImageInfo(VK_NULL_HANDLE, ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[1].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
                tool::descriptorImageInfo(VK_NULL_HANDLE, ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[2].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
                tool::descriptorImageInfo(VK_NULL_HANDLE, ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[3].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        };
        std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};
        for (size_t i = 0; i < descriptorImageInfos.size(); i++) {
            writeDescriptorSets[i] = (tool::writeDescriptorSet(SingleSetPool["gBuffer"], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, i, &descriptorImageInfos[i]));
        }
        vkUpdateDescriptorSets(SingleDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

        float view[2] = {SingleAPP.getPresentViewPort().view.width, SingleAPP.getPresentViewPort().view.height};
        memcpy(SingleBPool.getBuffer("ViewPort", Buffer_Type::Uniform_Buffer)->mapped, view, 8);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[0].imageView;
        imageInfo.sampler = ((VulkanFrameBuffer*)SingleRender.colorFrameBuffers)->m_attachments[0].sampler;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        auto writeSet = tool::writeDescriptorSet(SingleRender.m_colorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &imageInfo);

        vkUpdateDescriptorSets(SingleDevice(), 1, &writeSet, 0, nullptr);
    }
}

