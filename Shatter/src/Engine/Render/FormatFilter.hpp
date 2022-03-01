//
// Created by AnWell on 2022/3/1.
//

#ifndef MAIN_FORMATFILTER_HPP
#define MAIN_FORMATFILTER_HPP

enum class TextureFormat{
    RGBA8U,

    DEPTH24STENCIL8,
    Depth = DEPTH24STENCIL8
};

enum class Usage{
    Color,
    Depth
};

struct AttachFormat{
    TextureFormat format;
    Usage usage;
};

#endif //MAIN_FORMATFILTER_HPP
