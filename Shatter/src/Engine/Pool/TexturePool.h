//
// Created by jonnxie on 2022/4/16.
//

#ifndef MAIN_TEXTUREPOOL_H
#define MAIN_TEXTUREPOOL_H

#include <vulkan/vulkan.h>
#include "pool.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"

struct Texture{
    VkImage image{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkDescriptorSet set{VK_NULL_HANDLE};
};

class TexturePool : public Pool<std::string , Texture> {
public:
    static TexturePool& getPool();
    DefineUnCopy(TexturePool);

    void init() override;

    void release() override;

    void addTexture(const std::string& _key,
                    const std::string& _filename,
                    TextureType _type);

    void create2DTexture(const std::string& _key,
                         const std::string& _filename,
                         Texture& _tex);

private:
    TexturePool() = default;
};

#define SingleTexturePool TexturePool::getPool()

#endif //MAIN_TEXTUREPOOL_H
