//
// Created by AnWell on 2022/3/8.
//

#ifndef MAIN_TRANSFER_H
#define MAIN_TRANSFER_H

#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleTransCommandBuffer();

void endSingleTransCommandBuffer(VkCommandBuffer cmb);

#endif //MAIN_TRANSFER_H
