//
// Created by maybe on 2020/12/19.
//

#ifndef SHATTER_ENGINE_SHATTERTEXTURE_H
#define SHATTER_ENGINE_SHATTERTEXTURE_H

#include "shatterbuffer.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include <string>
#include <fstream>

namespace shatter {
    namespace buffer {

        class ShatterTexture : public ShatterBuffer {
        public:
            static ShatterTexture* createTexture(const std::string &texture_name);

            static ShatterTexture* createTexture3d(int width, int height, int depth);

            static ShatterTexture* createTexture(ImageType _type,const std::string& _file);

            static ShatterTexture* createCubeTexture(const std::vector<std::string>& _files);

            static ShatterTexture* createVolumeTexture(const std::string&_file,int _width,int _height,int _depth);

            ShatterTexture();

            ~ShatterTexture();

//            bool initTextureKtx(const std::string &texture_name);
            bool initTextureKtx(std::string        filename,
                                VkFormat           format          = VK_FORMAT_R8G8B8A8_UNORM,
                                VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
                                VkImageLayout      imageLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                bool               forceLinear     = false);

            bool initTexture(const std::string &texture_name);

            void initCubeTexture(const std::vector<std::string>& _files);

            bool initTexture(ImageType _type, const std::string& _file);

            bool initVolumeTexture(const std::string&_file,int _width,int _height,int _depth);

            bool initDatTexture(const std::string&_file);

            bool initDDSTexture(const std::string& _file);

            bool initBnTexture(const std::string& _file);

            bool init3DTexture(int _width, int _height, int _depth);
        public:
            static VkImageView
            Create_ImageView(VkDevice *device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

            static void Create_Image(VkDevice *device,
                                     uint32_t width,
                                     uint32_t height,
                                     VkFormat format,
                                     VkImageTiling tiling,
                                     VkImageUsageFlags usage,
                                     VkMemoryPropertyFlags properties,
                                     VkImage &image,
                                     VkDeviceMemory &imageMemory);

            static void Transition_ImageLayout(VkImage image,
                                               VkFormat format,
                                               VkImageLayout oldLayout,
                                               VkImageLayout newLayout);

        protected:
            void create1dImage(uint32_t width,
                               VkFormat format,
                               VkImageTiling tiling,
                               VkImageUsageFlags usage,
                               VkMemoryPropertyFlags properties,
                               VkImage &image,
                               VkDeviceMemory &imageMemory);

            void create2dImage(uint32_t width,
                             uint32_t height,
                             VkFormat format,
                             VkImageTiling tiling,
                             VkImageUsageFlags usage,
                             VkMemoryPropertyFlags properties,
                             VkImage &image,
                             VkDeviceMemory &imageMemory);

            void create3dImage(uint32_t width,
                               uint32_t height,
                               uint32_t depth,
                               VkFormat format,
                               VkImageTiling tiling,
                               VkImageUsageFlags usage,
                               VkMemoryPropertyFlags properties,
                               VkImage &image,
                               VkDeviceMemory &imageMemory);

            void createCubeImage(uint32_t width,
                               uint32_t height,
                               VkFormat format,
                               VkImageTiling tiling,
                               VkImageUsageFlags usage,
                               VkMemoryPropertyFlags properties,
                               VkImage &image,
                               VkDeviceMemory &imageMemory);


            void transitionImageLayout(VkImage image,
                                       VkFormat format,
                                       VkImageLayout oldLayout,
                                       VkImageLayout newLayout);

            void transitionCubeImageLayout(VkImage image,
                                       VkFormat format,
                                       VkImageLayout oldLayout,
                                       VkImageLayout newLayout);


            void setImageLayout(
                    VkImage image,
                    VkImageAspectFlags aspectMask,
                    VkImageLayout oldImageLayout,
                    VkImageLayout newImageLayout,
                    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    uint32_t mipLevelCount = 1,
                    uint32_t layerCount = 1);

            void setCubeImageLayout(
                    VkImage image,
                    VkImageAspectFlags aspectMask,
                    VkImageLayout oldImageLayout,
                    VkImageLayout newImageLayout,
                    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            void setImageLayout(
                    VkImage image,
                    VkImageLayout oldImageLayout,
                    VkImageLayout newImageLayout,
                    VkImageSubresourceRange subresourceRange,
                    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

            void copyBufferTo3DImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth);

            static void copyBufferToCubeImage(int face,VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

            void copyBufferToImageLevels(VkBuffer buffer, VkImage image, std::vector<VkBufferImageCopy>& copy_vec,uint32_t width, uint32_t height);

            void prepareImage(ImageType _type,VkFormat _format,VkImageAspectFlags _aspectFlags);

            void create1dImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

            void create2dImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

            void create3dImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

            void createCubeImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

            void createTextureSampler();

        public:
            VkImageView getImageView() const { return m_textureImageView; };

            VkSampler getSampler() const { return m_textureSampler; };

            VkImage getImage() const{return m_textureImage;};

            void genImageInfo(VkImageLayout _layout) {
                m_ImageInfo.imageView = m_textureImageView;
                m_ImageInfo.sampler = m_textureSampler;
                m_ImageInfo.imageLayout = _layout;
            }
        public:
            VkDescriptorImageInfo m_ImageInfo;
            VkImage m_textureImage;
            VkImageView m_textureImageView;
            VkDeviceMemory m_memory{VK_NULL_HANDLE};
//            VkImageLayout ImageLayout;
            VkSampler m_textureSampler;
            uint32_t m_width,m_height;
            uint32_t mipLevels;
        };

    };
};




#endif //SHATTER_ENGINE_SHATTERTEXTURE_H
