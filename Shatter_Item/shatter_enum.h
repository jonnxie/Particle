//
// Created by maybe on 2021/5/12.
//

#ifndef SHATTER_ENGINE_SHATTER_ENUM_H
#define SHATTER_ENGINE_SHATTER_ENUM_H

// #include <glm.hpp>
#include <string>

enum class ShaderName{
    CPU_Particle,
    Default,
    Marching_Cube,
    Marching_Cube_Density,
    Marching_Cube_Vertex,
    Marching_Cube_Surface,
    Boom_Integrate_Buffer,
    Boom_Integrate_Image,
    Integrate_GET_IMAGE,
};

enum class ModelName {
    Boom_Model,
    Polyline,
    Under_Screen_Line,
    CPU_Particle,
    Marching_Cube,
    Marching_Cube_Debug,
    Integrate_Box,
    PC_UV,
};

enum class LightType{
    Parallel,
    Point,
    Spot,
};

enum class PipelineType{
    CPU_Particle,
    Boom_Integrate_Image,
    Boom_Integrate_Buffer,
    PC_UV,
    Gen_Image,
    Marching_Cube,
    Marching_Cube_Density,
    Marching_Cube_Surface,
};

enum class InputEvent{
    Mouse_Move = 0x00000001,
    Mouse_Down = 0x00000002,
    Mouse_Up   = 0x00000004,
    Key_Down   = 0x00000008,
    Key_Up     = 0x00000010,
};

typedef uint32_t Shatter_Action;

typedef enum Response_Action {
    Shatter_ViewMove = 0x00000001,
    Shatter_DrawLine = 0x00000002,
    Shatter_MainResponese = 0x00000003
} Response_Action;

enum class Input_Type{
    Basic,
    Point3d ,
    Point2d,
    Point2dUV3d,
    Point2dColor,
    Point3dNormal,
    Point3dColor,
    Point3dColorUV,
    Point3dUV,
    Point3dUV1d,
    Point3dNormalUV, // gltf
    Point3dColorSize,
    FrustumCullSample,
    FrustumCullSampleInstance,
    TransparentGlass,
    Building,
    NONE ,
};

enum class Buffer_Type{
    Vertex_Buffer = 0,
    Index_Buffer,
    Uniform_Buffer,
    Storage_Buffer,
    VS_Buffer,
    Vertex_Host_Buffer,
    Storage_Host_Buffer,
};

enum class ObjectType{
    Line3d,
    Plane3d,
};

typedef std::string B_id;

typedef uint32_t D_id;

typedef std::string P_id;

typedef uint32_t U_id;

typedef std::string Set_id;

typedef std::string Slb_id;

typedef std::string Sl_id;

typedef std::string Shader_id;

typedef std::string SGroup_id;

typedef std::string Task_id;

typedef uint32_t ShaderType;

#define VertexShader    0
#define FragmentShader  1
#define ComputeShader   2
#define RayGen          3
#define RayMiss         4
#define RayClosest      5
#define TaskShader      6
#define MeshShader      7

typedef std::string Input_Action;
typedef uint32_t State;
#define unactivated 0
#define activate 1

using STATE = bool;

#define STATE_OLD false
#define STATE_NEW true

#define IDLE true
#define ACTIVE false

enum class AssemState{
    Point_List,
    Line_Strip,
    Line_List,
    Triangle_List,
    Triangle_Strip,
};

enum class BlendState{
    /*
     * no transparent
     */
    Default,
    CPU_Particle,
    Particle,
    GPass,
};

enum class RenderPassType{
    Default,
    CascadeShadow,
    OffScreen,
};

enum class RasterState{
    Point,
    Polyline,
    TriangleLine,
    TriangleFace,
    CPU_Particle,
    Marching_Cube,
    Back_Face,
};

enum class DrawType{
    Index,
    Vertex,
    Indirect,
};

enum class MultisampleState{
    Default,
};

enum class DepthStencilState{
    Default,
    UnderScreenLine,
	DepthWriteDisable,
	DepthDisable,
    MaxDepth,
};

enum class Relation3D{
    Front,
    Back,
    Planar,
    Spanning,
    Clipped,
};

enum class InsertTaskState{
    Pre,
    Later,
};

enum class TaskType{
    Normal,
    Update,
};

enum class ReactionType{
    Key,
    Mouse,
};

enum class DType{
    Normal,
    RayTracing,
    Instance,
};

enum class ImageType{
    OneDimension,
    TwoDimension,
    ThreeDimension,
    CubeDimension,
    DDSDimension,
    OneDATDimension,
    BntexDimension,
    KTXDimension,
};

enum class InputMode{
    InputPoint,
};

enum class Event{
    SingleClick = 0,
    SinglePress,
    DoubleClick,
    DoublePress,
    DeletePress,
    MouseMove,
    Count,
};

enum class Color{
    Ambient,
    Diffuse,
    Specular,
    Emissive,
};

enum class MeshDrawType{
    Line,
    Face,
};

enum Attachment{
    AttachmentBack = 0,
    AttachmentPosition,
    AttachmentNormal,
    AttachmentAlbedo,
    AttachmentDepth,
//    AttachmentOpaque,
//    AttachmentTransparency,
    AttachmentCount,
};

enum Subpass{
//    SubpassDepth = 0,
//    SubpassTransparency,
    SubpassG = 0,
    SubpassLight,
    SubpassTransparency,
//    SubpassComposite,
    SubpassCount,
};

enum SubpassDependency{
    ExternalToG = 0,
    GtoLight,
    LightToTransparency,
    TransparencyToExternal,
    DependencyCount,
};

//enum SubpassDependency{
////    ExternalToDepth = 0,
//    DepthToTransparency = 0,
//    DepthToG,
//    GtoLight,
//    LightToComposite,
//    TransparencyToComposite,
////    CompositeToColor,
//    DependencyCount,
//};

/*
 * custom color
 */
// const glm::vec3 RED_COLOR{1.0f,0.0f,0.0f};
// const glm::vec3 YELLOW_COLOR{1.0f,1.0f,0.0f};
// const glm::vec3 GREEN_COLOR{0.0f,0.5f,0.0f};
// const glm::vec3 PURPLE_COLOR{0.5f,0.0f,0.5f};
// const glm::vec3 BLUE_COLOR{0.0f,0.0f,1.0f};
// const glm::vec3 CYAN_COLOR{0.0f,1.0f,1.0f};
// const glm::vec3 GOLD_COLOR{1.0f,0.84f,0.0f};
// const glm::vec3 ORANGE_COLOR{1.0f,0.64f,0.0f};
// const glm::vec3 WHITE_COLOR{1.0f,1.0f,1.0f};
// const glm::vec3 BLACK_COLOR{0.0f,0.0f,0.0f};


#define pai 3.141592f
#define two_pai 6.283184f
#define half_pai 1.57079f
//#define degree (pai/180.0f)
#define degree 0.017453f
#define float_limit 0.000001f

#define one_byte 1
#define one_vec3 12
#define one_vec4 16
#define one_matrix 64

#define defaultModelCount 20

#define thread_num 10

#define model_num 100

#define SHADOWMAP_DIM 4096

#define SHADOW_MAP_CASCADE_COUNT 4

#define CascadeShadowSize 256

#define numObjectsPerThread 2

#define ObjectCatalog      "../Shatter_Object/object.h"
#define CObjectCatalog     "../Shatter_Object/cobject.h"
#define DObjectCatalog     "../Shatter_Object/dobject.h"
#define GObjectCatalog     "../Shatter_Object/gobject.h"
#define MPoolCatalog       "../Shatter_Object/mpool.h"
#define BPoolCatalog       "../Shatter_Object/bpool.h"
#define PPoolCatalog       "../Shatter_Object/ppool.h"
#define DeviceCatalog      "../Shatter_Object/device.h"
#define SetPoolCatalog     "../Shatter_Object/setpool.h"
#define SlbPoolCatalog     "../Shatter_Object/slbpool.h"
#define ModelSetCatalog    "../Shatter_Object/modelsetpool.h"
#define TaskCatalog        "../Shatter_Object/taskpool.h"
#define ShaderCatalog      "../Shatter_Object/shaderpool.h"
#define GLTFCatalog        "../Shatter_Object/VulkanglTFModels.h"
#define CameraCatalog      "../Shatter_Object/camera.h"
#define SGroupCatalog      "../Shatter_Object/shadergrouppool.h"
#define OffScreenCatalog   "../Shatter_Object/offscreen.h"
#define ListenerCatalog    "../Shatter_Object/listener.h"
#define MathCatalog        "../Shatter_Object/shatter_math.h"
#define MemPoolCatalog     "../Shatter_Object/mempool.hpp"
#define InputTypeCatalog   "../Shatter_Object/inputtype.h"
#define InputActionCatalog "../Shatter_Object/inputaction.h"
#define BufferCatalog   "../Shatter_Buffer/shatterbufferinclude.h"
#define PipelineCatalog "../Shatter_Render/pipeline.h"
#define RenderCatalog   "../Shatter_Render/shatter_render_include.h"
#define PCatalog        "../Shatter_Render/pipeline.h"
#define AppCatalog      "../Shatter_App/shatterapp.h"

#define ItemCatalog     "../Shatter_Item/shatter_item.h"
#define MacroCatalog    "../Shatter_Item/shatter_macro.h"
#define ConfigCatalog   "../Shatter_Item/configs.h"
#define UniformCatalog  "../Shatter_Item/uniform.h"

#define GuiCatalog      "../Shatter_Base/GUI.h"
//#define RayCatalog      "../Shatter_Base/Raytracing.h"

#define ShaderFilePath      "C:/Users/AnWell/Desktop/Particle/shaders/"
#define ModelFilePath       "C:/Users/AnWell/Desktop/ASSET/models/"
#define TextureFilePath     "C:/Users/AnWell/Desktop/ASSET/textures/"

#endif //SHATTER_ENGINE_SHATTER_ENUM_H
