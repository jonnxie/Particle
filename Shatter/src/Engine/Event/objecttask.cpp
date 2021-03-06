//
// Created by AnWell on 2021/9/14.
//
#include "precompiledhead.h"

#include "objecttask.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/pool/mpool.h"
#include "Engine/Object/dobject.h"
#include "Engine/Object/cobject.h"
#include "Engine/Object/device.h"

void ObjectTask::rayTracingTask(int _threadIndex,int _objectIndex,int _id,VkCommandBufferInheritanceInfo _inheritanceInfo,int _imageIndex)
{
    auto threadPool = getThreadObjectPool();
    ThreadObject threadObject = (*threadPool)[_threadIndex];

    auto drawPool = MPool<DObject>::getPool();
    DObject drawObject = *(*drawPool)[_id];
    if(drawObject.m_type == DType::RayTracing)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        commandBufferBeginInfo.pInheritanceInfo = &_inheritanceInfo;

        VkCommandBuffer cmdBuffer = threadObject.buffers[_objectIndex];

        VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo));


        drawObject.draw(cmdBuffer,_imageIndex);
        VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
        (*threadPool)[_threadIndex].visibility[_objectIndex] = true;
    }
}


void ObjectTask::computeTask(int _id, VkCommandBufferInheritanceInfo _inheritanceInfo, VkCommandBuffer* _cb) {
    VkCommandPool pool = getCommandPool(CommandPoolType::GraphicsPool);

    auto computePool = MPool<CObject>::getPool();
    CObject* computeObject = (*computePool)[_id];

    VkCommandBufferBeginInfo commandBufferBeginInfo {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = &_inheritanceInfo;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
    commandBufferAllocateInfo.commandPool = pool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    commandBufferAllocateInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(SingleDevice.logicalDevice,&commandBufferAllocateInfo,_cb);

    auto cb = *_cb;

    VK_CHECK_RESULT(vkBeginCommandBuffer(cb, &commandBufferBeginInfo));

    computeObject->compute(cb);

    VK_CHECK_RESULT(vkEndCommandBuffer(cb));
}

void ObjectTask::shadowDepthTask(int _threadIndex, int _id, VkCommandBufferInheritanceInfo _inheritanceInfo, VkCommandBuffer* _cb)
{
    auto threadPool = getThreadCommandPool();
    VkCommandPool pool = (*threadPool)[_threadIndex].graphicsPool;

    auto DPool = MPool<DObject>::getPool();
    DObject* d = (*DPool)[_id];

    VkCommandBufferBeginInfo commandBufferBeginInfo {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = &_inheritanceInfo;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
    commandBufferAllocateInfo.commandPool = pool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(SingleDevice.logicalDevice,&commandBufferAllocateInfo,_cb);

    VK_CHECK_RESULT(vkBeginCommandBuffer(*_cb, &commandBufferBeginInfo));

    d->m_drawDepth(*_cb);

    VK_CHECK_RESULT(vkEndCommandBuffer(*_cb));
}

void ObjectTask::newGraphicsTask(uint32_t _threadId, int _id, VkCommandBufferInheritanceInfo _inheritanceInfo, VkCommandBuffer* _cb)
{
    VkCommandPool pool = getCommandPool(CommandPoolType::GraphicsPool, _threadId);

    auto DPool = MPool<DObject>::getPool();
    DObject* d = (*DPool)[_id];

    VkCommandBufferBeginInfo commandBufferBeginInfo {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = &_inheritanceInfo;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
    commandBufferAllocateInfo.commandPool = pool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(SingleDevice.logicalDevice,&commandBufferAllocateInfo,_cb);

    d->drawCMB = *_cb;

    VK_CHECK_RESULT(vkBeginCommandBuffer(*_cb, &commandBufferBeginInfo));

    d->m_newDraw(*_cb);

    VK_CHECK_RESULT(vkEndCommandBuffer(*_cb));
}

void ObjectTask::gTask(uint32_t _threadId, int _id, VkCommandBufferInheritanceInfo _inheritanceInfo, VkCommandBuffer* _cb)
{
    VkCommandPool pool = getCommandPool(CommandPoolType::GraphicsPool, _threadId);

    auto DPool = MPool<DObject>::getPool();
    DObject* d = (*DPool)[_id];

    VkCommandBufferBeginInfo commandBufferBeginInfo {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = &_inheritanceInfo;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
    commandBufferAllocateInfo.commandPool = pool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(SingleDevice.logicalDevice, &commandBufferAllocateInfo, _cb);

    d->gCMB = *_cb;

    VK_CHECK_RESULT(vkBeginCommandBuffer(*_cb, &commandBufferBeginInfo));

    d->m_gGraphics(*_cb);

    VK_CHECK_RESULT(vkEndCommandBuffer(*_cb));
}

