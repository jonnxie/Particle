//
// Created by maybe on 2020/11/16.
//

#ifndef SHATTER_ENGINE_SHATTER_ITEM_H
#define SHATTER_ENGINE_SHATTER_ITEM_H

//#define GLFW_INCLUDE_VULKAN
#include <glm.hpp>
//#include <glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <array>
#include <unordered_map>
#include <gtx/hash.hpp>
#include <memory>
#include <ktx.h>
#include <ktxvulkan.h>
#include "shatter_enum.h"
#include <map>
#include <chrono>

#define PARTICLE_SIZE 10.0f

//#define log(x) std::cout << x << std::endl
#define throwError(x) throw std::runtime_error(x)
#define throwFile(x) throw std::runtime_error(std::string("no such file ") + x)

struct CPUParticleUniform
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec2 viewportDim;
    float     pointSize = PARTICLE_SIZE;
};

struct Light_Uniform
{
    glm::vec3 light_pos;
    glm::vec3 light_tar;
    float     light_radius;
    uint32_t  light_type;
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct CameraBuffer
{
    glm::mat4 view;
    glm::mat4 proj;
};

#define CameraBufferSize 128


struct Light
{
    float     intensity;//强度
    glm::vec3 diffuse_color;//漫反射颜色
    glm::vec3 specular;//高光反射颜色
    uint32_t  type;//光照类型
    glm::vec3 position;//位置
    glm::vec3 direction;//方向
    float     spot;//cosine range
    float     radius;//半径
};

struct SPHInfo
{
    int   particle_num;
    float particle_radius;
    float particle_mass;
    float smoothing_length;
    float diffusivity_coefficient;//扩散力系数
    float air_resistance;//空气阻力系数
    float air_floatation_correction;//空气浮力系数
    float indoor_temperature;//室内温度
};

struct IntegrateInfo
{
    float time_step;
    float wall_damping;
};

struct MarchingCubeInfo
{
    int   grid_num;
    float iso_level;
};

struct MarchingCubeDensity
{
    glm::vec3 pos;
    float val;
};

struct Triangle{
    glm::vec3 pos[3];
    glm::vec3 norm;
};

struct ParticleData{
    float time;
    float pointSize;
};

struct ThreadObject{
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> buffers;
    std::vector<bool> visibility;
    std::vector<VkCommandBuffer> off_buffers;
    std::vector<bool> off_visibility;
};

struct Point3d_Color_UV {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Point3d_Color_UV);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Point3d_Color_UV, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Point3d_Color_UV, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Point3d_Color_UV, uv);

        return attributeDescriptions;
    }

    bool operator==(const Point3d_Color_UV &other) const {
        return pos == other.pos && color == other.color && uv == other.uv;
    }

};

/*
 * point struct contain position and color
 */
struct Point_PC{
    glm::vec3 pos;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Point_PC);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Point_PC, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Point_PC, color);

        return attributeDescriptions;
    }

    bool operator==(const Point_PC &other) const {
        return pos == other.pos && color == other.color;
    }
};

/*
 * used in box
 */
struct PC_UV{
    glm::vec2 pos;
    glm::vec3 uv;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(PC_UV);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(PC_UV, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(PC_UV, uv);

        return attributeDescriptions;
    }

    bool operator==(const PC_UV &other) const {
        return pos == other.pos && uv == other.uv;
    }
};

/*
 * point struct contain screen position and color under screen
 */
struct Point_UNDER_PC{
    glm::vec2 pos;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Point_UNDER_PC);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Point_UNDER_PC, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Point_UNDER_PC, color);

        return attributeDescriptions;
    }

    bool operator==(const Point_UNDER_PC &other) const {
        return pos == other.pos && color == other.color;
    }
};

/*
 * point struct contain screen position and uv coordiante
 */
struct Point_UV{
    glm::vec3 pos;
    glm::vec2 uv;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Point_UV);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Point_UV, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Point_UV, uv);

        return attributeDescriptions;
    }

    bool operator==(const Point_UV &other) const {
        return pos == other.pos && uv == other.uv;
    }
};

/*
 * point struct contain screen position and uv coordiante
 */
struct Point_3dUV{
    glm::vec3 pos;
    glm::vec3 uv;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Point_3dUV);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Point_3dUV, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Point_3dUV, uv);

        return attributeDescriptions;
    }

    bool operator==(const Point_3dUV &other) const {
        return pos == other.pos && uv == other.uv;
    }
};

/*
 * point struct contain  position
 */
struct Point3d{
    glm::vec3 pos;

    bool operator==(const Point3d &other) const {
        return pos == other.pos;
    }
};

struct Point3d_Normal{
    glm::vec3 pos;
    glm::vec3 nor;

    bool operator ==(const Point3d_Normal &other)const{
        return pos == other.pos && nor == other.nor;
    }
};

struct Point2d{
    glm::vec2 pos;

    bool operator==(const Point2d &other) const {
        return pos == other.pos;
    }
};

struct Point2dUV3d{
    glm::vec2 pos;
    glm::vec3 uv;

    bool operator==(const Point2dUV3d &other) const{
        return pos == other.pos && uv == other.uv;
    }
};

struct Point3dUV1d{
    glm::vec3 pos;
    float coordinate;

    bool operator==(const Point3dUV1d &other) const{
        return pos == other.pos && coordinate == other.coordinate;
    }
};

struct Point3dColorSize {
    glm::vec3 pos;
    glm::vec3 color;
    float     size{4};

    bool operator==(const Point3dColorSize& other) const{
        return pos == other.pos && color == other.color && size == other.size;
    }
};

#define Point3dColorSizeSize 28

/*
 * point struct contain position,Color_With_Alpha,size,rotation,type
 */

struct Point_CP{
    glm::vec3 pos;
    glm::vec4 color;
    glm::float32 size;
    glm::float32 rotation;
    glm::uint type;
    glm::vec4 vel;
    float rotationSpeed;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Point_CP);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Point_CP, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Point_CP, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 1;
        attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Point_CP, size);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 1;
        attributeDescriptions[3].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Point_CP, rotation);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 1;
        attributeDescriptions[4].format = VK_FORMAT_R32_UINT;
        attributeDescriptions[4].offset = offsetof(Point_CP, type);

        return attributeDescriptions;
    }

    bool operator==(const Point_CP &other) const {
        return pos == other.pos &&
        color == other.color &&
        size == other.size &&
        rotation == other.rotation &&
        type == other.type;
    }
};

struct GltfPoint{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator == (const GltfPoint &other) const {
        return pos == other.pos && normal == other.normal && uv == other.uv;
    }
};

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;
    int computeFamily = -1;
    int transferFamily = -1;

    bool isComplete() {
        return graphicsFamily >= 0 && presentFamily >= 0 && computeFamily >= 0 && transferFamily >= 0;
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct RayTracingUniformData{
    glm::mat4 viewInverse;
    glm::mat4 projInverse;
    glm::vec4 lightPos;
    int32_t   vertexSize;
};

struct InstanceData{
    glm::vec3 pos;
    float scale;
};

struct ObjectBox{
    glm::vec3 pos;
    float radius;
};

#define MAX_LOD_LEVEL 5

static struct IndirectDrawState{
    uint32_t drawCount;						// Total number of indirect draw counts to be issued
    uint32_t lodCount[MAX_LOD_LEVEL + 1];	// Statistics for number of draws per LOD level (written by compute shader)
} indirect_draw_state;

IndirectDrawState getIndirectState();

void* getIndirectDrawState();

struct IndexedIndirectCommand
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t vertexOffset;
    uint32_t firstInstance;
};

struct ComputeCullUniform
{
    glm::mat4 projection;
    glm::mat4 modelview;
    glm::vec4 cameraPos;
    glm::vec4 frustumPlanes[6];
};
#define ComputeCullUniformSize 240

struct Point{
    glm::vec3 pos;
    glm::vec3 color;
};
#define PointSize 24

struct Line{
    Point begin;
    Point end;
};
#define LineSize 48

struct Tri{
    Point points[3];
};

#define TriSize 72

struct TagetPlane
{
    glm::vec3 x_coordinate;
    glm::vec3 y_coordinate;
    glm::vec3 z_coordinate;
};
#define TargetPlaneSize 36

#define TargetPlaneDoubleCoordinateSize 144

static struct Material{
    float roughness;
    float metallic;
    float r;
    float g;
    float b;
} material;

Material& getMaterial();

struct VolumeInfo{
    float       StepSize;
    float 	    Dumy;
    glm::vec2   ScreenSize;
};

namespace std {
    template<>
    struct hash<Point3d_Color_UV> {
        size_t operator()(Point3d_Color_UV const &vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };

    template<>
    struct hash<Point_PC> {
        size_t operator()(Point_PC const &point) const{
            return hash<glm::vec3>()(point.pos) ^ (hash<glm::vec3>()(point.color) << 1);
        }
    };

    template<>
    struct hash<Point_UNDER_PC> {
        size_t operator()(Point_UNDER_PC const &point) const{
            return hash<glm::vec2>()(point.pos) ^ (hash<glm::vec3>()(point.color) << 1);
        }
    };

    template<>
    struct hash<Point_CP> {
        size_t operator()(Point_CP const &point) const{
            return ((((((hash<glm::vec3>()(point.pos) ^ (hash<glm::vec4>()(point.color) << 1)) >> 1) ^
                   (hash<glm::float32>()(point.size) << 1)) >> 1) ^ (hash<glm::float32>()(point.rotation) << 1)) >> 1) ^
                    (hash<glm::uint>()(point.type) << 1);
        }
    };
}

namespace file{
    /*
    * 检测文件是否存在
    */
    bool fileExists(const std::string &filename);

    /*
    *读glsl文件随后创建shader的模块
    */
    std::vector<char> readFile(const std::string &filename);

    /*
     * load .ktx format texture
     */
    ktxResult loadKTXFile(std::string filename, ktxTexture **target);

    typedef enum TexTypeENUM
    {
        COMMON = 0,
        RGBA8_BC3_DXT5 = 1,
    } TexType;

    class TexDataObject
    {
    public:
        int width;
        int height;
        int dataByteCount;
        unsigned char* data;
        TexType type;

        TexDataObject(int width,int height,unsigned char* data,int dataByteCount,TexType type = COMMON);
        ~TexDataObject();
    };

    //根据dxt5纹理的宽度和高度计算纹理数据字节数的函数

    int textureLevelSizeS3tcDxt5(int width, int height);

    int mymax(int a, int b);

    TexDataObject* load_DXT5_BC3_TexData(const std::string& _filename);

    std::string loadAssetStr(const std::string& fname);

    TexDataObject* loadCommonTexData(std::string fname);

    unsigned char* loadBinary(std::string fname);

    int myReadInt(unsigned char* binaryData, int* binaryData_index);

    short myReadUnsignedShort(unsigned char* binaryData, int* binaryData_index);

    int myReadByte(unsigned char* binaryData, int* binaryData_index);

    float myReadFloat(unsigned char* binaryData, int* binaryData_index);

    std::string myReadString(unsigned char* binaryData, int* binaryData_index,int length);
}

/*
 *创建shader的着色器模块
 */
VkShaderModule createShaderModule(const std::vector<char> &code,VkDevice device);

/*
 * 限定extent的范围
 */
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,unsigned int width,unsigned int heigh);

/*
 * 选定呈现模式
 */
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

/*
 * 选定表面格式
 */
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

/*
 * 检验深度模板数据格式
 */
bool hasStencilComponent(VkFormat format);

static uint32_t mat4_size = sizeof(glm::mat4);

uint32_t mat4Size();

static VkPhysicalDeviceFeatures deviceFeatures;

static std::map<std::string,bool> deviceFeatureMap;

void setDeviceFeatures(const VkPhysicalDeviceFeatures&);

VkPhysicalDeviceFeatures& getDeviceFeatures();

bool checkFeatures(const std::string&);

void printDeviceFeatures();

static VkPhysicalDeviceMemoryProperties m_VkPhysicalDeviceMemoryProperties;

#define generatevk(type) \
void set##type(const type& _type){ \
    m_##type = _type;                        \
}                           \
type get##type()       \
{                                       \
return m_##type;              \
}

#define definevk(type)          \
void set##type(const type& _type);\
type get##type();

definevk(VkPhysicalDeviceMemoryProperties);

bool memoryTypeFromProperties(VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

class ResultMap{
public:
    static ResultMap& getResultMap();
    std::map<int,std::string>& operator()(){
        return m_map;
    }
    ResultMap();

    std::map<int,std::string> m_map{};
};

/**
 *
 * @return  name of VkResult
 */
void getResult(int);

std::string getVkResult(int);

int patchNum(int _objectNum,int _thread);

static QueueFamilyIndices queueIndices;

QueueFamilyIndices getIndices();

void setIndices(const QueueFamilyIndices& _indices);

static VkViewport viewPort;

VkViewport getViewPort();

void setViewPort(VkViewport _viewport);

static VkRect2D scissor;

VkRect2D getScissor();

void setScissor(VkRect2D _scissor);

static std::vector<ThreadObject> threadObjects;

std::vector<ThreadObject>* getThreadObjectPool();

void releaseThreadObjectPool();

struct UnionCommandPool{
    VkCommandPool graphicsPool;
    VkCommandPool computePool;
};

enum class CommandPoolType{
    GraphicsPool,
    ComputePool
};

VkCommandPool getCommandPool(CommandPoolType _type);

static std::vector<UnionCommandPool> threadCommandPool;

std::vector<UnionCommandPool>* getThreadCommandPool();

void releaseThreadCommandPool();

typedef  std::chrono::time_point<std::chrono::system_clock> time_point;

//static std::chrono::time_point<std::chrono::steady_clock,std::chrono::nanoseconds > shatter_time;
namespace timer{

    static float shatter_time;

    float getTime();

    void setTime(const float& _time);

    static float deta_time;

    float getDetaTime();

    void setDetaTime(const float& _time);

    static time_point chrono_time;

    void setChronoTime(const time_point& _time);

    time_point* getChronoTime();
}

static void* deviceCreatepNextChain = nullptr;

void* getDeviceCreateNextChain();

void setDeviceCreateNextChain(void* pNext);

static VkSurfaceFormatKHR swapchain_format;

void setSwapChainFormat(const VkSurfaceFormatKHR& _format);

VkSurfaceFormatKHR getSwapChainFormat();

static VkFormat depth_format;

void setDepthFormat(const VkFormat& _format);

VkFormat getDepthFormat();

static int currentSwapChainIndex;

void setSwapChainIndex(int _index);

int getSwapChainIndex();

namespace tool{
    std::string combine(const std::string& _str,int num);

    std::string combineShader(const std::string& _shaderName);

    std::string combineTexture(const std::string& _shaderName);

    std::string combineModel(const std::string& _shaderName);

    template<class T>
    void printNum(T _num)
    {
        printf("%f\n",_num);
    }

    void printMat(const glm::mat4& _in);

    VkPipelineShaderStageCreateInfo createShaderStage(const std::string& _filename,ShaderType _type);

    VkDescriptorSetLayoutBinding getSetLayoutBinding(VkDescriptorType _type,VkShaderStageFlags _flags,uint32_t _binding);

    VkBufferCreateInfo getBufferCreateInfo();

    void setLayout(VkCommandBuffer _buffer, VkImage _image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange sub);

    VkMemoryAllocateInfo memoryAllocateInfo();

    VkImageCreateInfo imageCreateInfo();

    VkSamplerCreateInfo samplerCreateInfo();

    VkImageViewCreateInfo imageViewCreateInfo();

    VkRenderPassCreateInfo renderPassCreateInfo();

    VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType _type,uint32_t _count);

    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType _type,VkShaderStageFlags _flags,uint32_t _binding,uint32_t _descriptorCount = 1);

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& poolSizes,uint32_t _maxSetCount);

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& _setLayoutBinding);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(VkDescriptorPool _pool,VkDescriptorSetLayout* _layout,uint32_t _count);

    VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);

    VkBufferCreateInfo bufferCreateInfo();

    VkBufferCreateInfo bufferCreateInfo(
            VkBufferUsageFlags usage,
            VkDeviceSize size);

    VkViewport viewport(
            float width,
            float height,
            float minDepth,
            float maxDepth);

    VkRect2D rect2D(
            int32_t width,
            int32_t height,
            int32_t offsetX,
            int32_t offsetY);

    VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            uint32_t binding,
            VkDescriptorBufferInfo* bufferInfo,
            uint32_t descriptorCount = 1);

    VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            uint32_t binding,
            VkDescriptorImageInfo *imageInfo,
            uint32_t descriptorCount = 1);

    VkPushConstantRange pushConstantRange(
            VkShaderStageFlags stageFlags,
            uint32_t size,
            uint32_t offset);

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
            VkPrimitiveTopology topology,
            VkPipelineInputAssemblyStateCreateFlags flags,
            VkBool32 primitiveRestartEnable);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t setLayoutCount = 1);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            uint32_t setLayoutCount = 1);

    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
            VkPolygonMode polygonMode,
            VkCullModeFlags cullMode,
            VkFrontFace frontFace,
            VkPipelineRasterizationStateCreateFlags flags = 0);

    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
            VkColorComponentFlags colorWriteMask,
            VkBool32 blendEnable);

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
            uint32_t attachmentCount,
            const VkPipelineColorBlendAttachmentState * pAttachments);

    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
            VkBool32 depthTestEnable,
            VkBool32 depthWriteEnable,
            VkCompareOp depthCompareOp);

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
            uint32_t viewportCount,
            uint32_t scissorCount,
            VkPipelineViewportStateCreateFlags flags = 0);

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
            VkSampleCountFlagBits rasterizationSamples,
            VkPipelineMultisampleStateCreateFlags flags = 0);

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
            const VkDynamicState * pDynamicStates,
            uint32_t dynamicStateCount,
            VkPipelineDynamicStateCreateFlags flags = 0);

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
            const std::vector<VkDynamicState>& pDynamicStates,
            VkPipelineDynamicStateCreateFlags flags = 0);

    VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo(
            VkPipelineLayout layout,
            VkRenderPass renderPass,
            VkPipelineCreateFlags flags = 0);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo();

    VkComputePipelineCreateInfo computePipelineCreateInfo(
            VkPipelineLayout layout,
            VkPipelineCreateFlags flags = 0);

    VkVertexInputBindingDescription vertexInputBindingDescription(
            uint32_t binding,
            uint32_t stride,
            VkVertexInputRate inputRate);

    VkVertexInputAttributeDescription vertexInputAttributeDescription(
            uint32_t binding,
            uint32_t location,
            VkFormat format,
            uint32_t offset);

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
            const std::vector<VkVertexInputBindingDescription> &vertexBindingDescriptions,
            const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions
    );

    VkImageMemoryBarrier imageMemoryBarrier();

    void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    // Uses a fixed sub resource layout with first mip level and layer
    void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkImageAspectFlags aspectMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    uint32_t alignedSize(uint32_t value, uint32_t alignment);

    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfoKHR();

    VkAccelerationStructureGeometryKHR accelerationStructureGeometryKHR();

    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR();

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoKHR();

    VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfoKHR();

    VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructureKHR();

    VkCommandBufferBeginInfo commandBufferBeginInfo();

    void insertImageMemoryBarrier(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkAccessFlags srcAccessMask,
            VkAccessFlags dstAccessMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            VkImageSubresourceRange subresourceRange);

    VkFramebufferCreateInfo framebufferCreateInfo();

    VkRenderPassBeginInfo renderPassBeginInfo();

    void saveScreenshot(std::basic_string<char> filename);

    inline std::string strBool(bool _val){
        return _val? "True":"False";
    }
}

class Frustum
{
public:
    enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };
    std::array<glm::vec4, 6> planes;

    void update(const glm::mat4& matrix)
    {
        planes[LEFT].x = matrix[0].w + matrix[0].x;
        planes[LEFT].y = matrix[1].w + matrix[1].x;
        planes[LEFT].z = matrix[2].w + matrix[2].x;
        planes[LEFT].w = matrix[3].w + matrix[3].x;

        planes[RIGHT].x = matrix[0].w - matrix[0].x;
        planes[RIGHT].y = matrix[1].w - matrix[1].x;
        planes[RIGHT].z = matrix[2].w - matrix[2].x;
        planes[RIGHT].w = matrix[3].w - matrix[3].x;

        planes[TOP].x = matrix[0].w - matrix[0].y;
        planes[TOP].y = matrix[1].w - matrix[1].y;
        planes[TOP].z = matrix[2].w - matrix[2].y;
        planes[TOP].w = matrix[3].w - matrix[3].y;

        planes[BOTTOM].x = matrix[0].w + matrix[0].y;
        planes[BOTTOM].y = matrix[1].w + matrix[1].y;
        planes[BOTTOM].z = matrix[2].w + matrix[2].y;
        planes[BOTTOM].w = matrix[3].w + matrix[3].y;

        planes[BACK].x = matrix[0].w + matrix[0].z;
        planes[BACK].y = matrix[1].w + matrix[1].z;
        planes[BACK].z = matrix[2].w + matrix[2].z;
        planes[BACK].w = matrix[3].w + matrix[3].z;

        planes[FRONT].x = matrix[0].w - matrix[0].z;
        planes[FRONT].y = matrix[1].w - matrix[1].z;
        planes[FRONT].z = matrix[2].w - matrix[2].z;
        planes[FRONT].w = matrix[3].w - matrix[3].z;

        for (auto i = 0; i < planes.size(); i++)
        {
            float length = sqrtf(planes[i].x * planes[i].x + planes[i].y * planes[i].y + planes[i].z * planes[i].z);
            planes[i] /= length;
        }
    }

    bool checkSphere(glm::vec3 pos, float radius)
    {
        for (auto i = 0; i < planes.size(); i++)
        {
            if ((planes[i].x * pos.x) + (planes[i].y * pos.y) + (planes[i].z * pos.z) + planes[i].w <= -radius)
            {
                return false;
            }
        }
        return true;
    }
};

struct FrameBufferAttachment{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format;
};

void transitionImageLayout(
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

void transitionImageLayout(
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

void testSIMD();


#endif //SHATTER_ENGINE_SHATTER_ITEM_H
