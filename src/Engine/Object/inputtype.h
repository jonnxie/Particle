//
// Created by maybe on 2021/6/5.
//

#ifndef SHATTER_ENGINE_INPUTTYPE_H
#define SHATTER_ENGINE_INPUTTYPE_H

#include <vector>
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"

VkVertexInputBindingDescription getBindingDescription(Input_Type _type);

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(Input_Type _type);

#endif //SHATTER_ENGINE_INPUTTYPE_H
