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

static std::map<int,std::string> resultMap = {
{0,std::string("VK_SUCCESS")},
{1,std::string("VK_NOT_READY")},
{2,std::string("VK_TIMEOUT")},
{3,std::string("VK_EVENT_SET")},
{4,std::string("VK_EVENT_RESET")},
{5,std::string("VK_INCOMPLETE")},
{-1,std::string("VK_ERROR_OUT_OF_HOST_MEMORY")},
{-2,std::string("VK_ERROR_OUT_OF_DEVICE_MEMORY")},
{-3,std::string("VK_ERROR_INITIALIZATION_FAILED")},
{-4,std::string("VK_ERROR_DEVICE_LOST")},
{-5,std::string("VK_ERROR_MEMORY_MAP_FAILED")},
{-6,std::string("VK_ERROR_LAYER_NOT_PRESENT")},
{-7,std::string("VK_ERROR_EXTENSION_NOT_PRESENT")},
{-8,std::string("VK_ERROR_FEATURE_NOT_PRESENT")},
{-9,std::string("VK_ERROR_INCOMPATIBLE_DRIVER")},
{-10,std::string("VK_ERROR_TOO_MANY_OBJECTS")},
{-11,std::string("VK_ERROR_FORMAT_NOT_SUPPORTED")},
{-12,std::string("VK_ERROR_FRAGMENTED_POOL")},
{-13,std::string("VK_ERROR_UNKNOWN")},
{-1000069000,std::string("VK_ERROR_OUT_OF_POOL_MEMORY")},
{-1000072003,std::string("VK_ERROR_INVALID_EXTERNAL_HANDLE")},
{-1000161000,std::string("VK_ERROR_FRAGMENTATION")},
{-1000257000,std::string("VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS")},
{-1000000000,std::string("VK_ERROR_SURFACE_LOST_KHR")},
{-1000000001,std::string("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR")},
{-1000001003,std::string("VK_SUBOPTIMAL_KHR")},
{-1000001004,std::string("VK_ERROR_OUT_OF_DATE_KHR")},
{-1000003001,std::string("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR")},
{-1000011001,std::string("VK_ERROR_VALIDATION_FAILED_EXT")},
{-1000012000,std::string("VK_ERROR_INVALID_SHADER_NV")},
{-1000158000,std::string("VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT")},
{-1000174001,std::string("VK_ERROR_NOT_PERMITTED_EXT")},
{-1000255000,std::string("VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT")},
{1000268000,std::string("VK_THREAD_IDLE_KHR")},
{1000268001,std::string("VK_THREAD_DONE_KHR")},
{1000268002,std::string("VK_OPERATION_DEFERRED_KHR")},
{1000268003,std::string("VK_OPERATION_NOT_DEFERRED_KHR")},
{1000297000,std::string("VK_PIPELINE_COMPILE_REQUIRED_EXT")},
{0x7FFFFFFF,std::string("VK_RESULT_MAX_ENUM")}
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


class Vector3f {
public:
    float* coord;
public:
    Vector3f();
    Vector3f(float x, float y, float z);
    ~Vector3f();
    void interpolate(Vector3f* v1, Vector3f* v2, float alpha);
    float* getVector3fArray();
    void setX(float x);
    void setY(float y);
    void setZ(float z);
    float getX();
    float getY();
    float getZ();
};

class Vector4f {
private:
public:
    float* coording;
    Vector4f();
    Vector4f(float x1, float x2, float x3, float x4);
    ~Vector4f();
    void setFromEulerAngleToQuaternion(float yaw, float pitch, float roll);
    void interpolate(Vector4f* v1, Vector4f* v2, float alpha);
    void setX(float x);
    void setY(float y);
    void setZ(float z);
    void setW(float w);
    float getX();
    float getY();
    float getZ();
    float getW();
};

class Matrix {
public:
    static void multiplyMM(float* result, int resultOffset, float* mlIn, int lhsOffset, float* mrIn, int rhsOffset)
    {
        double ml[16];
        double mr[16];
        for(int i=0;i<16;i++)
        {
            ml[i]=mlIn[i];
            mr[i]=mrIn[i];
        }
        result[0+resultOffset]=(float) (ml[0+lhsOffset]*mr[0+rhsOffset]+ml[4+lhsOffset]*mr[1+rhsOffset]+ml[8+lhsOffset]*mr[2+rhsOffset]+ml[12+lhsOffset]*mr[3+rhsOffset]);
        result[1+resultOffset]=(float) (ml[1+lhsOffset]*mr[0+rhsOffset]+ml[5+lhsOffset]*mr[1+rhsOffset]+ml[9+lhsOffset]*mr[2+rhsOffset]+ml[13+lhsOffset]*mr[3+rhsOffset]);
        result[2+resultOffset]=(float) (ml[2+lhsOffset]*mr[0+rhsOffset]+ml[6+lhsOffset]*mr[1+rhsOffset]+ml[10+lhsOffset]*mr[2+rhsOffset]+ml[14+lhsOffset]*mr[3+rhsOffset]);
        result[3+resultOffset]=(float) (ml[3+lhsOffset]*mr[0+rhsOffset]+ml[7+lhsOffset]*mr[1+rhsOffset]+ml[11+lhsOffset]*mr[2+rhsOffset]+ml[15+lhsOffset]*mr[3+rhsOffset]);
        result[4+resultOffset]=(float) (ml[0+lhsOffset]*mr[4+rhsOffset]+ml[4+lhsOffset]*mr[5+rhsOffset]+ml[8+lhsOffset]*mr[6+rhsOffset]+ml[12+lhsOffset]*mr[7+rhsOffset]);
        result[5+resultOffset]=(float) (ml[1+lhsOffset]*mr[4+rhsOffset]+ml[5+lhsOffset]*mr[5+rhsOffset]+ml[9+lhsOffset]*mr[6+rhsOffset]+ml[13+lhsOffset]*mr[7+rhsOffset]);
        result[6+resultOffset]=(float) (ml[2+lhsOffset]*mr[4+rhsOffset]+ml[6+lhsOffset]*mr[5+rhsOffset]+ml[10+lhsOffset]*mr[6+rhsOffset]+ml[14+lhsOffset]*mr[7+rhsOffset]);
        result[7+resultOffset]=(float) (ml[3+lhsOffset]*mr[4+rhsOffset]+ml[7+lhsOffset]*mr[5+rhsOffset]+ml[11+lhsOffset]*mr[6+rhsOffset]+ml[15+lhsOffset]*mr[7+rhsOffset]);
        result[8+resultOffset]=(float) (ml[0+lhsOffset]*mr[8+rhsOffset]+ml[4+lhsOffset]*mr[9+rhsOffset]+ml[8+lhsOffset]*mr[10+rhsOffset]+ml[12+lhsOffset]*mr[11+rhsOffset]);
        result[9+resultOffset]=(float) (ml[1+lhsOffset]*mr[8+rhsOffset]+ml[5+lhsOffset]*mr[9+rhsOffset]+ml[9+lhsOffset]*mr[10+rhsOffset]+ml[13+lhsOffset]*mr[11+rhsOffset]);
        result[10+resultOffset]=(float) (ml[2+lhsOffset]*mr[8+rhsOffset]+ml[6+lhsOffset]*mr[9+rhsOffset]+ml[10+lhsOffset]*mr[10+rhsOffset]+ml[14+lhsOffset]*mr[11+rhsOffset]);
        result[11+resultOffset]=(float) (ml[3+lhsOffset]*mr[8+rhsOffset]+ml[7+lhsOffset]*mr[9+rhsOffset]+ml[11+lhsOffset]*mr[10+rhsOffset]+ml[15+lhsOffset]*mr[11+rhsOffset]);
        result[12+resultOffset]=(float) (ml[0+lhsOffset]*mr[12+rhsOffset]+ml[4+lhsOffset]*mr[13+rhsOffset]+ml[8+lhsOffset]*mr[14+rhsOffset]+ml[12+lhsOffset]*mr[15+rhsOffset]);
        result[13+resultOffset]=(float) (ml[1+lhsOffset]*mr[12+rhsOffset]+ml[5+lhsOffset]*mr[13+rhsOffset]+ml[9+lhsOffset]*mr[14+rhsOffset]+ml[13+lhsOffset]*mr[15+rhsOffset]);
        result[14+resultOffset]=(float) (ml[2+lhsOffset]*mr[12+rhsOffset]+ml[6+lhsOffset]*mr[13+rhsOffset]+ml[10+lhsOffset]*mr[14+rhsOffset]+ml[14+lhsOffset]*mr[15+rhsOffset]);
        result[15+resultOffset]=(float) (ml[3+lhsOffset]*mr[12+rhsOffset]+ml[7+lhsOffset]*mr[13+rhsOffset]+ml[11+lhsOffset]*mr[14+rhsOffset]+ml[15+lhsOffset]*mr[15+rhsOffset]);
    }
    static void multiplyMV (float* resultVec, int resultVecOffset, float* mlIn, int lhsMatOffset,
                            float* vrIn, int rhsVecOffset)
    {
        double ml[16];
        double vr[4];
        for(int i=0;i<16;i++)
        {
            ml[i]=mlIn[i];
        }
        vr[0]=vrIn[0];
        vr[1]=vrIn[1];
        vr[2]=vrIn[2];
        vr[3]=vrIn[3];
        resultVec[0+resultVecOffset]=(float) (ml[0+lhsMatOffset]*vr[0+rhsVecOffset]+ml[4+lhsMatOffset]*vr[1+rhsVecOffset]+ml[8+lhsMatOffset]*vr[2+rhsVecOffset]+ml[12+lhsMatOffset]*vr[3+rhsVecOffset]);
        resultVec[1+resultVecOffset]=(float) (ml[1+lhsMatOffset]*vr[0+rhsVecOffset]+ml[5+lhsMatOffset]*vr[1+rhsVecOffset]+ml[9+lhsMatOffset]*vr[2+rhsVecOffset]+ml[13+lhsMatOffset]*vr[3+rhsVecOffset]);
        resultVec[2+resultVecOffset]=(float) (ml[2+lhsMatOffset]*vr[0+rhsVecOffset]+ml[6+lhsMatOffset]*vr[1+rhsVecOffset]+ml[10+lhsMatOffset]*vr[2+rhsVecOffset]+ml[14+lhsMatOffset]*vr[3+rhsVecOffset]);
        resultVec[3+resultVecOffset]=(float) (ml[3+lhsMatOffset]*vr[0+rhsVecOffset]+ml[7+lhsMatOffset]*vr[1+rhsVecOffset]+ml[11+lhsMatOffset]*vr[2+rhsVecOffset]+ml[15+lhsMatOffset]*vr[3+rhsVecOffset]);
    }
    static void setIdentityM (float* sm, int smOffset)
    {
        for(int i=0;i<16;i++)
        {
            sm[i]=0;
        }
        sm[0]=1;
        sm[5]=1;
        sm[10]=1;
        sm[15]=1;
    }
    static void translateM(float* m, int mOffset,float x, float y, float z)
    {
        for (int i=0 ; i<4 ; i++)
        {
            int mi = mOffset + i;
            m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
        }
    }
    static void rotateM(float* m, int mOffset,float a, float x, float y, float z)
    {
        float rm[16];
        setRotateM(rm, 0, a, x, y, z);
        float rem[16];
        multiplyMM(rem, 0, m, 0, rm, 0);
        for(int i=0;i<16;i++)
        {
            m[i]=rem[i];
        }
    }
    static void setRotateM(float* m, int mOffset,float a, float x, float y, float z)
    {
        float radians = a * 3.14159f / 180.0f;
        float s = sin(radians);
        float c = cos(radians);
        float sm[16];
        setIdentityM(sm, 0);
        sm[0] = x * x * (1 - c) + c;
        sm[1] = x * y * (1 - c) + z * s;
        sm[2] = x * z * (1 - c) - y * s;
        sm[4] = x * y * (1 - c) - z * s;
        sm[5] = y * y * (1 - c) + c;
        sm[6] = y * z * (1 - c) + x * s;
        sm[8] = x * z * (1 - c) + y * s;
        sm[9] = y * z * (1 - c) - x * s;
        sm[10] = z * z * (1 - c) + c;
        for (int i = 0; i<16; i++)
        {
            m[i] = sm[i];
        }
    }
    static void scaleM(float* m, int mOffset, float x, float y, float z)
    {
        float sm[16];
        setIdentityM(sm, 0);
        sm[0] = x;
        sm[5] = y;
        sm[10] = z;
        sm[15] = 1;
        float tm[16];
        multiplyMM(tm,0,m,0,sm,0);
        for(int i=0;i<16;i++)
        {
            m[i]=tm[i];
        }
    }
    static void transposeM(float* mTrans, int mTransOffset, float* m, int mOffset)
    {
        for (int i = 0; i < 4; i++)
        {
            int mBase = i * 4 + mOffset;
            mTrans[i + mTransOffset] = m[mBase];
            mTrans[i + 4 + mTransOffset] = m[mBase + 1];
            mTrans[i + 8 + mTransOffset] = m[mBase + 2];
            mTrans[i + 12 + mTransOffset] = m[mBase + 3];
        }
    }
    static void frustumM(float* m, int offset, float left, float right, float bottom, float top, float near1, float far1)
    {
        const float r_width  = 1.0f / (right - left);
        const float r_height = 1.0f / (top - bottom);
        const float r_depth  = 1.0f / (near1 - far1);
        const float x = 2.0f * (near1 * r_width);
        const float y = 2.0f * (near1 * r_height);
        const float A = 2.0f * ((right + left) * r_width);
        const float B = (top + bottom) * r_height;
        const float C = (far1 + near1) * r_depth;
        const float D = 2.0f * (far1 * near1 * r_depth);
        m[offset + 0] = x;
        m[offset + 5] = y;
        m[offset + 8] = A;
        m[offset +  9] = B;
        m[offset + 10] = C;
        m[offset + 14] = D;
        m[offset + 11] = -1.0f;
        m[offset +  1] = 0.0f;
        m[offset +  2] = 0.0f;
        m[offset +  3] = 0.0f;
        m[offset +  4] = 0.0f;
        m[offset +  6] = 0.0f;
        m[offset +  7] = 0.0f;
        m[offset + 12] = 0.0f;
        m[offset + 13] = 0.0f;
        m[offset + 15] = 0.0f;
    }
    static void orthoM(float * m, int mOffset, float left, float right, float bottom, float top, float near1, float far1)
    {
        assert(left != right);
        assert(bottom != top);
        assert(near1 != far1);
        float r_width  = 1.0f / (right - left);
        float r_height = 1.0f / (top - bottom);
        float r_depth  = 1.0f / (far1 - near1);
        float x =  2.0f * (r_width);
        float y =  2.0f * (r_height);
        float z = -2.0f * (r_depth);
        float tx = -(right + left) * r_width;
        float ty = -(top + bottom) * r_height;
        float tz = -(far1 + near1) * r_depth;
        m[mOffset + 0] = x;
        m[mOffset + 5] = y;
        m[mOffset +10] = z;
        m[mOffset +12] = tx;
        m[mOffset +13] = ty;
        m[mOffset +14] = tz;
        m[mOffset +15] = 1.0f;
        m[mOffset + 1] = 0.0f;
        m[mOffset + 2] = 0.0f;
        m[mOffset + 3] = 0.0f;
        m[mOffset + 4] = 0.0f;
        m[mOffset + 6] = 0.0f;
        m[mOffset + 7] = 0.0f;
        m[mOffset + 8] = 0.0f;
        m[mOffset + 9] = 0.0f;
        m[mOffset + 11] = 0.0f;
    }
    static void setLookAtM(float* rm, int rmOffset, float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ)
    {
        float fx = centerX - eyeX;
        float fy = centerY - eyeY;
        float fz = centerZ - eyeZ;
        float rlf = 1.0f /sqrt(fx*fx + fy*fy +fz*fz);
        fx *= rlf;
        fy *= rlf;
        fz *= rlf;
        float sx = fy * upZ - fz * upY;
        float sy = fz * upX - fx * upZ;
        float sz = fx * upY - fy * upX;
        float rls = 1.0f /sqrt(sx*sx + sy*sy +sz*sz);
        sx *= rls;
        sy *= rls;
        sz *= rls;
        float ux = sy * fz - sz * fy;
        float uy = sz * fx - sx * fz;
        float uz = sx * fy - sy * fx;
        rm[rmOffset + 0] = sx;
        rm[rmOffset + 1] = ux;
        rm[rmOffset + 2] = -fx;
        rm[rmOffset + 3] = 0.0f;
        rm[rmOffset + 4] = sy;
        rm[rmOffset + 5] = uy;
        rm[rmOffset + 6] = -fy;
        rm[rmOffset + 7] = 0.0f;
        rm[rmOffset + 8] = sz;
        rm[rmOffset + 9] = uz;
        rm[rmOffset + 10] = -fz;
        rm[rmOffset + 11] = 0.0f;
        rm[rmOffset + 12] = 0.0f;
        rm[rmOffset + 13] = 0.0f;
        rm[rmOffset + 14] = 0.0f;
        rm[rmOffset + 15] = 1.0f;
        translateM(rm, rmOffset, -eyeX, -eyeY, -eyeZ);
    }
    static void invertM(float* mInv, int mInvOffset, float* m,int mOffset)
    {
        float src0  = m[mOffset +  0];
        float src4  = m[mOffset +  1];
        float src8  = m[mOffset +  2];
        float src12 = m[mOffset +  3];
        float src1  = m[mOffset +  4];
        float src5  = m[mOffset +  5];
        float src9  = m[mOffset +  6];
        float src13 = m[mOffset +  7];
        float src2  = m[mOffset +  8];
        float src6  = m[mOffset +  9];
        float src10 = m[mOffset + 10];
        float src14 = m[mOffset + 11];
        float src3  = m[mOffset + 12];
        float src7  = m[mOffset + 13];
        float src11 = m[mOffset + 14];
        float src15 = m[mOffset + 15];
        float atmp0  = src10 * src15;
        float atmp1  = src11 * src14;
        float atmp2  = src9  * src15;
        float atmp3  = src11 * src13;
        float atmp4  = src9  * src14;
        float atmp5  = src10 * src13;
        float atmp6  = src8  * src15;
        float atmp7  = src11 * src12;
        float atmp8  = src8  * src14;
        float atmp9  = src10 * src12;
        float atmp10 = src8  * src13;
        float atmp11 = src9  * src12;
        float dst0  = (atmp0 * src5 + atmp3 * src6 + atmp4  * src7)
                      - (atmp1 * src5 + atmp2 * src6 + atmp5  * src7);
        float dst1  = (atmp1 * src4 + atmp6 * src6 + atmp9  * src7)
                      - (atmp0 * src4 + atmp7 * src6 + atmp8  * src7);
        float dst2  = (atmp2 * src4 + atmp7 * src5 + atmp10 * src7)
                      - (atmp3 * src4 + atmp6 * src5 + atmp11 * src7);
        float dst3  = (atmp5 * src4 + atmp8 * src5 + atmp11 * src6)
                      - (atmp4 * src4 + atmp9 * src5 + atmp10 * src6);
        float dst4  = (atmp1 * src1 + atmp2 * src2 + atmp5  * src3)
                      - (atmp0 * src1 + atmp3 * src2 + atmp4  * src3);
        float dst5  = (atmp0 * src0 + atmp7 * src2 + atmp8  * src3)
                      - (atmp1 * src0 + atmp6 * src2 + atmp9  * src3);
        float dst6  = (atmp3 * src0 + atmp6 * src1 + atmp11 * src3)
                      - (atmp2 * src0 + atmp7 * src1 + atmp10 * src3);
        float dst7  = (atmp4 * src0 + atmp9 * src1 + atmp10 * src2)
                      - (atmp5 * src0 + atmp8 * src1 + atmp11 * src2);
        float btmp0  = src2 * src7;
        float btmp1  = src3 * src6;
        float btmp2  = src1 * src7;
        float btmp3  = src3 * src5;
        float btmp4  = src1 * src6;
        float btmp5  = src2 * src5;
        float btmp6  = src0 * src7;
        float btmp7  = src3 * src4;
        float btmp8  = src0 * src6;
        float btmp9  = src2 * src4;
        float btmp10 = src0 * src5;
        float btmp11 = src1 * src4;
        float dst8  = (btmp0  * src13 + btmp3  * src14 + btmp4  * src15)
                      - (btmp1  * src13 + btmp2  * src14 + btmp5  * src15);
        float dst9  = (btmp1  * src12 + btmp6  * src14 + btmp9  * src15)
                      - (btmp0  * src12 + btmp7  * src14 + btmp8  * src15);
        float dst10 = (btmp2  * src12 + btmp7  * src13 + btmp10 * src15)
                      - (btmp3  * src12 + btmp6  * src13 + btmp11 * src15);
        float dst11 = (btmp5  * src12 + btmp8  * src13 + btmp11 * src14)
                      - (btmp4  * src12 + btmp9  * src13 + btmp10 * src14);
        float dst12 = (btmp2  * src10 + btmp5  * src11 + btmp1  * src9 )
                      - (btmp4  * src11 + btmp0  * src9  + btmp3  * src10);
        float dst13 = (btmp8  * src11 + btmp0  * src8  + btmp7  * src10)
                      - (btmp6  * src10 + btmp9  * src11 + btmp1  * src8 );
        float dst14 = (btmp6  * src9  + btmp11 * src11 + btmp3  * src8 )
                      - (btmp10 * src11 + btmp2  * src8  + btmp7  * src9 );
        float dst15 = (btmp10 * src10 + btmp4  * src8  + btmp9  * src9 )
                      - (btmp8  * src9  + btmp11 * src10 + btmp5  * src8 );
        float det =
                src0 * dst0 + src1 * dst1 + src2 * dst2 + src3 * dst3;
        assert (det != 0.0f);
        float invdet = 1.0f / det;
        mInv[     mInvOffset] = dst0  * invdet;
        mInv[ 1 + mInvOffset] = dst1  * invdet;
        mInv[ 2 + mInvOffset] = dst2  * invdet;
        mInv[ 3 + mInvOffset] = dst3  * invdet;
        mInv[ 4 + mInvOffset] = dst4  * invdet;
        mInv[ 5 + mInvOffset] = dst5  * invdet;
        mInv[ 6 + mInvOffset] = dst6  * invdet;
        mInv[ 7 + mInvOffset] = dst7  * invdet;
        mInv[ 8 + mInvOffset] = dst8  * invdet;
        mInv[ 9 + mInvOffset] = dst9  * invdet;
        mInv[10 + mInvOffset] = dst10 * invdet;
        mInv[11 + mInvOffset] = dst11 * invdet;
        mInv[12 + mInvOffset] = dst12 * invdet;
        mInv[13 + mInvOffset] = dst13 * invdet;
        mInv[14 + mInvOffset] = dst14 * invdet;
        mInv[15 + mInvOffset] = dst15 * invdet;
    }
};

class Mat4{
public:
    float* matrix;
public:
    Mat4();
    ~Mat4();
    void loadIdentity();
    void setTranslation(Vector3f* v);
    void genRotationFromEulerAngle(Vector3f* angles);
    Mat4* mul(Mat4* m1, Mat4* m2);
    void copyFrom(Mat4* m);
    void getRotateFromQuaternion(Vector4f* v);
    Vector3f* invTransformAndRotate(Vector3f* point);
    Vector3f* transform(Vector3f* point);
    void translate(float x, float y, float z);
    void rotate(float angle, float x, float y, float z);
    void scale(float x, float y, float z);
    static void print_matrix(Mat4 m);
};

#endif //SHATTER_ENGINE_SHATTER_ITEM_H
