//
// Created by AnWell on 2021/9/14.
//

#ifndef SHATTER_ENGINE_OBJECTTASK_H
#define SHATTER_ENGINE_OBJECTTASK_H

#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
struct ThreadObject;

class ObjectTask {
public:
    static void computeTask(int _id,VkCommandBufferInheritanceInfo _inheritanceInfo,VkCommandBuffer* _cb);

    static void shadowDepthTask(int _threadIndex, int _id, VkCommandBufferInheritanceInfo _inheritanceInfo, VkCommandBuffer* _cb);

    static void newGraphicsTask(uint32_t _threadId, int _id, VkCommandBufferInheritanceInfo _inheritanceInfo, VkCommandBuffer* _cb);

    static void gTask(uint32_t _threadId, int _id, VkCommandBufferInheritanceInfo _inheritanceInfo, VkCommandBuffer* _cb);

    static void rayTracingTask(int _threadIndex,int _objectIndex,int _id,VkCommandBufferInheritanceInfo _inheritanceInfo,int _imageIndex);
};

#endif //SHATTER_ENGINE_OBJECTTASK_H
