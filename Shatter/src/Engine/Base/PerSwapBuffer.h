//
// Created by jonnxie on 2022/5/8.
//

#ifndef MAIN_PERSWAPBUFFER_H
#define MAIN_PERSWAPBUFFER_H

#include <iostream>
#include <vulkan/vulkan.h>
#include "Engine/Item/shatter_enum.h"

class PerSwapBuffer {
public:
    static auto getSwapBuffer(const std::string _name);
    static void createSwapBuffer(const std::string _name, VkDeviceSize _size, Buffer_Type _type, void* data = nullptr);
};


#endif //MAIN_PERSWAPBUFFER_H
