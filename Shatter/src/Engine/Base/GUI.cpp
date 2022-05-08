//
// Created by jonnxie on 2021/9/29.
//
#include "precompiledhead.h"

#include "GUI.h"
#include <mutex>
#include <Engine/Mesh/line.h>
#include <Engine/Particle/particle.h>
#include "Engine/Object/inputaction.h"
#include "Engine/Event/delayevent.h"
#include "Engine/Item/configs.h"
#include RenderCatalog
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include BufferCatalog
#include DeviceCatalog
#include ShaderCatalog
#include ListenerCatalog
#include AppCatalog
#include DLineCatalog
#include "points.h"
#include "tris.h"
#include "WorkPlane.h"

GUI *GUI::gui = new GUI;

static std::mutex lock;

GUI *GUI::getGUI() {

    return gui;
}

void GUI::pushUI(Task_id _id, std::function<void()> _func) {
    std::lock_guard<std::mutex> guard_lock(lock);
    if(gui->m_tasks.count(_id) == 0)
    {
        gui->m_tasks[std::move(_id)] = std::move(_func);
    }
//#ifdef NDEBUG
    else {
//        WARNING(gui task is already existed!);
    }
//#endif
}

void GUI::popUI(const Task_id& _id) {
    std::lock_guard<std::mutex> guard_lock(lock);
    if(gui->m_tasks.count(_id) == 0)
    {
//        WARNING(gui task is already erased!);
    }else{
        gui->m_tasks.erase(_id);
    }
}

void GUI::initResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string &shadersPath) {
    ImGuiIO& io = ImGui::GetIO();

    // Create font texture
    unsigned char* fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    VkDeviceSize uploadSize = texWidth*texHeight * 4 * sizeof(char);

    // Create target image for copy
    VkImageCreateInfo imageInfo = tool::imageCreateInfo();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageInfo, nullptr, &fontImage));
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device->logicalDevice, fontImage, &memReqs);
    VkMemoryAllocateInfo memAllocInfo = tool::memoryAllocateInfo();
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &fontMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, fontImage, fontMemory, 0));

    // Image view
    VkImageViewCreateInfo viewInfo = tool::imageViewCreateInfo();
    viewInfo.image = fontImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewInfo, nullptr, &fontView));

    // Staging buffers for font data upload
    VkBuffer stagingBuffer;

    VkDeviceMemory stagingMemory = device->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer,
            uploadSize);

    void *data;
    vkMapMemory(device->logicalDevice,
                stagingMemory,
                0,
                uploadSize,
                0,
                &data);
    memset(data,0,(size_t)uploadSize);
    memcpy(data,
           fontData,
           uploadSize);
    vkUnmapMemory(device->logicalDevice, stagingMemory);

    // Copy buffer data to font image
//        VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkCommandBuffer commandBuffer = SingleRender.beginSingleTimeCommands();

    // Prepare for transfer
    tool::setImageLayout(
            commandBuffer,
            fontImage,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Copy
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = texWidth;
    bufferCopyRegion.imageExtent.height = texHeight;
    bufferCopyRegion.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
            commandBuffer,
            stagingBuffer,
            fontImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &bufferCopyRegion
    );

    // Prepare for shader read
    tool::setImageLayout(
            commandBuffer,
            fontImage,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    SingleRender.endSingleTimeCommands(commandBuffer) ;

    vkDestroyBuffer(device->logicalDevice,stagingBuffer,VK_NULL_HANDLE);
    vkFreeMemory(device->logicalDevice,
                 stagingMemory,
                 nullptr);

    // Font texture Sampler
    VkSamplerCreateInfo samplerInfo = tool::samplerCreateInfo();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerInfo, nullptr, &sampler));

    // Descriptor Pool
    std::vector<VkDescriptorPoolSize> poolSizes = {
            tool::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    };
    VkDescriptorPoolCreateInfo descriptorPoolInfo = tool::descriptorPoolCreateInfo(poolSizes, 2);
    VK_CHECK_RESULT(vkCreateDescriptorPool(device->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool));

    // Descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            tool::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
    };
    VkDescriptorSetLayoutCreateInfo descriptorLayout = tool::descriptorSetLayoutCreateInfo(setLayoutBindings);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout));

    // Descriptor set
    VkDescriptorSetAllocateInfo allocInfo = tool::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device->logicalDevice, &allocInfo, &descriptorSet));
    VkDescriptorImageInfo fontDescriptor = tool::descriptorImageInfo(
            sampler,
            fontView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            tool::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
    };
    vkUpdateDescriptorSets(device->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    // Pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

    // Pipeline layout
    // Push constants for UI rendering parameters
    VkPushConstantRange pushConstantRange = tool::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = tool::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    VK_CHECK_RESULT(vkCreatePipelineLayout(device->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

    // Setup graphics pipeline for UI rendering
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            tool::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
            tool::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    // Enable blending
    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendState =
            tool::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
            tool::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState =
            tool::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState =
            tool::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    std::vector<VkDynamicState> dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState =
            tool::pipelineDynamicStateCreateInfo(dynamicStateEnables);

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = tool::pipelineCreateInfo(pipelineLayout, renderPass);

    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.subpass = 0;

    // Vertex bindings an attributes based on ImGui vertex definition
    std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
            tool::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
    };
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
            tool::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
            tool::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
            tool::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
    };
    VkPipelineVertexInputStateCreateInfo vertexInputState = tool::pipelineVertexInputStateCreateInfo();
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
    vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

    pipelineCreateInfo.pVertexInputState = &vertexInputState;

    shaderStages[0] = ShaderPool::getPool()["gui_vs"];
    shaderStages[1] = ShaderPool::getPool()["gui_fs"];

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

void GUI::drawFrame(VkCommandBuffer commandBuffer) {
    ImGuiIO& io = ImGui::GetIO();

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport = tool::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // UI scale and translate via push constants
    pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    pushConstBlock.translate = glm::vec2(-1.0f);
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

    // Renderer commands
    ImDrawData* imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;

    if (imDrawData->CmdListsCount > 0) {

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.m_buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT16);

        for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                VkRect2D scissorRect;
                scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
                scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
                scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

                VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
                if (!pcmd->TextureId)
                {
                    desc_set[0] = descriptorSet;
                }
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, desc_set, 0, NULL);

                vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                indexOffset += pcmd->ElemCount;
            }
            vertexOffset += cmd_list->VtxBuffer.Size;
        }
    }
}

void GUI::updateBuffers() {
    ImDrawData* imDrawData = ImGui::GetDrawData();

    // Note: Alignment is done inside buffer creation
    VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

    if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
        return;
    }

    // Update buffers only if vertex or index count has been changed compared to current buffer size

    // Vertex buffer
    if ((vertexBuffer.m_buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
        vertexBuffer.unmap();
        vertexBuffer.release();
        vertexBuffer.m_device = &Device::getDevice().logicalDevice;
        vertexBuffer.m_memory = device->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vertexBuffer.m_buffer, vertexBufferSize);
        vertexCount = imDrawData->TotalVtxCount;
        vertexBuffer.map();
    }

    // Index buffer
    if ((indexBuffer.m_buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
        indexBuffer.unmap();
        indexBuffer.release();
        indexBuffer.m_device = &Device::getDevice().logicalDevice;
        indexBuffer.m_memory = device->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &indexBuffer.m_buffer, indexBufferSize);
        indexCount = imDrawData->TotalIdxCount;
        indexBuffer.map();
    }

    // Upload data
    ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
    ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

    for (int n = 0; n < imDrawData->CmdListsCount; n++) {
        const ImDrawList* cmd_list = imDrawData->CmdLists[n];
        memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += cmd_list->VtxBuffer.Size;
        idxDst += cmd_list->IdxBuffer.Size;
    }

    // Flush to make writes visible to GPU
    vertexBuffer.flush();
    indexBuffer.flush();
}

void GUI::newFrame(bool updateFrameGraph) {
    static bool enable_dock = Config::getConfig("enableDockSpace");
    static bool dock_space_open = true;
    static bool fullscreen = Config::getConfig("enableFullScreenPersistant");
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    if(enable_dock)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
    }
    ImGui::NewFrame();

    // Init imGui windows and elements

    if(enable_dock)
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", &dock_space_open, window_flags);
        ImGui::PopStyleVar();

        if (fullscreen) ImGui::PopStyleVar(2);

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        style.WindowMinSize.x = minWinSizeX;
    }

    for(auto& [id,task] : m_tasks)
    {
        task();
    }

//    ImGui::ShowDemoWindow();

    if(enable_dock)
    {
        ImGui::End();// End DockSpace
    }

    // Renderer to generate draw buffers
    ImGui::Render();
    if(enable_dock)
    {
        ImGui::UpdatePlatformWindows();
    }
}

void GUI::updateUI() {
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)getWindowViewPort().view.width,(float)getWindowViewPort().view.height);
    io.DeltaTime = 1.0f;

    glm::vec2& pos = input::getCursorWindow();
//        input::cursorWindow(pos,STATE_OUT);
    io.MousePos = ImVec2(pos.x, pos.y);

    io.MouseWheelH += getScrollPos().x;
    io.MouseWheel += getScrollPos().y;

    io.MouseDown[0] = checkMouse(GLFW_MOUSE_BUTTON_LEFT);
    io.MouseDown[1] = checkMouse(GLFW_MOUSE_BUTTON_RIGHT);
    io.MouseDown[2] = checkMouse(GLFW_MOUSE_BUTTON_MIDDLE);
//    if(ImGui::IsAnyItemActive() != anyItemActive) {
//        std::cout << "IsAnyItemActive:" << anyItemActive <<std::endl;
//    };
    anyItemActive = ImGui::IsAnyItemActive();
}

void GUI::init(float width, float height) {
    // Color scheme
    device = &Device::getDevice();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    // Dimensions
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    if(Config::getConfig("enableScreenGui") && !Config::getConfig("enableDockSpace"))
    {
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
    }

    ImGui::StyleColorsLight();

    pushUI("default", [&]() {
        ImGui::Begin("Setting");

        static char buf[32] = "1";
        ImGui::InputText("filename", buf, IM_ARRAYSIZE(buf));

        if (ImGui::Button("CaptureScreenShot"))
        {
            PushDelayAction([=]() {
                tool::saveScreenshot(std::string(buf) + ".ppm");
            }, []() {
                std::cout << "action!" << std::endl;
            });
        }

#ifdef SHATTER_GPU_CAPTURE
        if (ImGui::Button("CaptureObject"))
        {
            static bool captureObject = true;
            if(captureObject)
            {
                captureObject = false;
                SingleAPP.appendListener("CaptureObject",new CaptureObjectListener);
            } else {
                SingleAPP.deleteListener("CaptureObject");
                captureObject = true;
            }
        }
#endif

        if (ImGui::Button("DrawLine"))
        {
            static bool drawLine = true;
            if(drawLine)
            {
                drawLine = false;
                SingleAPP.appendListener("DrawLine",new DrawLine);
            } else {
                SingleAPP.deleteListener("DrawLine");
                drawLine = true;
            }
        }

        if (ImGui::Button("DrawPlane"))
        {
            static bool drawPlane = true;
            if(drawPlane)
            {
                drawPlane = false;
                SingleAPP.appendListener("DrawPlane",new DrawPlane);
            } else {
                SingleAPP.deleteListener("DrawPlane");
                drawPlane = true;
            }
        }

        if (ImGui::Button("DrawPoint"))
        {
            static bool drawPoint = true;
            if(drawPoint)
            {
                drawPoint = false;
                if (Config::getConfig("enableScreenGui"))
                {
                    auto pointHandle = new PointsHandle();
                    SingleRender.normalChanged = true;
                } else {
                    SingleAPP.appendListener("DrawPoint",new DrawPoint);
                }
            } else {
                SingleAPP.deleteListener("DrawPoint");
                drawPoint = true;
            }
        }

        if (ImGui::Button("DrawLinePool"))
        {
            static bool drawLinePool = true;
            if(drawLinePool)
            {
                drawLinePool = false;
                if (Config::getConfig("enableScreenGui"))
                {
                    auto lineHandle = new LineHandle();
                    SingleRender.normalChanged = true;
                } else {
                    SingleAPP.appendListener("drawLinePool", new DrawLinePool);
                }
            } else {
                SingleAPP.deleteListener("drawLinePool");
                drawLinePool = true;
            }
        }

        if (ImGui::Button("DrawNPlane"))
        {
            static bool drawNPlane = true;
            if(drawNPlane)
            {
                drawNPlane = false;
                if (Config::getConfig("enableScreenGui"))
                {
                    auto planeHandle = new DPlaneHandle();
                    SingleRender.normalChanged = true;
                } else {
                    SingleAPP.appendListener("drawNPlane", new DrawNPlane);
                }
            } else {
                SingleAPP.deleteListener("drawNPlane");
                drawNPlane = true;
            }
        }

        if (ImGui::Button("DrawCube"))
        {
            static bool drawCube = true;
            if(drawCube)
            {
                drawCube = false;
                if (Config::getConfig("enableScreenGui"))
                {
                    auto cubeHandle = new DCubeHandle();
                    SingleRender.normalChanged = true;
                } else {
                    SingleAPP.appendListener("drawCube",new DrawCube);
                }
            } else {
                SingleAPP.deleteListener("drawCube");
                drawCube = true;
            }
        }

        if (ImGui::Button("ChooseWorkPlane"))
        {
            static bool chooseWorkPlane = true;
            if(chooseWorkPlane)
            {
                chooseWorkPlane = false;
                SingleAPP.appendListener("chooseWorkPlane",new ChooseWorkPlane);
            } else {
                SingleAPP.deleteListener("chooseWorkPlane");
                chooseWorkPlane = true;
            }
        }

        ImGui::End();// End setting

        {
            ImGui::Begin("ViewPort");

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

            static bool firstDraw = true;
            if (viewportPanelSize.x != SingleAPP.getPresentViewPort().view.width || viewportPanelSize.y != SingleAPP.getPresentViewPort().view.height)
            {
                auto& view = SingleAPP.getPresentViewPort();
                view.view.width = viewportPanelSize.x;
                view.view.height = viewportPanelSize.y;
                view.scissor.extent = {static_cast<uint32_t>(view.view.width), static_cast<uint32_t>(view.view.height)};
                SingleAPP.setViewPortTouched(true);
                if (firstDraw) {
                    SingleAPP.viewportChanged = true;
                    firstDraw = false;
                }
            } else if (SingleAPP.getViewPortTouched()) {
                auto& view = SingleAPP.getPresentViewPort();
                view.view.width = viewportPanelSize.x;
                view.view.height = viewportPanelSize.y;
                view.scissor.extent = {static_cast<uint32_t>(view.view.width), static_cast<uint32_t>(view.view.height)};
                SingleAPP.viewportChanged = true;
                SingleAPP.setViewPortTouched(false);
            }

            ImGui::Image(SingleRender.m_colorSet, viewportPanelSize);

            ImGui::End();//ViewPort
        }
    });
}

GUI::~GUI() {
    if(Config::getConfig("enableDockSpace")) {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }
    ImGui::DestroyContext();
    // Release all Vulkan resources required for rendering imGui
    vertexBuffer.release();
    indexBuffer.release();
    vkDestroyImage(device->logicalDevice, fontImage, nullptr);
    vkDestroyImageView(device->logicalDevice, fontView, nullptr);
    vkFreeMemory(device->logicalDevice, fontMemory, nullptr);
    vkDestroySampler(device->logicalDevice, sampler, nullptr);
    vkDestroyPipelineCache(device->logicalDevice, pipelineCache, nullptr);
    vkDestroyPipeline(device->logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(device->logicalDevice, pipelineLayout, nullptr);
    vkDestroyDescriptorPool(device->logicalDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device->logicalDevice, descriptorSetLayout, nullptr);
}

void ShowHelpMarker(const char *desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
