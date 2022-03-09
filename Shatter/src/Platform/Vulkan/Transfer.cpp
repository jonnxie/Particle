//
// Created by jonnxie on 2022/3/8.
//

#include "Transfer.h"
#include "Engine/Renderer/renderer.h"

VkCommandBuffer beginSingleCommandBuffer() {
    return SingleRender.beginSingleTimeCommands();
}

void endSingleCommandBuffer(VkCommandBuffer cmd) {
    return SingleRender.endSingleTimeCommands(cmd);
}

VkCommandBuffer beginSingleTransCommandBuffer() {
    return SingleRender.beginSingleTimeTransCommands();
}

void endSingleTransCommandBuffer(VkCommandBuffer cmd) {
    return SingleRender.endSingleTimeTransCommands(cmd);
}
