//
// Created by AnWell on 2022/3/1.
//

#ifndef MAIN_FRAMEBUFFER_H
#define MAIN_FRAMEBUFFER_H

#include <utility>

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "FormatFilter.hpp"

struct FrameBufferSpecification
{
    uint32_t Width = 0,
    Height = 0;
    std::vector<AttachFormat> formats{};
    uint32_t Samples = 1;

    bool SwapChainTarget = false;
#ifdef SHATTER_GRAPHICS_VULKAN
    VkRenderPass RenderPass{};
#endif
};


class FrameBuffer{
public:
    static FrameBuffer* createFramebuffer(FrameBufferSpecification _spec);
    explicit FrameBuffer(FrameBufferSpecification _spec):
    m_spec(std::move(_spec))
    {};
    virtual ~FrameBuffer()= default;;
public:
    virtual void resize(uint32_t _width,uint32_t _height) = 0;
    virtual void release() = 0;

protected:
    FrameBufferSpecification m_spec;
};


#endif //MAIN_FRAMEBUFFER_H
