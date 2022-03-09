//
// Created by jonnxie on 2022/3/8.
//

#ifndef MAIN_TRANSFER_H
#define MAIN_TRANSFER_H

#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleCommandBuffer();

void endSingleCommandBuffer(VkCommandBuffer cmd);

VkCommandBuffer beginSingleTransCommandBuffer();

void endSingleTransCommandBuffer(VkCommandBuffer cmd);



#endif //MAIN_TRANSFER_H
