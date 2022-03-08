//
// Created by AnWell on 2022/3/8.
//

#include "Transfer.h"
#include "Engine/Renderer/renderer.h"

VkCommandBuffer beginSingleTransCommandBuffer() {
    return SingleRender.beginSingleTimeTransCommands();
}

void endSingleTransCommandBuffer(VkCommandBuffer cmb) {
    return SingleRender.endSingleTimeTransCommands(cmb);
}
