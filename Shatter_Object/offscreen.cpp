//
// Created by jonnxie on 2021/10/13.
//

#include "offscreen.h"
#include "../Shatter_Item/shatter_item.h"
#include "../Shatter_Object/device.h"

#include <mutex>

static std::mutex lock;
static bool ready = false;

OffScreen &OffScreen::getOffScreen() {
    static OffScreen screen;
    std::lock_guard<std::mutex> lockGuard(lock);
    if(!ready)
    {
        screen.setup(SingleDevice.logicalDevice,getViewPort().width,getViewPort().height);
        ready = true;
    }
    return screen;
}

void OffScreen::setup(const VkDevice& _device,
           uint32_t _width,
           uint32_t _height){
    m_width = _width;
    m_height = _height;
    m_color_format = getSwapChainFormat().format;
    m_depth_format = getDepthFormat();

    VkImageCreateInfo image = tool::imageCreateInfo();
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = m_color_format;
    image.extent.width = _width;
    image.extent.height = _height;
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    // We will sample directly from the color attachment
    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkMemoryAllocateInfo memAlloc = tool::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(_device, &image, nullptr, &m_color_image));
    vkGetImageMemoryRequirements(_device, m_color_image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = SingleDevice.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(_device, &memAlloc, nullptr, &m_color_memory));
    VK_CHECK_RESULT(vkBindImageMemory(_device, m_color_image, m_color_memory, 0));

    VkImageViewCreateInfo colorImageView = tool::imageViewCreateInfo();
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = m_color_format;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = m_color_image;
    VK_CHECK_RESULT(vkCreateImageView(_device, &colorImageView, nullptr, &m_color_image_view));

    // Create sampler to sample from the attachment in the fragment shader
    VkSamplerCreateInfo samplerInfo = tool::samplerCreateInfo();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(_device, &samplerInfo, nullptr, &m_sampler));

    // Depth stencil attachment
    image.format = m_depth_format;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT| VK_IMAGE_USAGE_SAMPLED_BIT;

    VK_CHECK_RESULT(vkCreateImage(_device, &image, nullptr, &m_depth_image));
    vkGetImageMemoryRequirements(_device, m_depth_image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = SingleDevice.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(_device, &memAlloc, nullptr, &m_depth_memory));
    VK_CHECK_RESULT(vkBindImageMemory(_device, m_depth_image, m_depth_memory, 0));

    VkImageViewCreateInfo depthStencilView = tool::imageViewCreateInfo();
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = m_depth_format;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
//    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = m_depth_image;
    VK_CHECK_RESULT(vkCreateImageView(_device, &depthStencilView, nullptr, &m_depth_image_view));

    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

    std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
    // Color attachment
    attchmentDescriptions[0].format = m_color_format;
    attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Depth attachment
    attchmentDescriptions[1].format = m_depth_format;
    attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
    renderPassInfo.pAttachments = attchmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &m_pass));

    VkImageView attachments[2];
    attachments[0] = m_color_image_view;
    attachments[1] = m_depth_image_view;

    VkFramebufferCreateInfo fbufCreateInfo = tool::framebufferCreateInfo();
    fbufCreateInfo.renderPass = m_pass;
    fbufCreateInfo.attachmentCount = 2;
    fbufCreateInfo.pAttachments = attachments;
    fbufCreateInfo.width = _width;
    fbufCreateInfo.height = _height;
    fbufCreateInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(_device, &fbufCreateInfo, nullptr, &m_frame_buffer));

    // Fill a descriptor for later use in a descriptor set
    m_color_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_color_descriptor.imageView = m_color_image_view;
    m_color_descriptor.sampler = m_sampler;

    m_depth_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_depth_descriptor.imageView = m_depth_image_view;
    m_depth_descriptor.sampler = m_sampler;

    m_inherit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    m_inherit_info.pNext = VK_NULL_HANDLE;
    m_inherit_info.renderPass = m_pass;
    m_inherit_info.subpass = 0;
    m_inherit_info.framebuffer = m_frame_buffer;
    m_inherit_info.occlusionQueryEnable = false;
}

void OffScreen::release() {
    vkDestroyImageView(SingleDevice.logicalDevice,m_depth_image_view,nullptr);
    vkDestroyImageView(SingleDevice.logicalDevice,m_color_image_view,nullptr);

    vkDestroyImage(SingleDevice.logicalDevice,m_depth_image,nullptr);
    vkDestroyImage(SingleDevice.logicalDevice,m_color_image,nullptr);

    vkFreeMemory(SingleDevice.logicalDevice, m_depth_memory, nullptr);
    vkFreeMemory(SingleDevice.logicalDevice, m_color_memory, nullptr);

    vkDestroySampler(SingleDevice.logicalDevice, m_sampler, nullptr);

    vkDestroyFramebuffer(SingleDevice.logicalDevice,m_frame_buffer,nullptr);
    vkDestroyRenderPass(SingleDevice.logicalDevice,m_pass,nullptr);
}

void OffScreen::beginRenderPass(VkCommandBuffer _cb,bool _multiple) const {
    VkClearValue clearValues[2];
//    clearValues[0].color = {0.92f, 0.92f, 0.92f, 1.0f}; //white
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = tool::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = m_pass;
    renderPassBeginInfo.framebuffer = m_frame_buffer;
    renderPassBeginInfo.renderArea.extent.width = m_width;
    renderPassBeginInfo.renderArea.extent.height = m_height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    if(!_multiple){
        vkCmdBeginRenderPass(_cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }else{
        vkCmdBeginRenderPass(_cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    }
}

void OffScreen::endRenderPass(VkCommandBuffer _cb) {
    vkCmdEndRenderPass(_cb);
}

CascadeShadow &CascadeShadow::getCascadeShadow() {
    static CascadeShadow shadow;
    static bool shadowReady = false;
    static std::mutex shadowLock;
    std::lock_guard<std::mutex> lockGuard(shadowLock);
    if(!shadowReady)
    {
        shadow.setup(SingleDevice.logicalDevice,SHADOWMAP_DIM,SHADOWMAP_DIM);
        shadowReady = true;
    }
    return shadow;
}

void CascadeShadow::release() {
    depthPass.destroy(SingleDevice());
    depth.destroy(SingleDevice());
    for(auto &i : cascades)
    {
        i.destroy(SingleDevice());
    }
}

void CascadeShadow::beginRenderPass(VkCommandBuffer _cb,uint32_t _index,bool _multiple) {
    VkClearValue clearValues[1];
    clearValues[0].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = tool::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = depthPass.renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = SHADOWMAP_DIM;
    renderPassBeginInfo.renderArea.extent.height = SHADOWMAP_DIM;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues;

    VkViewport viewport = tool::viewport((float)SHADOWMAP_DIM, (float)SHADOWMAP_DIM, 0.0f, 1.0f);
    vkCmdSetViewport(_cb, 0, 1, &viewport);

    VkRect2D scissor = tool::rect2D(SHADOWMAP_DIM, SHADOWMAP_DIM, 0, 0);
    vkCmdSetScissor(_cb, 0, 1, &scissor);

    // One pass per cascade
    // The layer that this pass renders to is defined by the cascade's image view (selected via the cascade's descriptor set)
    renderPassBeginInfo.framebuffer = cascades[_index].frameBuffer;
    vkCmdBeginRenderPass(_cb, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    inheritInfo.framebuffer = cascades[_index].frameBuffer;
}

void CascadeShadow::endRenderPass(VkCommandBuffer _cb) {
    vkCmdEndRenderPass(_cb);
}

void CascadeShadow::setup(VkDevice const &_device, uint32_t _width, uint32_t _height) {
    VkFormat depthFormat = getDepthFormat();
    /*
        Depth map renderpass
    */
    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = depthFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 0;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthReference;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = tool::renderPassCreateInfo();
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassCreateInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(SingleDevice(), &renderPassCreateInfo, nullptr, &depthPass.renderPass));

    /*
        Layered depth image and views
    */

    VkImageCreateInfo imageInfo = tool::imageCreateInfo();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = SHADOWMAP_DIM;
    imageInfo.extent.height = SHADOWMAP_DIM;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.format = depthFormat;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VK_CHECK_RESULT(vkCreateImage(SingleDevice(), &imageInfo, nullptr, &depth.image));
    VkMemoryAllocateInfo memAlloc = tool::memoryAllocateInfo();
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(SingleDevice(), depth.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = SingleDevice.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(SingleDevice(), &memAlloc, nullptr, &depth.mem));
    VK_CHECK_RESULT(vkBindImageMemory(SingleDevice(), depth.image, depth.mem, 0));
    // Full depth map view (all layers)
    VkImageViewCreateInfo viewInfo = tool::imageViewCreateInfo();
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange = {};
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = SHADOW_MAP_CASCADE_COUNT;
    viewInfo.image = depth.image;
    VK_CHECK_RESULT(vkCreateImageView(SingleDevice(), &viewInfo, nullptr, &depth.view));

    // One image and framebuffer per cascade
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        // Image view for this cascade's layer (inside the depth map)
        // This view is used to render to that specific depth image layer
        VkImageViewCreateInfo viewInfo = tool::imageViewCreateInfo();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange = {};
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = i;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.image = depth.image;
        VK_CHECK_RESULT(vkCreateImageView(SingleDevice(), &viewInfo, nullptr, &cascades[i].view));
        // Framebuffer
        VkFramebufferCreateInfo framebufferInfo = tool::framebufferCreateInfo();
        framebufferInfo.renderPass = depthPass.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &cascades[i].view;
        framebufferInfo.width = SHADOWMAP_DIM;
        framebufferInfo.height = SHADOWMAP_DIM;
        framebufferInfo.layers = 1;
        VK_CHECK_RESULT(vkCreateFramebuffer(SingleDevice(), &framebufferInfo, nullptr, &cascades[i].frameBuffer));
    }

    // Shared sampler for cascade depth reads
    VkSamplerCreateInfo sampler = tool::samplerCreateInfo();
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 1.0f;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(SingleDevice(), &sampler, nullptr, &depth.sampler));

    inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritInfo.pNext = VK_NULL_HANDLE;
    inheritInfo.renderPass = depthPass.renderPass;
    inheritInfo.subpass = 0;
    inheritInfo.occlusionQueryEnable = false;

    depth.descriptor.sampler = depth.sampler;
    depth.descriptor.imageView = depth.view;
    depth.descriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
}
