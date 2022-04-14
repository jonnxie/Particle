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
            "VK_LAYER_KHRONOS_validation"
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
        positionAttachment = new FrameBufferAttachment;
        normalAttachment = new FrameBufferAttachment;
        albedoAttachment = new FrameBufferAttachment;
        depthAttachment = new FrameBufferAttachment;
        clearValues.resize(AttachmentCount);
        clearValues[0].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[1].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[2].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[3].color = { { 0.92f, 0.92f, 0.92f, 1.0f } };
        clearValues[4].depthStencil = { 1.0f, 0 };
        initWindow();
        initVulkan();
    }

    void ShatterRender::loop(){
//        glfwPollEvents();
        if(Config::getConfig("enableScreenGui"))
        {
            GUI::getGUI()->updateUI();
        }
        draw();
    }

    void ShatterRender::cleanup(){
        cleanupSwapChain();
        if(m_renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, m_renderPass, nullptr);
        }

#ifdef SHATTER_GPU_CAPTURE
        if(m_captureRenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, m_captureRenderPass, nullptr);
        }
        m_frameBuffers->release();
        delete m_frameBuffers;
#endif

        vkDestroySwapchainKHR(device, swapchain, nullptr);

        delete positionAttachment;
        delete normalAttachment;
        delete albedoAttachment;
        delete depthAttachment;

        delete imGui;

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(device, computeFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, computeReadySemaphore, nullptr);

        vkDestroyCommandPool(device, graphic_commandPool, nullptr);
        vkDestroyCommandPool(device, compute_commandPool, nullptr);
        vkDestroyCommandPool(device, transfer_commandPool, nullptr);

        releaseThreadObjectPool();
        releaseThreadCommandPool();

        vkDestroyDevice(device, nullptr);
        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void ShatterRender::initWindow(){
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(SingleAPP.getScreenWidth(), SingleAPP.getScreenHeight(), "Vulkan", nullptr, nullptr);

        setViewPort(UnionViewPort{
                VkViewport{0,
                           0,
                           float(SingleAPP.getScreenWidth()),
                           float(SingleAPP.getScreenHeight()),
                           0,
                           1.0f}
        });

        setScissor(VkRect2D{
            VkOffset2D{0,0},
            VkExtent2D{uint32_t(SingleAPP.getScreenWidth()), uint32_t(SingleAPP.getScreenHeight())}
        });

        //在此处设置了将当前对象指针存放在窗体对象中，在回调函数中取出
        glfwSetWindowUserPointer(window, this);

        //注册回调函数
        glfwSetWindowSizeCallback(window, onWindowResized);

        glfwSetKeyCallback(window, keyCallback);

        glfwSetMouseButtonCallback(window, mouseCallback);

        glfwSetCursorPosCallback(window, cursorPositionCallback);

        glfwSetScrollCallback(window, scrollCallback);

        glfwSetCharCallback(window, keyTypeCallback);
    }

    void ShatterRender::initVulkan(){
        createInstance();
        setupDebugCallback();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        if(Config::getConfig("EnableMSAA"))
        {
            createMSAARenderPass();
        }else{
            createRenderPass();
        }
#ifdef SHATTER_GPU_CAPTURE
        createCaptureRenderPass();
        createCaptureFramebuffers();
#endif
        createGraphicsCommandPool();
        createComputeCommandPool();
        createTransferCommandPool();
        createDescriptorPool();
        createDepthResources();
        createFramebuffers();
        prepareImGui();
        createPrimaryCommandBuffers();
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
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
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
        VkExtent2D extent = ::chooseSwapExtent(swapChainSupport.capabilities,getViewPort().view.width,getViewPort().view.height);

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

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        Config::setConfig("SwapChainImageCount", imageCount);

        if(swapChainSupport.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT){
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if(swapChainSupport.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT){
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));

        min_image_count = createInfo.minImageCount;

        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        swapchain_images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchain_images.data());

        pre_offscreen_buffer.resize(imageCount);
        pre_shadow_buffer.resize(imageCount);
        pre_g_buffer.resize(imageCount);
        pre_norm_buffer.resize(imageCount);
        pre_trans_buffer.resize(imageCount);
        pre_capture_buffers.resize(imageCount);

        swapchain_image_format = surfaceFormat.format;
        swapchain_extent = extent;
        m_depthFormat = findDepthFormat();
    }

    void ShatterRender::createAttachmentResources()
    {
        /*
         * back
         */
        {
            uint32_t imageCount;
            vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
            m_swapchainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(device, swapchain, &imageCount, m_swapchainImages.data());
            m_swapChainImageviews.resize(swapchain_images.size());
            m_swapChainSamplers.resize(swapchain_images.size());
            VkSamplerCreateInfo samplerInfo = tool::samplerCreateInfo();
            m_swapChainSets.resize(swapchain_images.size());

            for (size_t i = 0; i < m_swapchainImages.size(); i++) {
                m_swapChainImageviews[i] = buffer::ShatterTexture::Create_ImageView(&device, m_swapchainImages[i], swapchain_image_format,
                                                                                    VK_IMAGE_ASPECT_COLOR_BIT);
                vkCreateSampler(device, &samplerInfo, nullptr, &m_swapChainSamplers[i]);
            }
        }
        /*
         * position
         */
        createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, positionAttachment);

        /*
         * normal
         */
        createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, normalAttachment);

        /*
         * albedo
         */
        createAttachment(VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, albedoAttachment);
        /*
         * depth
         */
        createAttachment(m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthAttachment);
    }

    void ShatterRender::createCaptureFramebuffers()
    {
        FrameBufferSpecification spec{};
        spec.Width = getViewPort().view.width;
        spec.Height = getViewPort().view.height;
        spec.formats = {{m_captureFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
                        {m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}};
        spec.Samples = 1;
        spec.SwapChainTarget = false;
        spec.RenderPass = m_captureRenderPass;

        m_frameBuffers = FrameBuffer::createFramebuffer(spec);
    }

    void ShatterRender::createFramebuffers(){
        createAttachmentResources();
        m_swapChainFramebuffers.resize(m_swapChainImageviews.size());

        for (size_t i = 0; i < m_swapChainImageviews.size(); i++) {
            std::array<VkImageView, AttachmentCount> attachments = {
                    m_swapChainImageviews[i],
                    positionAttachment->view,
                    normalAttachment->view,
                    albedoAttachment->view,
                    depthAttachment->view,
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapchain_extent.width;
            framebufferInfo.height = swapchain_extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void ShatterRender::clearAttachment(FrameBufferAttachment* _attachment)
    {
        vkDestroyImageView(device, _attachment->view, nullptr);
        _attachment->view = VK_NULL_HANDLE;
        vkDestroyImage(device, _attachment->image, nullptr);
        _attachment->image = VK_NULL_HANDLE;
        vkFreeMemory(device, _attachment->mem, nullptr);
        _attachment->mem = VK_NULL_HANDLE;
    }

    void ShatterRender::createAttachment(VkFormat _format, VkImageUsageFlags _usage, FrameBufferAttachment* _attachment)
    {
        if (_attachment->image != VK_NULL_HANDLE) {
            vkDestroyImageView(device, _attachment->view, nullptr);
            vkDestroyImage(device, _attachment->image, nullptr);
            vkFreeMemory(device, _attachment->mem, nullptr);
        }

        VkImageAspectFlags aspectMask = 0;
        VkImageLayout imageLayout;

        _attachment->format = _format;

        if (_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        assert(aspectMask > 0);

        VkImageCreateInfo image = tool::imageCreateInfo();
        image.imageType = VK_IMAGE_TYPE_2D;
        image.format = _format;
        image.extent.width = getViewPort().view.width;
        image.extent.height = getViewPort().view.height;
//        image.extent.width = swapchain_extent.width;
//        image.extent.height = swapchain_extent.height;
        image.extent.depth = 1;
        image.mipLevels = 1;
        image.arrayLayers = 1;
        image.samples = VK_SAMPLE_COUNT_1_BIT;
        image.tiling = VK_IMAGE_TILING_OPTIMAL;
        // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT flag is required for input attachments
        image.usage = _usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkMemoryAllocateInfo memAlloc = tool::memoryAllocateInfo();
        VkMemoryRequirements memReqs;

        VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &_attachment->image));
        vkGetImageMemoryRequirements(device, _attachment->image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &_attachment->mem));
        VK_CHECK_RESULT(vkBindImageMemory(device, _attachment->image, _attachment->mem, 0));

        VkImageViewCreateInfo imageView = tool::imageViewCreateInfo();
        imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageView.format = _format;
        imageView.subresourceRange = {};
        imageView.subresourceRange.aspectMask = aspectMask;
        imageView.subresourceRange.baseMipLevel = 0;
        imageView.subresourceRange.levelCount = 1;
        imageView.subresourceRange.baseArrayLayer = 0;
        imageView.subresourceRange.layerCount = 1;
        imageView.image = _attachment->image;
        VK_CHECK_RESULT(vkCreateImageView(device, &imageView, nullptr, &_attachment->view));
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


    void ShatterRender::createDepthResources(){
        m_depthFormat = findDepthFormat();

        setDepthFormat(m_depthFormat);

        buffer::ShatterTexture::Create_Image(&device,
                                             swapchain_extent.width,
                                             swapchain_extent.height,
                                             m_depthFormat,
                                             VK_IMAGE_TILING_OPTIMAL,
                                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                             m_depthImage,
                                             m_depthImageMemory);
        m_depthImageView = buffer::ShatterTexture::Create_ImageView(&device, m_depthImage, m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        buffer::ShatterTexture::Transition_ImageLayout(m_depthImage,
                                                       m_depthFormat,
                                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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

    void ShatterRender::createRenderPass(){
        std::array<VkAttachmentDescription,AttachmentCount> attachments{};
        {
            //Color attachment
            attachments[AttachmentBack].format = swapchain_image_format;
            attachments[AttachmentBack].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentBack].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentBack].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[AttachmentBack].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentBack].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentBack].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentBack].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            // Position
            attachments[AttachmentPosition].format = positionAttachment->format = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachments[AttachmentPosition].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentPosition].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentPosition].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentPosition].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentPosition].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentPosition].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentPosition].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Normals
            attachments[AttachmentNormal].format = normalAttachment->format = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachments[AttachmentNormal].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[AttachmentNormal].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[AttachmentNormal].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentNormal].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[AttachmentNormal].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[AttachmentNormal].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[AttachmentNormal].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Albedo
            attachments[AttachmentAlbedo].format = albedoAttachment->format = VK_FORMAT_R8G8B8A8_UNORM;
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

        uint32_t preserveAttachmentIndex = 1;

        subpassDescriptions[SubpassLight].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[SubpassLight].colorAttachmentCount = 1;
        subpassDescriptions[SubpassLight].pColorAttachments = &colorReference;
        subpassDescriptions[SubpassLight].pDepthStencilAttachment = &depthReference;
        subpassDescriptions[SubpassLight].inputAttachmentCount = 3;
        subpassDescriptions[SubpassLight].pInputAttachments = inputReferences;

        colorReference = { AttachmentBack, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        inputReferences[0] = { AttachmentPosition, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        subpassDescriptions[SubpassTransparency].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[SubpassTransparency].colorAttachmentCount = 1;
        subpassDescriptions[SubpassTransparency].pColorAttachments = &colorReference;
        subpassDescriptions[SubpassTransparency].pDepthStencilAttachment = &depthReference;
        // Use the color/depth attachments filled in the first pass as input attachments
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

        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass));
    }

    void ShatterRender::createMSAARenderPass(){
        std::array<VkAttachmentDescription, MSAAAttachmentCount> attachments{};
        {
            //Color attachment
            attachments[MSAAAttachmentBack].format = swapchain_image_format;
            attachments[MSAAAttachmentBack].samples = VkSampleCountFlagBits(Config::getConfig("FramebufferSampleCount"));
            attachments[MSAAAttachmentBack].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[MSAAAttachmentBack].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[MSAAAttachmentBack].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[MSAAAttachmentBack].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentBack].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[MSAAAttachmentBack].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Position
            attachments[MSAAAttachmentPosition].format = positionAttachment->format = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachments[MSAAAttachmentPosition].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[MSAAAttachmentPosition].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[MSAAAttachmentPosition].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentPosition].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[MSAAAttachmentPosition].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentPosition].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[MSAAAttachmentPosition].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Normals
            attachments[MSAAAttachmentNormal].format = normalAttachment->format = VK_FORMAT_R16G16B16A16_SFLOAT;
            attachments[MSAAAttachmentNormal].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[MSAAAttachmentNormal].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[MSAAAttachmentNormal].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentNormal].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[MSAAAttachmentNormal].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentNormal].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[MSAAAttachmentNormal].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Albedo
            attachments[MSAAAttachmentAlbedo].format = albedoAttachment->format = VK_FORMAT_R8G8B8A8_UNORM;
            attachments[MSAAAttachmentAlbedo].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[MSAAAttachmentAlbedo].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[MSAAAttachmentAlbedo].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentAlbedo].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[MSAAAttachmentAlbedo].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentAlbedo].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[MSAAAttachmentAlbedo].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            //Depth attachment
            attachments[MSAAAttachmentDepth].format = m_depthFormat;
            attachments[MSAAAttachmentDepth].samples = VkSampleCountFlagBits(Config::getConfig("FramebufferSampleCount"));
            attachments[MSAAAttachmentDepth].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[MSAAAttachmentDepth].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentDepth].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[MSAAAttachmentDepth].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentDepth].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[MSAAAttachmentDepth].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            //present attachment
            attachments[MSAAAttachmentPresent].format = swapchain_image_format;
            attachments[MSAAAttachmentPresent].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[MSAAAttachmentPresent].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[MSAAAttachmentPresent].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[MSAAAttachmentPresent].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[MSAAAttachmentPresent].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[MSAAAttachmentPresent].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[MSAAAttachmentPresent].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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

        uint32_t preserveAttachmentIndex = 1;

        subpassDescriptions[SubpassLight].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[SubpassLight].colorAttachmentCount = 1;
        subpassDescriptions[SubpassLight].pColorAttachments = &colorReference;
        subpassDescriptions[SubpassLight].pDepthStencilAttachment = &depthReference;
        subpassDescriptions[SubpassLight].inputAttachmentCount = 3;
        subpassDescriptions[SubpassLight].pInputAttachments = inputReferences;

        colorReference = { AttachmentBack, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        inputReferences[0] = { AttachmentPosition, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

        VkAttachmentReference colorAttachmentResolveRef{};
//        colorAttachmentResolveRef.attachment = AttachmentResolve;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        subpassDescriptions[SubpassTransparency].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[SubpassTransparency].colorAttachmentCount = 1;
        subpassDescriptions[SubpassTransparency].pColorAttachments = &colorReference;
        subpassDescriptions[SubpassTransparency].pDepthStencilAttachment = &depthReference;
        // Use the color/depth attachments filled in the first pass as input attachments
        subpassDescriptions[SubpassTransparency].inputAttachmentCount = 1;
        subpassDescriptions[SubpassTransparency].pInputAttachments = inputReferences;
        subpassDescriptions[SubpassTransparency].pResolveAttachments = &colorAttachmentResolveRef;

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

        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass));
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

    void ShatterRender::createPrimaryCommandBuffers(){
        VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
        commandBufferAllocateInfo.commandPool = graphic_commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        graphics_buffers.resize(m_swapChainFramebuffers.size());
        commandBufferAllocateInfo.commandBufferCount = m_swapChainFramebuffers.size();
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device,&commandBufferAllocateInfo,graphics_buffers.data()));

        commandBufferAllocateInfo.commandPool = compute_commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &compute_buffer));
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
            std::vector<VkCommandBuffer> depthBuffer(drawid_vec.size());
            for (int t = 0; t < std::thread::hardware_concurrency(); t++)
            {
                for (int j = 0; j < numObjectsPerThread; j++)
                {
                    if(draw_index >= drawid_vec.size()) break;
                    (*threadPool).threads[t]->addTask([&,draw_index] {
                        VkCommandBuffer *cb = &depthBuffer[draw_index];
                        ObjectTask::shadowDepthTask(t,drawid_vec[draw_index],SingleCascade.inheritInfo,cb);
                    });
                    draw_index++;
                }
                if(draw_index >= drawid_vec.size()) break;
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
        std::advance(end,drawid_vec.size());
        for(size_t index = 0; index < SHADOW_MAP_CASCADE_COUNT; index++)
        {
            SingleCascade.beginRenderPass(_cb, index,true);
            std::vector<VkCommandBuffer> depthBuffer(begin,end);
            vkCmdExecuteCommands(_cb, depthBuffer.size(), depthBuffer.data());
            std::advance(begin , drawid_vec.size());
            std::advance(end , drawid_vec.size());
            SingleCascade.endRenderPass(_cb);
        }
    }


    void ShatterRender::createSecondaryCommandBuffers()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = graphic_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        allocInfo.commandBufferCount = m_swapChainFramebuffers.size();
        gui_buffer.resize(m_swapChainFramebuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, gui_buffer.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate gui command buffer!");
        }

        allocInfo.commandBufferCount = swapchain_images.size();
        offscreen_buffers.resize(swapchain_images.size());
        if (vkAllocateCommandBuffers(device, &allocInfo, offscreen_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate off screen command buffers!");
        }

        composite_buffers.resize(swapchain_images.size());
        if (vkAllocateCommandBuffers(device, &allocInfo, composite_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate off screen command buffers!");
        }

    }

    void ShatterRender::cleanupObject()
    {
        pre_compute_buffers.clear();

        SingleDPool->free(offdrawid_vec);
        offdrawid_vec.clear();
        SingleDPool->free(drawid_vec);
        drawid_vec.clear();
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

        VK_CHECK_RESULT(vkBeginCommandBuffer(compute_buffer, &cmdBufInfo));
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

        for(size_t compute_index = 0; compute_index < normal_vec.size(); compute_index++)
        {
            threadPool->addTask([&,compute_index] {
                VkCommandBuffer* cb = &commandBuffers[compute_index];
                ObjectTask::computeTask(computeid_vec[compute_index], inheritanceInfo, cb);
            });
        }

        (*threadPool).wait();

        pre_compute_buffers = commandBuffers;

        // Execute render commands from the secondary command buffer
        vkCmdExecuteCommands(compute_buffer, commandBuffers.size(), commandBuffers.data());

//        TaskPool::computeBarrierRelease(compute_buffer);

        VK_CHECK_RESULT(vkEndCommandBuffer(compute_buffer));
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
        VK_CHECK_RESULT(vkBeginCommandBuffer(compute_buffer, &cmdBufInfo));
        if(!computeid_vec.empty())
        {
            vkCmdExecuteCommands(compute_buffer, computeid_vec.size(), pre_compute_buffers.data());
        }

        VK_CHECK_RESULT(vkEndCommandBuffer(compute_buffer));
    }

    void ShatterRender::createCaptureCommandBuffers(VkCommandBuffer _cb, int _imageIndex){
        std::array<VkClearValue,2> clearCaptureValue{};
        clearCaptureValue[0].color = { { uint32_t(0) } };
        clearCaptureValue[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        {
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = VK_NULL_HANDLE;
            renderPassBeginInfo.renderPass = m_captureRenderPass;
            renderPassBeginInfo.framebuffer = ((VulkanFrameBuffer*)m_frameBuffers)->m_frame_buffer;
            renderPassBeginInfo.renderArea.offset = {0, 0};
            renderPassBeginInfo.renderArea.extent = getScissor().extent;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearCaptureValue.data();
        }
//        TaskPool::captureBarrierRequire(m_capture_buffer);
        vkCmdBeginRenderPass(_cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
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
            inheritanceInfo.framebuffer = ((VulkanFrameBuffer*)m_frameBuffers)->m_frame_buffer;
            inheritanceInfo.occlusionQueryEnable = false;
        }

        VkCommandBufferBeginInfo commandBufferBeginInfo {};
        {
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
        }
        VkViewport tmp = getViewPort().view;
        VkRect2D& scissor = getScissor();

        int threadIndex = 0;
        for(auto& pair: aabb_map)
        {
            int Id =  pair.second;
            (*SingleThreadPool)[threadIndex]->addTask([&, Id, threadIndex, index](){
                VkCommandPool pool = getCommandPool(CommandPoolType::GraphicsPool, threadIndex);
                VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
                commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
                commandBufferAllocateInfo.commandPool = pool;
                commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                commandBufferAllocateInfo.commandBufferCount = 1;
                vkAllocateCommandBuffers(SingleDevice.logicalDevice, &commandBufferAllocateInfo, &captureBuffers[index]);

                vkBeginCommandBuffer(captureBuffers[index], &commandBufferBeginInfo);
                vkCmdSetViewport(captureBuffers[index], 0, 1, &tmp);
                vkCmdSetScissor(captureBuffers[index], 0, 1, &scissor);
                vkCmdBindPipeline(captureBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, (*SinglePPool["AABBCapture"])());
                int model_index = (*SingleAABBPool)[Id]->m_model_index;
                ShatterBuffer* buffer = SingleBPool.getBuffer(tool::combine("Capture", Id), Buffer_Type::Vertex_Buffer);
                vkCmdBindVertexBuffers(captureBuffers[index], 0, 1, &buffer->m_buffer, &offsets);
                std::array<VkDescriptorSet, 3> set_array{};
                set_array[0] = *(*set_pool)[model_index];
                set_array[1] = SingleSetPool["Camera"];
                set_array[2] = (*SingleAABBPool)[Id]->m_capture_set;
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
            vkCmdExecuteCommands(_cb, captureBuffers.size(), captureBuffers.data());
        }
        pre_capture_buffers[_imageIndex].clear();
        pre_capture_buffers[_imageIndex].insert(pre_capture_buffers[_imageIndex].end(), captureBuffers.begin(), captureBuffers.end());
        vkCmdEndRenderPass(_cb);
//        TaskPool::captureBarrierRelease(m_capture_buffer);
    }

    void ShatterRender::updateCaptureCommandBuffers(VkCommandBuffer _cb,int _imageIndex)
    {
        std::array<VkClearValue,2> clearCaptureValue{};
        clearValues[0].color = { { uint32_t(0) } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        {
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = VK_NULL_HANDLE;
            renderPassBeginInfo.renderPass = m_captureRenderPass;
            renderPassBeginInfo.framebuffer = ((VulkanFrameBuffer*)m_frameBuffers)->m_frame_buffer;
            renderPassBeginInfo.renderArea.offset = {0, 0};
            renderPassBeginInfo.renderArea.extent = getScissor().extent;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearCaptureValue.data();
        }
        vkCmdBeginRenderPass(_cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        if(!aabb_map.empty())
        {
            vkCmdExecuteCommands(_cb, aabb_map.size(), pre_capture_buffers[_imageIndex].data());
        }

        vkCmdEndRenderPass(_cb);
    }

    void ShatterRender::createGraphicsCommandBuffersMultiple(){
        // Contains the list of secondary command buffers to be submitted
        vkQueueWaitIdle(graphics_queue);
        for (size_t i = 0; i < graphics_buffers.size(); i++) {
            std::vector<VkCommandBuffer> commandBuffers;
            commandBuffers.reserve(3);

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
                renderPassBeginInfo.renderPass = m_renderPass;
                renderPassBeginInfo.framebuffer = m_swapChainFramebuffers[i];
                renderPassBeginInfo.renderArea.offset = {0, 0};
                renderPassBeginInfo.renderArea.extent = getScissor().extent;
                renderPassBeginInfo.clearValueCount = AttachmentCount;
                renderPassBeginInfo.pClearValues = clearValues.data();
            }

            if(Config::getConfig("enableScreenGui"))
            {
                imGui->newFrame(false);
                imGui->updateBuffers();
            }

            vkBeginCommandBuffer(graphics_buffers[i], &cmdBufInfo);
            auto threadPool = ThreadPool::pool();
            // Inheritance info for the secondary command buffers
            VkCommandBufferInheritanceInfo inheritanceInfo {};
            {
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.pNext = VK_NULL_HANDLE;
                inheritanceInfo.renderPass = m_renderPass;
                inheritanceInfo.subpass = 0;
                inheritanceInfo.framebuffer = m_swapChainFramebuffers[i];
                inheritanceInfo.occlusionQueryEnable = false;
            }


            #ifdef SHATTER_OFFSCREEN
                createOffScreenBuffers(graphics_buffers[i], i);
            #endif

            #ifdef SHATTER_SHADOW
                createShadowGraphicsBuffers(graphics_buffers[i], i);
            #endif

            #ifdef SHATTER_GPU_CAPTURE
                createCaptureCommandBuffers(graphics_buffers[i], i);
            #endif
            auto dPool = SingleDPool;
            /*
             * Use barrier to changed offscreen and shadow image layout to shader resource
             */
            TaskPool::barrierRequire(graphics_buffers[i]);

            vkCmdBeginRenderPass(graphics_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            int draw_index = 0;
            int threadIndex = 0;

            /*
             * G
             */
            {
                draw_index = 0;
                std::vector<VkCommandBuffer> gBuffers(drawid_vec.size());
                inheritanceInfo.subpass = SubpassG;
                threadIndex = 0;
                for(size_t d_index = 0; d_index < drawid_vec.size(); d_index++)
                {
                    (*threadPool)[threadIndex]->addTask([&, d_index, threadIndex](){
                        VkCommandBuffer* cb = &gBuffers[d_index];
                        ObjectTask::gTask(threadIndex, drawid_vec[d_index], inheritanceInfo, cb);
                    });
                    if( ++threadIndex >= threadPool->m_thread_count){
                        threadIndex -= threadPool->m_thread_count;
                    }
                }
                (*threadPool).wait();
                vkCmdExecuteCommands(graphics_buffers[i], gBuffers.size(), gBuffers.data());
                pre_g_buffer[i].clear();
                pre_g_buffer[i].insert(pre_g_buffer[i].end(), gBuffers.begin(), gBuffers.end());
            }
            vkCmdNextSubpass(graphics_buffers[i], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

            /*
             * Light
             */
            {
                inheritanceInfo.subpass = SubpassLight;

                VkCommandBufferBeginInfo commandBufferBeginInfo {};
                commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
                commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

                vkBeginCommandBuffer(composite_buffers[i], &commandBufferBeginInfo);
                UnionViewPort& tmp = getViewPort();
                vkCmdSetViewport(composite_buffers[i], 0, 1, &tmp.view);
                VkRect2D& scissor = getScissor();
                vkCmdSetScissor(composite_buffers[i],0,1,&scissor);

                std::array<VkDescriptorSet,2> sets{};
                {
                    sets[0] = SingleSetPool["gBuffer"];
                    sets[1] = SingleSetPool["MultiLight"];
                }
                vkCmdBindDescriptorSets(composite_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Composition"]->getPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
                vkCmdBindPipeline(composite_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Composition"]->getPipeline());
                vkCmdDraw(composite_buffers[i], 3, 1, 0, 0);
                vkEndCommandBuffer(composite_buffers[i]);

                vkCmdExecuteCommands(graphics_buffers[i], 1, &composite_buffers[i]);
            }
            vkCmdNextSubpass(graphics_buffers[i], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            inheritanceInfo.subpass = SubpassTransparency;

            /*
             * normal object pass
             */
            {
                if(!normal_vec.empty())
                {
                    std::vector<VkCommandBuffer> normalBuffers(normal_vec.size());
                    threadIndex = 0;
                    for(size_t normal_index = 0; normal_index < normal_vec.size(); normal_index++)
                    {
                        (*threadPool)[threadIndex]->addTask([&, normal_index, threadIndex](){
                            VkCommandBuffer* cb = &normalBuffers[normal_index];
                            ObjectTask::newGraphicsTask(threadIndex, normal_vec[normal_index], inheritanceInfo, cb);
                        });
                        if( ++threadIndex >= threadPool->m_thread_count){
                            threadIndex -= threadPool->m_thread_count;
                        }
                    }
                    (*threadPool).wait();
                    vkCmdExecuteCommands(graphics_buffers[i], normalBuffers.size(), normalBuffers.data());
                    pre_norm_buffer[i].clear();
                    pre_norm_buffer[i].insert(pre_norm_buffer[i].end(),normalBuffers.begin(),normalBuffers.end());
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
                    vkCmdExecuteCommands(graphics_buffers[i], transparencyBuffers.size(), transparencyBuffers.data());
//                    pre_trans_buffers.insert(pre_trans_buffers.end(),transparencyBuffers.begin(),transparencyBuffers.end());
                    pre_trans_buffer[i].clear();
                    pre_trans_buffer[i].insert(pre_trans_buffer[i].end(),transparencyBuffers.begin(),transparencyBuffers.end());
                }
            }
//            vkCmdNextSubpass(graphics_buffers[i], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

            std::thread offscreen([&](bool config){
                if(config)
                {
                    VkCommandBufferBeginInfo commandBufferBeginInfo {};
                    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
                    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                    commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

                    vkBeginCommandBuffer(offscreen_buffers[i], &commandBufferBeginInfo);
                    UnionViewPort& tmp = getViewPort();
                    vkCmdSetViewport(offscreen_buffers[i], 0, 1, &tmp.view);

                    VkRect2D& scissor = getScissor();
                    vkCmdSetScissor(offscreen_buffers[i],0,1,&scissor);
                    VkDescriptorSet tmp_set = SingleSetPool["OffScreen"];
                    vkCmdBindDescriptorSets(offscreen_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Quad"]->getPipelineLayout(), 0, 1, &tmp_set, 0, nullptr);
                    vkCmdBindPipeline(offscreen_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, SinglePPool["Quad"]->getPipeline());
                    vkCmdDraw(offscreen_buffers[i], 4, 1, 0, 0);
                    vkEndCommandBuffer(offscreen_buffers[i]);
                }
            },Config::getConfig("enableOffscreenDebug"));

            std::thread gui_thread([&](bool config){
                if(config)
                {
                    VkCommandBufferBeginInfo commandBufferBeginInfo {};
                    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
                    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                    commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

                    vkBeginCommandBuffer(gui_buffer[i], &commandBufferBeginInfo);

                    imGui->drawFrame(gui_buffer[i]);

                    vkEndCommandBuffer(gui_buffer[i]);
                }
            },Config::getConfig("enableScreenGui"));

            offscreen.join();
            if(Config::getConfig("enableOffscreenDebug"))
            {
                commandBuffers.push_back(offscreen_buffers[i]);
            }
            gui_thread.join();
            if(Config::getConfig("enableScreenGui"))
            {
                commandBuffers.push_back(gui_buffer[i]);
            }

            // Execute render commands from the secondary command buffer
            if(!commandBuffers.empty())
            {
                vkCmdExecuteCommands(graphics_buffers[i], commandBuffers.size(), commandBuffers.data());
            }

            vkCmdEndRenderPass(graphics_buffers[i]);

            /*
             * create new render pass to render imgui docking image
             */
//            if(Config::getConfig("enableDockSpace"))
//            {
//                commandBuffers.push_back(gui_buffer);
//            }

            TaskPool::barrierRelease(graphics_buffers[i]);

            vkEndCommandBuffer(graphics_buffers[i]);
        }
    }

    void ShatterRender::updateGraphicsCommandBuffersMultiple(int _index)
    {
        std::vector<VkCommandBuffer> commandBuffers;

        static VkCommandBufferBeginInfo cmdBufInfo{
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                VK_NULL_HANDLE,
                VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                VK_NULL_HANDLE
        };

        static VkRenderPassBeginInfo renderPassBeginInfo{
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                VK_NULL_HANDLE,
                m_renderPass,
                m_swapChainFramebuffers[_index],
                {{0, 0},getScissor().extent},
                AttachmentCount,
                clearValues.data()
        };

        renderPassBeginInfo.framebuffer = m_swapChainFramebuffers[_index];
        renderPassBeginInfo.renderPass = m_renderPass;
        renderPassBeginInfo.renderArea = {{0, 0},getScissor().extent};

        vkQueueWaitIdle(graphics_queue);

        if(Config::getConfig("enableScreenGui"))
        {
            imGui->newFrame(false);
            imGui->updateBuffers();
        }

        vkBeginCommandBuffer(graphics_buffers[_index], &cmdBufInfo);
        // Inheritance info for the secondary command buffers
        static VkCommandBufferInheritanceInfo inheritanceInfo
        {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                VK_NULL_HANDLE,
                m_renderPass,
                0,
                m_swapChainFramebuffers[_index],
                false
        };

        inheritanceInfo.renderPass = m_renderPass;
        inheritanceInfo.framebuffer = m_swapChainFramebuffers[_index];

        TaskPool::barrierRequire(graphics_buffers[_index]);

#ifdef SHATTER_OFFSCREEN
        updateOffscreenBufferAsync(graphics_buffers[_index], int(_index));
#endif

#ifdef SHATTER_SHADOW
        updateShadowGraphicsAsync(graphics_buffers[_index], int(_index));
#endif

#ifdef SHATTER_GPU_CAPTURE
        updateCaptureCommandBuffers(graphics_buffers[_index], int(_index));
#endif

        vkCmdBeginRenderPass(graphics_buffers[_index], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        /*
         * G
         */
        if(!drawid_vec.empty())
        {
            vkCmdExecuteCommands(graphics_buffers[_index], drawid_vec.size(), pre_g_buffer[_index].data());
        }
        vkCmdNextSubpass(graphics_buffers[_index], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        /*
         * Light
         */
        vkCmdExecuteCommands(graphics_buffers[_index], 1, &composite_buffers[_index]);
        vkCmdNextSubpass(graphics_buffers[_index], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);


        /*
         * normal object pass
         */
        if(!normal_vec.empty())
        {
            vkCmdExecuteCommands(graphics_buffers[_index], normal_vec.size(), pre_norm_buffer[_index].data());
        }

        /*
         * transparency pass
         */
        if(!transparency_vec.empty())
        {
            vkCmdExecuteCommands(graphics_buffers[_index], transparency_vec.size(), pre_trans_buffer[_index].data());
        }


        if(Config::getConfig("enableScreenGui"))
        {
            inheritanceInfo.subpass = SubpassTransparency;
            VkCommandBufferBeginInfo commandBufferBeginInfo {};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

            vkBeginCommandBuffer(gui_buffer[_index], &commandBufferBeginInfo);

            imGui->drawFrame(gui_buffer[_index]);

            vkEndCommandBuffer(gui_buffer[_index]);

            commandBuffers.push_back(gui_buffer[_index]);
        }

        // Execute render commands from the secondary command buffer
        if(!commandBuffers.empty())
        {
            vkCmdExecuteCommands(graphics_buffers[_index], commandBuffers.size(), commandBuffers.data());
        }

        vkCmdEndRenderPass(graphics_buffers[_index]);

        TaskPool::barrierRelease(graphics_buffers[_index]);

        vkEndCommandBuffer(graphics_buffers[_index]);
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
        if (vkBeginCommandBuffer(compute_buffer, &command_buffer_begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("command buffer begin failed");
        }

        auto cpool = MPool<CObject>::getPool();
        for(auto& computeobj : computeid_vec){
            (*cpool)[computeobj]->compute(compute_buffer);
        }

        vkEndCommandBuffer(compute_buffer);
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
    }

    void ShatterRender::recreateSwapChain(){
        vkDeviceWaitIdle(device);
        cleanupSwapChain();
        createSwapChain();
//        createRenderPass();
        createDepthResources();
        createFramebuffers();
        createCaptureFramebuffers();

        {
            SinglePPool.release();
            SinglePPool.init();
        }
        createPrimaryCommandBuffers();
        createComputeCommandBuffer();
        createSecondaryCommandBuffers();
        createGraphicsCommandBuffersMultiple();
        vkDeviceWaitIdle(device);
    }

    void ShatterRender::keyEventCallback(int key, int action){
        ImGuiIO& io = ImGui::GetIO();

        if(action == GLFW_PRESS)
        {
            pressKey(key);
            io.KeysDown[key] =  true;
        }else if(action == GLFW_RELEASE)
        {
            io.KeysDown[key] =  false;
            releaseKey(key);
        }
        app::ShatterApp::getApp().key_event_callback(key, action);
    }

    void ShatterRender::mouseEventCallback(int button, int action, int xpos, int ypos){
        if(action == GLFW_PRESS)
        {
            glm::uvec2& coordinate = input::getMousePressCoordiante();
            coordinate.x = xpos;
            coordinate.y = ypos;
//            input::MousePressCoordiante(coordinate, STATE_IN);
            pressMouse(button);
            glm::vec2& tmp = getCursorPressPos();
            tmp.x = float(coordinate.x) / getViewPort().view.width;
            tmp.y = float(coordinate.y) / getViewPort().view.height;
            tmp *= 2.0f;
            tmp -= 1.0f;
            glm::vec3& press_cursor = input::getCursorPress();
            press_cursor = input::getCursor();
//            updateCursorPressPos(tmp);
        }else if(action == GLFW_RELEASE)
        {
            releaseMouse(button);
        }
        app::ShatterApp::getApp().mouse_event_callback(button, action, xpos, ypos);
    }

    void ShatterRender::cleanupSwapChain(){
        vkDestroyImageView(device, m_depthImageView, nullptr);
        vkDestroyImage(device, m_depthImage, nullptr);
        vkFreeMemory(device, m_depthImageMemory, nullptr);

        {
            clearAttachment(positionAttachment);
            clearAttachment(normalAttachment);
            clearAttachment(albedoAttachment);
            clearAttachment(depthAttachment);
        }

        vkFreeCommandBuffers(device,graphic_commandPool,graphics_buffers.size(),graphics_buffers.data());
        vkFreeCommandBuffers(device,graphic_commandPool,gui_buffer.size(),gui_buffer.data());
        vkFreeCommandBuffers(device,graphic_commandPool,offscreen_buffers.size(),offscreen_buffers.data());
        vkFreeCommandBuffers(device,compute_commandPool,1,&compute_buffer);


        {
            for (auto & swapChainFramebuffer : m_swapChainFramebuffers) {
                vkDestroyFramebuffer(device, swapChainFramebuffer, nullptr);
            }

//            if(m_renderPass != VK_NULL_HANDLE)
//            {
//                vkDestroyRenderPass(device, m_renderPass, nullptr);
//            }
            for (auto & s : m_swapChainSamplers){
                vkDestroySampler(device, s, nullptr);
            }

            for (auto & swapChainImageview : m_swapChainImageviews) {
                vkDestroyImageView(device, swapChainImageview, nullptr);
            }


        }


//        for (auto & swapChainImageview : swapChainImageviews) {
//            vkDestroyImageView(device, swapChainImageview, nullptr);
//        }



    }

    void ShatterRender::draw() {
        static bool firstDraw = true;
        static VkPipelineStageFlags computeWaitDstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        if(firstDraw)
        {
            computeSubmitInfo = {
                    VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    nullptr,
                    0,
                    nullptr,
                    nullptr,
                    1,
                    &compute_buffer,
                    1,
                    &computeFinishedSemaphore
            };
            VK_CHECK_RESULT(vkQueueSubmit(compute_queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeReadySemaphore;
            computeSubmitInfo.pWaitDstStageMask = &computeWaitDstStageMask;

            uint32_t imageIndex;
            VK_CHECK_RESULT(vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(),imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));
            setSwapChainIndex(int(imageIndex));

            static VkSemaphore waitSemaphores[] = {imageAvailableSemaphore,computeFinishedSemaphore};
            static VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            static VkSemaphore signalSemaphores[] = {renderFinishedSemaphore,computeReadySemaphore};
            graphicsSubmitInfo = {
                    VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    nullptr,
                    2,
                    waitSemaphores,
                    waitStages,
                    1,
                    &graphics_buffers[imageIndex],
                    2,
                    signalSemaphores
            };

            VK_CHECK_RESULT(vkQueueSubmit(graphics_queue, 1, &graphicsSubmitInfo, VK_NULL_HANDLE));
//            VkPresentInfoKHR presentInfo = {};
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
            VK_CHECK_RESULT(vkQueuePresentKHR(present_queue, &presentInfo));
            firstDraw = false;
        }else{
            if(!windowStill)
            {
//                vkDeviceWaitIdle(device);
                computeSubmitInfo.waitSemaphoreCount = 0;
                computeSubmitInfo.pWaitSemaphores = nullptr;
                computeSubmitInfo.pWaitDstStageMask = nullptr;
                VK_CHECK_RESULT(vkQueueSubmit(compute_queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
                windowStill = true;
                computeSubmitInfo.waitSemaphoreCount = 1;
                computeSubmitInfo.pWaitSemaphores = &computeReadySemaphore;
                computeSubmitInfo.pWaitDstStageMask = &computeWaitDstStageMask;
            }else{
                VK_CHECK_RESULT(vkQueueSubmit(compute_queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
            }

            uint32_t imageIndex;
//            VK_CHECK_RESULT(vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));
            auto requireImageResult = vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
            if((requireImageResult == VK_ERROR_OUT_OF_DATE_KHR) || (requireImageResult == VK_SUBOPTIMAL_KHR)){
                recreateSwapChain();
                windowStill = false;
            }
            setSwapChainIndex(int(imageIndex));

            if(guiChanged || offChanged || drawChanged || normalChanged || transChanged || aabbChanged)
            {
                createGraphicsCommandBuffersMultiple();
                guiChanged = offChanged = drawChanged = normalChanged = transChanged = aabbChanged = false;
            }else{
                if(Config::getConfig("enableScreenGui"))
                {
                    updateGraphicsCommandBuffersMultiple(imageIndex);
                }
            }

            graphicsSubmitInfo.pCommandBuffers = &graphics_buffers[imageIndex];
            assert(vkQueueSubmit(graphics_queue, 1, &graphicsSubmitInfo, VK_NULL_HANDLE) == VK_SUCCESS);

            presentInfo.pImageIndices = &imageIndex;
            auto result = vkQueuePresentKHR(present_queue, &presentInfo);
            if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
                if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    recreateSwapChain();
                    return;
                } else {
                    VK_CHECK_RESULT(result);
                }
            }
//            vkQueueWaitIdle(present_queue);
//            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
//            {
//                recreateSwapChain();
//                windowStill = false;
//            } else if(result != VK_SUCCESS)
//            {
//                VK_CHECK_RESULT(result);
//            }
        }
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
        std::cout << "multiDrawIndirect:" << tool::strBool(checkFeatures("multiDrawIndirect")) << std::endl;

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
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                if(indices.graphicsFamily == -1) {
                    indices.graphicsFamily = i;
                }
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (queueFamily.queueCount > 0 && presentSupport && i != indices.graphicsFamily) {
                    indices.presentFamily = i;
            }else if(queueFamily.queueCount > 0 && presentSupport){
                if(indices.presentFamily == -1){
                    indices.presentFamily = i;
                }
            }

            if(queueFamily.queueCount > 0 && queueFamily.queueFlags &
            VK_QUEUE_COMPUTE_BIT && (i != indices.presentFamily || i != indices.graphicsFamily)){
                indices.computeFamily = i;
            }
            else if(queueFamily.queueCount > 0 && queueFamily.queueFlags &VK_QUEUE_COMPUTE_BIT)
            {
                if(indices.computeFamily == -1)
                {
                    indices.computeFamily = i;
                }
            }

            if(queueFamily.queueCount > 0 && queueFamily.queueFlags &
            VK_QUEUE_TRANSFER_BIT && (i != indices.graphicsFamily || i != indices.computeFamily)){
                indices.transferFamily = i;
            }
            else if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                if(indices.computeFamily == -1)
                {
                    indices.computeFamily = i;
//                    i++;
//                    continue;
                }
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
//        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        getResult(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));
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

    void ShatterRender::onWindowResized(GLFWwindow *window, int width, int height) {
        if (width == 0 || height == 0) return;

        auto *app = reinterpret_cast<ShatterRender *>(glfwGetWindowUserPointer(window));

        setViewPort(UnionViewPort{
                VkViewport{0,0,float(width),float(height),0,1}
        });

        setScissor(VkRect2D{VkOffset2D{0,0},VkExtent2D{uint32_t(width),uint32_t(height)}});

        app->recreateSwapChain();
        app->windowStill = false;
    }

    void ShatterRender::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
//        if(action == GLFW_PRESS){
            auto *app = reinterpret_cast<ShatterRender *>(glfwGetWindowUserPointer(window));
            app->keyEventCallback(key, action);
//        }
    }

    void ShatterRender::mouseCallback(GLFWwindow* window, int button, int action, int mods){
        auto *app = reinterpret_cast<ShatterRender *>(glfwGetWindowUserPointer(window));
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        app->mouseEventCallback(button, action, xpos, ypos);
    }


    void ShatterRender::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos){
        static glm::vec2& cursor = input::getCursorWindow();
        cursor.x = xpos;
        cursor.y = ypos;
        UnionViewPort& viewport = getViewPort();
        glm::vec2& tmp = getCursorPos();
        tmp.x = cursor.x * viewport.inverseWidth;
        tmp.y = cursor.y * viewport.inverseHeight;
        tmp *= 2.0f;
        tmp -= 1.0f;

        glm::vec4 center = SingleCamera.m_camera.proj * SingleCamera.m_camera.view * glm::vec4(SingleCamera.center,1.0f);
        float& depth = input::getTargetDepth();
        depth = center.z / center.w;
        {
//        glm::mat4 p = SingleCamera.m_camera.proj;
//        input::targetDepth(depth, STATE_IN);
        }
        glm::vec4 view = glm::inverse(SingleCamera.m_camera.proj) * glm::vec4(getCursorPos(), depth, 1.0f);
        view /= view.w;
        glm::vec3& world = input::getCursor();
        world = glm::inverse(SingleCamera.m_camera.view) * view;
        SingleCamera.updateCursorRay();
        {
            ImGuiIO& io = ImGui::GetIO();
            bool handled = io.WantCaptureMouse;
        }
    }

    void ShatterRender::scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
        updateScrollPos(glm::vec2(xoffset,yoffset));
        app::ShatterApp::getApp().cameraChanged = true;
    }

    void ShatterRender::keyTypeCallback(GLFWwindow* window, unsigned int code)
    {
        ImGuiIO& io = ImGui::GetIO();
        if(code > 0 && code < 0x10000)
        {
            io.AddInputCharacter((unsigned short)code);
        }
    }

    void ShatterRender::allocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& des_set_layout,
                                               VkDescriptorSet* set){
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = des_set_layout.size();
        allocInfo.pSetLayouts = des_set_layout.data();

        if (vkAllocateDescriptorSets(device, &allocInfo, set) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor set!");
        }
    }

    void ShatterRender::createCommandBuffer(){
        if(Config::getConfig("enableMultipleComputeQueue"))
        {
            createComputeCommandBuffersMultiple();
        }else{
            createComputeCommandBuffer();
        }
        createSecondaryCommandBuffers();
        createGraphicsCommandBuffersMultiple();
    }

    std::vector<int>* ShatterRender::getDObjects()
    {
        return &drawid_vec;
    }

    std::vector<int>* ShatterRender::getCObjects()
    {
        return &computeid_vec;
    }

    std::vector<int>* ShatterRender::getTObjects()
    {
        return &transparency_vec;
    }

    std::vector<int>* ShatterRender::getNObjects()
    {
        return &normal_vec;
    }

    std::vector<int>* ShatterRender::getOffDObjects()
    {
        return &offdrawid_vec;
    }

    void ShatterRender::releaseObject(int _id, DrawObjectType _type)
    {
        switch (_type) {
            case DrawObjectType::Default:
            {
                auto iterator = std::find(drawid_vec.begin(), drawid_vec.end(), _id);
                if(iterator != drawid_vec.end())
                {
                    drawid_vec.erase(iterator);
                }
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
                auto iterator = std::find(normal_vec.begin(), normal_vec.end(), _id);
                if(iterator != normal_vec.end())
                {
                    normal_vec.erase(iterator);
                }
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

    [[maybe_unused]] void ShatterRender::addDObject(int _drawid) {
        drawid_vec.push_back(_drawid);
    }

    void ShatterRender::addCObject(int _computeId) {
        computeid_vec.push_back(_computeId);
    }

    void ShatterRender::prepareImGui() {
        imGui = GUI::getGUI();

        imGui->init((float)swapchain_extent.width,(float)swapchain_extent.height);
        imGui->initResources(m_renderPass, graphics_queue, "../shaders/");

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = instance;
        initInfo.PhysicalDevice = physicalDevice;
        initInfo.Device = device;
        initInfo.QueueFamily = getIndices().graphicsFamily;
        initInfo.Queue = graphics_queue;
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = descriptorPool;
        initInfo.Subpass = SubpassTransparency;
        initInfo.MinImageCount =  min_image_count;
        initInfo.ImageCount = m_swapchainImages.size();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = check_vk_result;

        if(Config::getConfig("enableDockSpace"))
        {
            ImGui_ImplGlfw_InitForVulkan(window, true);
            ImGui_ImplVulkan_Init(&initInfo, m_renderPass);


            for(size_t index = 0; index < m_swapchainImages.size(); index++)
            {
                m_swapChainSets[index] = ImGui_ImplVulkan_AddTexture(m_swapChainSamplers[index],
                                                                     m_swapChainImageviews[index],
                                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                std::cout << "SwapChainImageSet: " << m_swapChainSets[index] << std::endl;
            }
        }
    }
}

