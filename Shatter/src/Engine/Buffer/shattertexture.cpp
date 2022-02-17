//
// Created by maybe on 2020/12/19.
//
#include "precompiledhead.h"

#include "shattertexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Engine/Render/render.h"
#include "Engine/Item/shatter_macro.h"
#include DeviceCatalog
#include <thread>
#include <mutex>
#include <array>
#include <sstream>
#include <iostream>
#include <cstdio>

using namespace std;

namespace shatter::buffer{

    static std::once_flag stage_flag;

    ShatterTexture* ShatterTexture::createTexture(const std::string &texture_name){
        auto texture = new ShatterTexture;
        texture->Set_Device(render::ShatterRender::getRender().getDevice());
        if(texture->initTexture(texture_name)){
            return texture;
        }else{
            throw std::runtime_error("create texture error!");
        }
    }

    ShatterTexture *ShatterTexture::createTexture(ImageType _type, const std::string &_file) {
        auto texture = new ShatterTexture;
        if(_type == ImageType::OneDATDimension){
            texture->initDatTexture(_file);
        }else{
            texture->initTexture(_type,_file);
        }
        return texture;
    }

    bool ShatterTexture::initTexture(ImageType _type, const std::string &_file) {
        int texWidth, texHeight, texChannels;
        if (!file::fileExists(_file)) {
            throwFile(_file);
        }
        if(_type == ImageType::DDSDimension)
        {
            return initDDSTexture(_file);
        }
        if(_type == ImageType::BntexDimension)
        {
            return initBnTexture(_file);
        }
        if(_type == ImageType::KTXDimension)
        {
            return initTextureKtx(_file);
        }
        stbi_uc *pixels = stbi_load(_file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(imageSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);
        void *data;       vkMapMemory(*m_device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(*m_device, stagingBufferMemory);
        stbi_image_free(pixels);

        switch (_type) {
            case ImageType::OneDimension:{
                if (!pixels) {
                    throw std::runtime_error("failed to load texture image!");
                }
                create1dImage(texWidth,
                              VK_FORMAT_R8G8B8A8_UNORM,
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              m_textureImage,
                              m_memory);

                transitionImageLayout(m_textureImage,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                copyBufferToImage(stagingBuffer,
                                  m_textureImage,
                                  static_cast<uint32_t>(texWidth),
                                  static_cast<uint32_t>(texHeight));
                transitionImageLayout(m_textureImage,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                prepareImage(_type,VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_ASPECT_COLOR_BIT);
                break;
            }
            case ImageType::TwoDimension:{
                if (!pixels) {
                    throw std::runtime_error("failed to load texture image!");
                }
                create2dImage(texWidth,
                              texHeight,
                              VK_FORMAT_R8G8B8A8_UNORM,
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              m_textureImage,
                              m_memory);

                transitionImageLayout(m_textureImage,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                copyBufferToImage(stagingBuffer,
                                  m_textureImage,
                                  static_cast<uint32_t>(texWidth),
                                  static_cast<uint32_t>(texHeight));
                transitionImageLayout(m_textureImage,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                prepareImage(_type,VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_ASPECT_COLOR_BIT);
                break;
            }

            default:{
                WARNING(no such image type!);
                break;
            }
        }
        vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
        vkFreeMemory(*m_device, stagingBufferMemory, nullptr);
        return true;
    }

    bool ShatterTexture::initVolumeTexture(const std::string&_file,int _width,int _height,int _depth)
    {
        if (!file::fileExists(_file)) {
            throwFile(_file);
        }
        FILE *fp;
        size_t size = _width * _height * _depth;
        GLubyte *data = new GLubyte[size];			  // 8bit
        char    c_file[200];							//文件路径
        strcpy(c_file, _file.c_str());					//将文件路径转换成字符数组
        if (!(fp = fopen(c_file, "rb")))
        {
            cout << "Error: opening .raw file failed" << endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            cout << "OK: open .raw file successed" << endl;
        }
        if ( fread(data, sizeof(char), size, fp)!= size)
        {
            cout << "Error: read .raw file failed" << endl;
            exit(1);
        }
        else
        {
            cout << "OK: read .raw file successed" << endl;
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkDeviceSize imageSize = _width * _height * _depth;

        createBuffer(imageSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);

        create3dImage(_width,
                      _height,
                      _depth,
                      VK_FORMAT_R8_UNORM,
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      m_textureImage,
                      m_memory);
//        transitionImageLayout(m_textureImage,
//                                  VK_FORMAT_R8_UNORM,
//                                  VK_IMAGE_LAYOUT_UNDEFINED,
//                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        setImageLayout(m_textureImage,VK_IMAGE_ASPECT_COLOR_BIT,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        void* mapData;
        vkMapMemory(*m_device, stagingBufferMemory, 0, imageSize, 0, &mapData);
        memcpy(mapData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(*m_device, stagingBufferMemory);
        copyBufferTo3DImage(stagingBuffer, m_textureImage, _width, _height, _depth);

        vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
        vkFreeMemory(*m_device, stagingBufferMemory, nullptr);
//        setCubeImageLayout(m_textureImage,
//                           VK_IMAGE_ASPECT_COLOR_BIT,
//                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        setImageLayout(m_textureImage,VK_IMAGE_ASPECT_COLOR_BIT,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        create3dImageView(m_textureImage, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
        createTextureSampler();
        fclose(fp);
        delete []data;
        cout << "volume texture created" << endl;
        return true;
    }

    bool ShatterTexture::initDatTexture(const std::string&_file)
    {
        if (!file::fileExists(_file)) {
            throwFile(_file);
        }
        char    filename[200];							//文件路径
        strcpy(filename, _file.c_str());

        ifstream inFile(filename, ifstream::in);
        if (!inFile)
        {
            cerr << "Error openning file: " << filename << endl;
            exit(EXIT_FAILURE);
        }

        const int MAX_CNT = 10000;
        GLubyte *tff = (GLubyte *) calloc(MAX_CNT, sizeof(GLubyte));
        inFile.read(reinterpret_cast<char *>(tff), MAX_CNT);
        {
            if (inFile.eof())
            {
                size_t bytecnt = inFile.gcount();
                *(tff + bytecnt) = '\0';
                cout << "bytecnt " << bytecnt << endl;
            }
            else if(inFile.fail())
            {
                cout << filename << "read failed " << endl;
            }
            else
            {
                cout << filename << "is too large" << endl;
            }
        }

        VkDeviceSize imageSize = 256 * 4;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(imageSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);
        void *data;
        vkMapMemory(*m_device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, tff, static_cast<size_t>(imageSize));
        vkUnmapMemory(*m_device, stagingBufferMemory);

        create1dImage(256,
                      VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      m_textureImage,
                      m_memory);

        transitionImageLayout(m_textureImage,
                              VK_FORMAT_R8G8B8A8_UNORM,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer,
                          m_textureImage,
                          256,
                          1);
        transitionImageLayout(m_textureImage,
                              VK_FORMAT_R8G8B8A8_UNORM,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        prepareImage(ImageType::OneDimension,VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_ASPECT_COLOR_BIT);
        vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
        vkFreeMemory(*m_device, stagingBufferMemory, nullptr);
        free(tff);
        return true;
    }

    bool ShatterTexture::initDDSTexture(const std::string& _file){
        if (!file::fileExists(_file)) {
            throwFile(_file);
        }

        file::TexDataObject* ctdo = file::load_DXT5_BC3_TexData(_file);

        auto memoryroperties = getVkPhysicalDeviceMemoryProperties();

        auto format = VK_FORMAT_BC3_UNORM_BLOCK;

        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(SingleDevice.physicalDevice, format, &formatProps);
        bool needStaging = (!(formatProps.linearTilingFeatures &VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) ? true : false;
//        printf("TextureManager %s", (needStaging ? "can not use linear tiling texture" : "can not use linear tiling texture"));
        if (needStaging)
        {
            VkBuffer tempBuf;
            VkBufferCreateInfo buf_info = {};
            buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buf_info.pNext = NULL;
            buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buf_info.size = ctdo->dataByteCount;
            buf_info.queueFamilyIndexCount = 0;
            buf_info.pQueueFamilyIndices = NULL;
            buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buf_info.flags = 0;
            VkResult result = vkCreateBuffer(SingleDevice.logicalDevice, &buf_info, NULL, &tempBuf);
            assert(result == VK_SUCCESS);
            VkMemoryRequirements mem_reqs;
            vkGetBufferMemoryRequirements(SingleDevice.logicalDevice, tempBuf, &mem_reqs);
            assert(ctdo->dataByteCount <= mem_reqs.size);
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.pNext = NULL;
            alloc_info.memoryTypeIndex = 0;
            alloc_info.allocationSize = mem_reqs.size;
            VkFlags requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            bool flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, requirements_mask, &alloc_info.memoryTypeIndex);
            if (flag)
            {
                printf("make sure memory type index %d", alloc_info.memoryTypeIndex);
            }
            else
            {
                printf("make sure memory type index filed");
            }
            VkDeviceMemory memTemp;
            result = vkAllocateMemory(SingleDevice.logicalDevice, &alloc_info, NULL, &memTemp);
            assert(result == VK_SUCCESS);
            uint8_t *pData;
            result = vkMapMemory(SingleDevice.logicalDevice, memTemp, 0, mem_reqs.size, 0, (void **)&pData);
            assert(result == VK_SUCCESS);
            memcpy(pData, ctdo->data, ctdo->dataByteCount);
            vkUnmapMemory(SingleDevice.logicalDevice, memTemp);
            result = vkBindBufferMemory(SingleDevice.logicalDevice, tempBuf, memTemp, 0);
            assert(result == VK_SUCCESS);
            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.pNext = NULL;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = format;
            image_create_info.extent.width = ctdo->width;
            image_create_info.extent.height = ctdo->height;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            image_create_info.queueFamilyIndexCount = 0;
            image_create_info.pQueueFamilyIndices = NULL;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.flags = 0;
            VkImage textureImage;
            result = vkCreateImage(SingleDevice.logicalDevice, &image_create_info, NULL, &textureImage);
            assert(result == VK_SUCCESS);
            m_textureImage = textureImage;
            VkMemoryAllocateInfo mem_alloc = {};
            mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc.pNext = NULL;
            mem_alloc.allocationSize = 0;
            mem_alloc.memoryTypeIndex = 0;
            vkGetImageMemoryRequirements(SingleDevice.logicalDevice, textureImage, &mem_reqs);
            mem_alloc.allocationSize = mem_reqs.size;
            flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex);
            VkDeviceMemory textureMemory;
            result = vkAllocateMemory(SingleDevice.logicalDevice, &mem_alloc, NULL, &(textureMemory));
            m_memory = textureMemory;
            result = vkBindImageMemory(SingleDevice.logicalDevice, textureImage, textureMemory, 0);

            copyBufferToImage(tempBuf,m_textureImage,ctdo->width,ctdo->height);

            vkDestroyBuffer(SingleDevice.logicalDevice, tempBuf, NULL);
            vkFreeMemory(SingleDevice.logicalDevice, memTemp, NULL);
        }
        else
        {
            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.pNext = NULL;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = format;
            image_create_info.extent.width = ctdo->width;
            image_create_info.extent.height = ctdo->height;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.queueFamilyIndexCount = 0;
            image_create_info.pQueueFamilyIndices = NULL;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.flags = 0;
            VkImage textureImage;
            VkResult result = vkCreateImage(SingleDevice.logicalDevice, &image_create_info, NULL, &textureImage);
            assert(result == VK_SUCCESS);
            m_textureImage = textureImage;
            VkMemoryAllocateInfo mem_alloc = {};
            mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc.pNext = NULL;
            mem_alloc.allocationSize = 0;
            mem_alloc.memoryTypeIndex = 0;
            VkMemoryRequirements mem_reqs;
            vkGetImageMemoryRequirements(SingleDevice.logicalDevice, textureImage, &mem_reqs);
            mem_alloc.allocationSize = mem_reqs.size;
            bool flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
            VkDeviceMemory textureMemory;
            result = vkAllocateMemory(SingleDevice.logicalDevice, &mem_alloc, NULL, &(textureMemory));
            m_memory = textureMemory;
            result = vkBindImageMemory(SingleDevice.logicalDevice, textureImage, textureMemory, 0);
            uint8_t *pData;
            vkMapMemory(SingleDevice.logicalDevice, textureMemory, 0, mem_reqs.size, 0, (void**)(&pData));
            memcpy(pData, ctdo->data, mem_reqs.size);
            vkUnmapMemory(SingleDevice.logicalDevice, textureMemory);
        }
        create2dImageView(m_textureImage,format,VK_IMAGE_ASPECT_COLOR_BIT);
        delete ctdo;
        return true;
    }

    bool ShatterTexture::initBnTexture(const std::string& _file){
        if (!file::fileExists(_file)) {
            throwFile(_file);
        }
        file::TexDataObject* ctdo = file::loadCommonTexData(_file);

        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(SingleDevice.physicalDevice,VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
        bool needStaging = (!(formatProps.linearTilingFeatures &VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) ? true : false;
//        printf("TextureManager %s\n", (needStaging ? "不能使用线性瓦片纹理" : "能使用线性瓦片纹理"));
//        if (needStaging)
//        {
            VkBuffer tempBuf;
            VkBufferCreateInfo buf_info = {};
            buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buf_info.pNext = NULL;
            buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buf_info.size = ctdo->dataByteCount;
            buf_info.queueFamilyIndexCount = 0;
            buf_info.pQueueFamilyIndices = NULL;
            buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buf_info.flags = 0;
            VkResult result = vkCreateBuffer(SingleDevice(), &buf_info, NULL, &tempBuf);
            assert(result == VK_SUCCESS);
            VkMemoryRequirements mem_reqs;
            vkGetBufferMemoryRequirements(SingleDevice(), tempBuf, &mem_reqs);
            assert(ctdo->dataByteCount <= mem_reqs.size);
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.pNext = NULL;
            alloc_info.memoryTypeIndex = 0;
            alloc_info.allocationSize = mem_reqs.size;
            VkFlags requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            VkPhysicalDeviceMemoryProperties prop = getVkPhysicalDeviceMemoryProperties();
            bool flag = memoryTypeFromProperties(prop, mem_reqs.memoryTypeBits, requirements_mask, &alloc_info.memoryTypeIndex);
            if (flag)
            {
//                printf("确定内存类型成功 类型索引为%d\n", alloc_info.memoryTypeIndex);
            }
            else
            {
                printf("确定内存类型失败!\n");
            }
            VkDeviceMemory memTemp;
            result = vkAllocateMemory(SingleDevice(), &alloc_info, NULL, &memTemp);
            assert(result == VK_SUCCESS);
            uint8_t *pData;
            result = vkMapMemory(SingleDevice(), memTemp, 0, mem_reqs.size, 0, (void **)&pData);
            assert(result == VK_SUCCESS);
            memcpy(pData, ctdo->data, ctdo->dataByteCount);
            vkUnmapMemory(SingleDevice(), memTemp);
            result = vkBindBufferMemory(SingleDevice(), tempBuf, memTemp, 0);
            assert(result == VK_SUCCESS);
            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.pNext = NULL;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
            image_create_info.extent.width = ctdo->width;
            image_create_info.extent.height = ctdo->height;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            image_create_info.queueFamilyIndexCount = 0;
            image_create_info.pQueueFamilyIndices = NULL;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.flags = 0;
            VkImage textureImage;
            result = vkCreateImage(SingleDevice(), &image_create_info, NULL, &textureImage);
            assert(result == VK_SUCCESS);
            m_textureImage = textureImage;
            VkMemoryAllocateInfo mem_alloc = {};
            mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc.pNext = NULL;
            mem_alloc.allocationSize = 0;
            mem_alloc.memoryTypeIndex = 0;
            vkGetImageMemoryRequirements(SingleDevice(), textureImage, &mem_reqs);
            mem_alloc.allocationSize = mem_reqs.size;
            memoryTypeFromProperties(prop, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex);
            VkDeviceMemory textureMemory;
            result = vkAllocateMemory(SingleDevice(), &mem_alloc, NULL, &(textureMemory));
            assert(result == VK_SUCCESS);
            m_memory = textureMemory;
            result = vkBindImageMemory(SingleDevice(), textureImage, textureMemory, 0);
            assert(result == VK_SUCCESS);

            transitionImageLayout(m_textureImage,
                                  VK_FORMAT_R8G8B8A8_UNORM,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(tempBuf,
                              m_textureImage,
                              static_cast<uint32_t>(ctdo->width),
                              static_cast<uint32_t>(ctdo->height));
            transitionImageLayout(m_textureImage,
                                  VK_FORMAT_R8G8B8A8_UNORM,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            vkDestroyBuffer(SingleDevice(), tempBuf, NULL);
            vkFreeMemory(SingleDevice(), memTemp, NULL);
//        }
//        else
//        {
//            VkImageCreateInfo image_create_info = {};
//            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//            image_create_info.pNext = NULL;
//            image_create_info.imageType = VK_IMAGE_TYPE_2D;
//            image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
//            image_create_info.extent.width = ctdo->width;
//            image_create_info.extent.height = ctdo->height;
//            image_create_info.extent.depth = 1;
//            image_create_info.mipLevels = 1;
//            image_create_info.arrayLayers = 1;
//            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
//            image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
//            image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
//            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
//            image_create_info.queueFamilyIndexCount = 0;
//            image_create_info.pQueueFamilyIndices = NULL;
//            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//            image_create_info.flags = 0;
//            VkImage textureImage;
//            VkResult result = vkCreateImage(SingleDevice(), &image_create_info, NULL, &textureImage);
//            assert(result == VK_SUCCESS);
//            m_textureImage = textureImage;
//            VkMemoryAllocateInfo mem_alloc = {};
//            mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//            mem_alloc.pNext = NULL;
//            mem_alloc.allocationSize = 0;
//            mem_alloc.memoryTypeIndex = 0;
//            VkMemoryRequirements mem_reqs;
//            vkGetImageMemoryRequirements(SingleDevice(), textureImage, &mem_reqs);
//            mem_alloc.allocationSize = mem_reqs.size;
//            VkPhysicalDeviceMemoryProperties prop = getVkPhysicalDeviceMemoryProperties();
//            bool flag = memoryTypeFromProperties(prop , mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
//            VkDeviceMemory textureMemory;
//            result = vkAllocateMemory(SingleDevice(), &mem_alloc, NULL, &(textureMemory));
//            m_memory = textureMemory;
//            result = vkBindImageMemory(SingleDevice(), textureImage, textureMemory, 0);
//            uint8_t *pData;
//            vkMapMemory(SingleDevice(), textureMemory, 0, mem_reqs.size, 0, (void**)(&pData));
//            memcpy(pData, ctdo->data, mem_reqs.size);
//            vkUnmapMemory(SingleDevice(), textureMemory);
//        }
        create2dImageView(m_textureImage,VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_ASPECT_COLOR_BIT);
        createTextureSampler();
        delete ctdo;
        return true;
    }

    void ShatterTexture::prepareImage(ImageType _type,VkFormat _format,VkImageAspectFlags _aspectFlags)
    {
        switch (_type) {
            case ImageType::OneDimension:{
                create1dImageView(m_textureImage, _format, _aspectFlags);
                break;
            }
            case ImageType::TwoDimension:{
                create2dImageView(m_textureImage, _format, _aspectFlags);
                break;
            }
            case ImageType::ThreeDimension:{
                create3dImageView(m_textureImage, _format, _aspectFlags);
                break;
            }
            case ImageType::CubeDimension:{
                createCubeImageView(m_textureImage, _format, _aspectFlags);
                break;
            }
            default:{
                WARNING(no such image type!);
                break;
            }
        }
        createTextureSampler();
    }


    ShatterTexture* ShatterTexture::createTexture3d(int width, int height, int depth){
        auto texture = new ShatterTexture;
        texture->Set_Device(render::ShatterRender::getRender().getDevice());
        if(texture->init3DTexture(width, height, depth)){
            return texture;
        }else{
            throw std::runtime_error("create texture error!");
        }
    }

    bool ShatterTexture::initTexture(const std::string& texture_name){
        bool val = false;
        do{
            int texWidth, texHeight, texChannels;
            if (!file::fileExists(texture_name)) {
                throwFile(texture_name);
            }
            stbi_uc *pixels = stbi_load(texture_name.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            VkDeviceSize imageSize = texWidth * texHeight * 4;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(imageSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);

            void *data;
            vkMapMemory(*m_device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(*m_device, stagingBufferMemory);

            stbi_image_free(pixels);

            create1dImage(texWidth,
                          VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                          m_textureImage,
                          m_memory);

            transitionImageLayout(m_textureImage,
                                  VK_FORMAT_R8G8B8A8_UNORM,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer,
                              m_textureImage,
                              static_cast<uint32_t>(texWidth),
                              static_cast<uint32_t>(texHeight));
            transitionImageLayout(m_textureImage,
                                  VK_FORMAT_R8G8B8A8_UNORM,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
            vkFreeMemory(*m_device, stagingBufferMemory, nullptr);

            create1dImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
            createTextureSampler();

            val = true;
        }while(false);
        return val;
    }

    bool ShatterTexture::init3DTexture(int _width, int _height, int _depth){
        bool val = false;
        do{
            VkDeviceSize imageSize = _width * _height * _depth * 4;

            VkFormatProperties formatProperties;

            // Get device properties for the requested texture format
            vkGetPhysicalDeviceFormatProperties(render::ShatterRender::getRender().getPhysicalDevice(),
                                                VK_FORMAT_R8G8B8A8_UNORM,
                                                &formatProperties);
            // Check if requested image format supports image storage operations
            assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

            create3dImage(_width,
                          _height,
                          _depth,
                        VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_SAMPLED_BIT|
                        VK_IMAGE_USAGE_STORAGE_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        m_textureImage,
                        m_memory);

            setImageLayout(
                    m_textureImage,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_GENERAL);

            create3dImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
            createTextureSampler();

            val = true;
        }while(false);
        return val;
    }

    ShatterTexture::ShatterTexture():
    m_textureImage{VK_NULL_HANDLE},
    m_textureImageView{VK_NULL_HANDLE},
    m_textureSampler{VK_NULL_HANDLE}
    {
        m_device = &Device::getDevice().logicalDevice;
    }

    ShatterTexture::~ShatterTexture() {
        vkDestroySampler(*m_device, m_textureSampler, nullptr);
        vkDestroyImageView(*m_device, m_textureImageView, nullptr);
        if(m_memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(*m_device, m_memory, nullptr);
        }

        vkDestroyImage(*m_device, m_textureImage, nullptr);
    }


    void ShatterTexture::create1dImage(uint32_t width,
                                       VkFormat format,
                                       VkImageTiling tiling,
                                       VkImageUsageFlags usage,
                                       VkMemoryPropertyFlags properties,
                                       VkImage &image,
                                       VkDeviceMemory &imageMemory
                                      ){
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_1D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = 1;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(*m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create iamge!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(*m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = render::ShatterRender::getRender().findMemoryType(memRequirements.memoryTypeBits,
                                                                                      properties);

        if (vkAllocateMemory(*m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(*m_device, image, imageMemory, 0);
    }

    void ShatterTexture::create2dImage(uint32_t width,
                       uint32_t height,
                       VkFormat format,
                       VkImageTiling tiling,
                       VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       VkImage &image,
                       VkDeviceMemory &imageMemory){
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(*m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create iamge!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(*m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = render::ShatterRender::getRender().findMemoryType(memRequirements.memoryTypeBits,
                                                                                      properties);

        if (vkAllocateMemory(*m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(*m_device, image, imageMemory, 0);
    }

    void ShatterTexture::create3dImage(uint32_t width,
                                       uint32_t height,
                                       uint32_t depth,
                                       VkFormat format,
                                       VkImageTiling tiling,
                                       VkImageUsageFlags usage,
                                       VkMemoryPropertyFlags properties,
                                       VkImage &image,
                                       VkDeviceMemory &imageMemory){
        VkImageCreateInfo imageInfo = {};

        std::array<uint32_t,2> queueFamilyIndices = {uint32_t(getIndices().computeFamily),uint32_t(getIndices().graphicsFamily)};

        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.format = format;
        imageInfo.extent = {width,height,depth};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//        imageInfo.queueFamilyIndexCount =  getIndices().graphicsFamily == getIndices().computeFamily ? 1 : 2;
//        imageInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(*m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create iamge!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(*m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = render::ShatterRender::getRender().findMemoryType(memRequirements.memoryTypeBits,
                                                                                      properties);

        if (vkAllocateMemory(*m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(*m_device, image, imageMemory, 0);
    }

    void ShatterTexture::createCubeImage(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage &image,
                         VkDeviceMemory &imageMemory)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = VK_NULL_HANDLE;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.extent = {width,height,1};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(*m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create iamge!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(*m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = render::ShatterRender::getRender().findMemoryType(memRequirements.memoryTypeBits,
                                                                                      properties);

        if (vkAllocateMemory(*m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(*m_device, image, imageMemory, 0);
    }

    ShatterTexture *ShatterTexture::createCubeTexture(const std::vector<std::string>& _files) {
        auto texture = new ShatterTexture;
        texture->initCubeTexture(_files);
        return texture;
    }

    ShatterTexture* ShatterTexture::createVolumeTexture(const std::string&_file,int _width,int _height,int _depth){
        auto texture = new ShatterTexture;
        texture->initVolumeTexture(_file,_width,_height,_depth);
        return texture;
    }


    void ShatterTexture::initCubeTexture(const std::vector<std::string>& _files)
    {
        int texWidth, texHeight, texChannels;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        void *data;

        for(int index = 0; index < 6; index++)
        {
            if (!file::fileExists(_files[index])) {
                throwFile(_files[index]);
            }
            stbi_uc *pixels = stbi_load(_files[index].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            VkDeviceSize imageSize = texWidth * texHeight * 4;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }
            if(index == 0){
                createBuffer(imageSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             stagingBuffer,
                             stagingBufferMemory);
                createCubeImage(texWidth,
                                texHeight,
                                VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                m_textureImage,
                                m_memory);
                transitionCubeImageLayout(m_textureImage,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            }
            vkMapMemory(*m_device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(*m_device, stagingBufferMemory);
            stbi_image_free(pixels);
            copyBufferToCubeImage(index,stagingBuffer,m_textureImage,static_cast<uint32_t>(texWidth),static_cast<uint32_t>(texHeight));
        }

        vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
        vkFreeMemory(*m_device, stagingBufferMemory, nullptr);
        setCubeImageLayout(m_textureImage,
                       VK_IMAGE_ASPECT_COLOR_BIT,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        createCubeImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
        createTextureSampler();
    }


    void ShatterTexture::Create_Image(VkDevice* device,
                                      uint32_t width,
                                      uint32_t height,
                                      VkFormat format,
                                      VkImageTiling tiling,
                                      VkImageUsageFlags usage,
                                      VkMemoryPropertyFlags properties,
                                      VkImage &image,
                                      VkDeviceMemory &imageMemory){
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(*device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create iamge!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(*device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = render::ShatterRender::getRender().findMemoryType(memRequirements.memoryTypeBits,
                                                                                      properties);

        if (vkAllocateMemory(*device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(*device, image, imageMemory, 0);
    }

    void ShatterTexture::transitionImageLayout(VkImage image,
                                               VkFormat format,
                                               VkImageLayout oldLayout,
                                               VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (::hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    }

    void ShatterTexture::transitionCubeImageLayout(VkImage image,
                                   VkFormat format,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout){
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (::hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 6;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    }

    void ShatterTexture::setImageLayout(
            VkImage image,
            VkImageAspectFlags aspectMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            uint32_t mipLevelCount,
            uint32_t layerCount){
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = mipLevelCount;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = layerCount;
        setImageLayout(image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
    }

    void ShatterTexture::setCubeImageLayout(
            VkImage image,
            VkImageAspectFlags aspectMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask){
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 6;
        setImageLayout(image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
    }

    void ShatterTexture::setImageLayout(
            VkImage image,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask){
        VkCommandBuffer cmdbuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkImageMemoryBarrier imageMemoryBarrier {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (oldImageLayout)
        {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                imageMemoryBarrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newImageLayout)
        {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                if (imageMemoryBarrier.srcAccessMask == 0)
                {
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);

        render::ShatterRender::getRender().endSingleTimeCommands(cmdbuffer);
    }

    void ShatterTexture::Transition_ImageLayout(VkImage       image,
                                                VkFormat      format,
                                                VkImageLayout oldLayout,
                                                VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (::hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    }

    void ShatterTexture::copyBufferToImage(VkBuffer buffer,
                                           VkImage image,
                                           uint32_t width,
                                           uint32_t height){
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    }

    void ShatterTexture::copyBufferTo3DImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth)
    {
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                depth
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    }

    void ShatterTexture::copyBufferToCubeImage(int face,VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = face;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);
    }

    void ShatterTexture::copyBufferToImageLevels(VkBuffer buffer,
                                                 VkImage image,
                                                 std::vector<VkBufferImageCopy>& copy_vec,
                                                 uint32_t width,
                                                 uint32_t height){
        VkCommandBuffer commandBuffer = render::ShatterRender::getRender().beginSingleTimeCommands();
        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(copy_vec.size()),
                copy_vec.data()
        );
        render::ShatterRender::getRender().endSingleTimeCommands(commandBuffer);

    }

    void ShatterTexture::create1dImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags){
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(*m_device, &viewInfo, nullptr, &m_textureImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void ShatterTexture::create2dImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(*m_device, &viewInfo, nullptr, &m_textureImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }


    void ShatterTexture::create3dImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags){
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(*m_device, &viewInfo, nullptr, &m_textureImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void ShatterTexture::createCubeImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 6;

        if (vkCreateImageView(*m_device, &viewInfo, nullptr, &m_textureImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }


    VkImageView ShatterTexture::Create_ImageView(VkDevice* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags){
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageview;
        if (vkCreateImageView(*device, &viewInfo, nullptr, &imageview) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
        return imageview;
    }

    void ShatterTexture::createTextureSampler(){
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(*m_device, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }


    bool ShatterTexture::initTextureKtx(std::string        filename,
                                        VkFormat           format,
                                        VkImageUsageFlags  imageUsageFlags,
                                        VkImageLayout      imageLayout,
                                        bool               forceLinear){
        ktxTexture* ktxTexture;
        ktxResult result = file::loadKTXFile(filename, &ktxTexture);
        assert(result == KTX_SUCCESS);

        auto& device = SingleDevice;
        uint32_t width = ktxTexture->baseWidth;
        uint32_t height = ktxTexture->baseHeight;
        mipLevels = ktxTexture->numLevels;

        ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

        // Get device properties for the requested texture format
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.physicalDevice, format, &formatProperties);

        // Only use linear tiling if requested (and supported by the device)
        // Support for linear tiling is mostly limited, so prefer to use
        // optimal tiling instead
        // On most implementations linear tiling will only support a very
        // limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
        VkBool32 useStaging = !forceLinear;

        VkMemoryAllocateInfo memAllocInfo = tool::memoryAllocateInfo();
        VkMemoryRequirements memReqs;

        // Use a separate command buffer for texture loading
//        VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        VkCommandBuffer copyCmd;
        VkDeviceMemory deviceMemory;
        VkImageLayout ImageLayout;
        if (useStaging)
        {
            // Create a host-visible staging buffer that contains the raw image data
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;

            VkBufferCreateInfo bufferCreateInfo = tool::bufferCreateInfo();
            bufferCreateInfo.size = ktxTextureSize;
            // This buffer is used as a transfer source for the buffer copy
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VK_CHECK_RESULT(vkCreateBuffer(device.logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

            // Get memory requirements for the staging buffer (alignment, memory type bits)
            vkGetBufferMemoryRequirements(device.logicalDevice, stagingBuffer, &memReqs);

            memAllocInfo.allocationSize = memReqs.size;
            // Get memory type index for a host visible buffer
            memAllocInfo.memoryTypeIndex = device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            VK_CHECK_RESULT(vkAllocateMemory(device.logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
            VK_CHECK_RESULT(vkBindBufferMemory(device.logicalDevice, stagingBuffer, stagingMemory, 0));

            // Copy texture data into staging buffer
            uint8_t *data;
            VK_CHECK_RESULT(vkMapMemory(device.logicalDevice, stagingMemory, 0, memReqs.size, 0, (void **)&data));
            memcpy(data, ktxTextureData, ktxTextureSize);
            vkUnmapMemory(device.logicalDevice, stagingMemory);

            // Setup buffer copy regions for each mip level
            std::vector<VkBufferImageCopy> bufferCopyRegions;

            for (uint32_t i = 0; i < mipLevels; i++)
            {
                ktx_size_t offset;
                KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
                assert(result == KTX_SUCCESS);

                VkBufferImageCopy bufferCopyRegion = {};
                bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                bufferCopyRegion.imageSubresource.mipLevel = i;
                bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
                bufferCopyRegion.imageSubresource.layerCount = 1;
                bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
                bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
                bufferCopyRegion.imageExtent.depth = 1;
                bufferCopyRegion.bufferOffset = offset;

                bufferCopyRegions.push_back(bufferCopyRegion);
            }

            // Create optimal tiled target image
            VkImageCreateInfo imageCreateInfo = tool::imageCreateInfo();
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = format;
            imageCreateInfo.mipLevels = mipLevels;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.extent = { width, height, 1 };
            imageCreateInfo.usage = imageUsageFlags;
            // Ensure that the TRANSFER_DST bit is set for staging
            if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
            {
                imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }
            VK_CHECK_RESULT(vkCreateImage(device.logicalDevice, &imageCreateInfo, nullptr, &m_textureImage));

            vkGetImageMemoryRequirements(device.logicalDevice, m_textureImage, &memReqs);

            memAllocInfo.allocationSize = memReqs.size;

            memAllocInfo.memoryTypeIndex = device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(device.logicalDevice, &memAllocInfo, nullptr, &deviceMemory));
            VK_CHECK_RESULT(vkBindImageMemory(device.logicalDevice, m_textureImage, deviceMemory, 0));

            VkImageSubresourceRange subresourceRange = {};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = mipLevels;
            subresourceRange.layerCount = 1;

            // Image barrier for optimal image (target)
            // Optimal image will be used as destination for the copy
//            tool::setImageLayout(
//                    copyCmd,
//                    m_textureImage,
//                    VK_IMAGE_LAYOUT_UNDEFINED,
//                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                    subresourceRange);

            setImageLayout(
                    m_textureImage,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    mipLevels);

            VkCommandBuffer cmdbuffer = render::ShatterRender::getRender().beginSingleTimeCommands();

            // Copy mip levels from staging buffer
            vkCmdCopyBufferToImage(
                    cmdbuffer,
                    stagingBuffer,
                    m_textureImage,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    static_cast<uint32_t>(bufferCopyRegions.size()),
                    bufferCopyRegions.data()
            );
            render::ShatterRender::getRender().endSingleTimeCommands(cmdbuffer);
            // Change texture image layout to shader read after all mip levels have been copied
            ImageLayout = imageLayout;
//            tool::setImageLayout(
//                    copyCmd,
//                    m_textureImage,
//                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                    imageLayout,
//                    subresourceRange);
            setImageLayout(
                    m_textureImage,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    imageLayout,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    mipLevels);
//            device->flushCommandBuffer(copyCmd, copyQueue);

            // Clean up staging resources
            vkFreeMemory(device.logicalDevice, stagingMemory, nullptr);
            vkDestroyBuffer(device.logicalDevice, stagingBuffer, nullptr);
        }
        else
        {
            // Prefer using optimal tiling, as linear tiling
            // may support only a small set of features
            // depending on implementation (e.g. no mip maps, only one layer, etc.)

            // Check if this support is supported for linear tiling
            assert(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

            VkImage mappableImage;
            VkDeviceMemory mappableMemory;

            VkImageCreateInfo imageCreateInfo = tool::imageCreateInfo();
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = format;
            imageCreateInfo.extent = { width, height, 1 };
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
            imageCreateInfo.usage = imageUsageFlags;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            // Load mip map level 0 to linear tiling image
            VK_CHECK_RESULT(vkCreateImage(device.logicalDevice, &imageCreateInfo, nullptr, &mappableImage));

            // Get memory requirements for this image
            // like size and alignment
            vkGetImageMemoryRequirements(device.logicalDevice, mappableImage, &memReqs);
            // Set memory allocation size to required memory size
            memAllocInfo.allocationSize = memReqs.size;

            // Get memory type that can be mapped to host memory
            memAllocInfo.memoryTypeIndex = device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            // Allocate host memory
            VK_CHECK_RESULT(vkAllocateMemory(device.logicalDevice, &memAllocInfo, nullptr, &mappableMemory));

            // Bind allocated image for use
            VK_CHECK_RESULT(vkBindImageMemory(device.logicalDevice, mappableImage, mappableMemory, 0));

            // Get sub resource layout
            // Mip map count, array layer, etc.
            VkImageSubresource subRes = {};
            subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subRes.mipLevel = 0;

            VkSubresourceLayout subResLayout;
            void *data;

            // Get sub resources layout
            // Includes row pitch, size offsets, etc.
            vkGetImageSubresourceLayout(device.logicalDevice, mappableImage, &subRes, &subResLayout);

            // Map image memory
            VK_CHECK_RESULT(vkMapMemory(device.logicalDevice, mappableMemory, 0, memReqs.size, 0, &data));

            // Copy image data into memory
            memcpy(data, ktxTextureData, memReqs.size);

            vkUnmapMemory(device.logicalDevice, mappableMemory);

            // Linear tiled images don't need to be staged
            // and can be directly used as textures
            m_textureImage = mappableImage;
            deviceMemory = mappableMemory;
            ImageLayout = imageLayout;

            // Setup image memory barrier
//            tool::setImageLayout(copyCmd, m_textureImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);
            setImageLayout(
                    m_textureImage,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    imageLayout,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    mipLevels);
//            device->flushCommandBuffer(copyCmd, copyQueue);
        }

        ktxTexture_Destroy(ktxTexture);

        // Create a default sampler
        VkSamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0f;
        // Max level-of-detail should match mip level count
        samplerCreateInfo.maxLod = (useStaging) ? (float)mipLevels : 0.0f;
        // Only enable anisotropic filtering if enabled on the device
//        samplerCreateInfo.maxAnisotropy = device->enabledFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
//        samplerCreateInfo.anisotropyEnable = device->enabledFeatures.samplerAnisotropy;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(device.logicalDevice, &samplerCreateInfo, nullptr, &m_textureSampler));

        // Create image view
        // Textures are not directly accessed by the shaders and
        // are abstracted by image views containing additional
        // information and sub resource ranges
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = format;
        viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        // Linear tiling usually won't support mip maps
        // Only set mip map count if optimal tiling is used
        viewCreateInfo.subresourceRange.levelCount = (useStaging) ? mipLevels : 1;
        viewCreateInfo.image = m_textureImage;
        VK_CHECK_RESULT(vkCreateImageView(device.logicalDevice, &viewCreateInfo, nullptr, &m_textureImageView));

        // Update descriptor image info member that can be used for setting up descriptor sets
        // updateDescriptor();
        m_memory = deviceMemory;
        return true;
    }




}








