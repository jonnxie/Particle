//
// Created by maybe on 2021/5/12.
//

#ifndef SHATTER_ENGINE_SHATTER_ENUM_H
#define SHATTER_ENGINE_SHATTER_ENUM_H

#include "precompiledhead.h"
#include <glm.hpp>
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

//enum class InputEvent{
//    Mouse_Move = 0x00000001,
//    Mouse_Down = 0x00000002,
//    Mouse_Up   = 0x00000004,
//    Key_Down   = 0x00000008,
//    Key_Up     = 0x00000010,
//};

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
    GLTF,
    GLTFInstance,
    NONE ,
};

enum class Buffer_Type{
    Vertex_Buffer = 0,
    Index_Host_Buffer,
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
    Capture,
};

enum class RenderPassType{
    CascadeShadow,
    OffScreen,
    Capture,
    Present,
    Color,
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
    MouseClick,
    SinglePress,
    DoubleClick,
    DoublePress,
    DeletePress,
    MouseMove,
    Count,
};

enum class DrawObjectType{
    Default,
    OffScreen,
    Transparency,
    Normal,
    AABB,
};

enum class WindowEvent{
    Resize,
    Key,
    MouseButton,
    CursorPos,
    Scroll,
    Char
};

using eventCallback = std::pair<WindowEvent,std::function<void()>>;

enum class Color{
    Ambient,
    Diffuse,
    Specular,
    Emissive,
};

enum class MeshDrawType {
    Point,
    Line,
    Face,
};

enum class SpatialType
{
    Abstract,
    Point,
    Triangle,
    Tetrahedron,
    Sphere,
};

enum class GeoRelation
{
    STRICTLY_OUTSIDE = 0,  // strictly outside the input simplex
    STRICTLY_INSIDE  = 1,  // strictly inside  the input simplex
    ON_VERT0         = 2,  // used for segs, tris and tets
    ON_VERT1         = 3,  // used for segs, tris and tets
    ON_VERT2         = 4,  // used for tris and tets
    ON_VERT3         = 5,  // used for tets
    ON_EDGE0         = 6,  // used for tris and tets
    ON_EDGE1         = 7,  // used for tris and tets
    ON_EDGE2         = 8,  // used for tris and tets
    ON_EDGE3         = 9,  // used for tets
    ON_EDGE4         = 10, // used for tets
    ON_EDGE5         = 11, // used for tets
    ON_FACE0         = 12, // used for tets
    ON_FACE1         = 13, // used for tets
    ON_FACE2         = 14, // used for tets
    ON_FACE3         = 15, // used for tets
};

enum Attachment{
    AttachmentBack = 0,
    AttachmentPosition,
    AttachmentNormal,
    AttachmentAlbedo,
    AttachmentDepth,
    AttachmentCount,
};

enum MSAAAttachment{
    MSAAAttachmentBack = 0,
    MSAAAttachmentPosition,
    MSAAAttachmentNormal,
    MSAAAttachmentAlbedo,
    MSAAAttachmentDepth,
    MSAAAttachmentPresent,
    MSAAAttachmentCount,
};

enum Subpass{
    SubpassG = 0,
    SubpassLight,
    SubpassTransparency,
    SubpassCount,
};

enum SubpassDependency{
    ExternalToG = 0,
    GtoLight,
    LightToTransparency,
    TransparencyToExternal,
    DependencyCount,
};

enum class ComponentType{
    None = -1,
    BYTE = 0,
    UNSIGNED_BYTE,
    SHORT,
    UNSIGNED_SHORT,
    INT,
    UNSIGNED_INT,
    FLOAT,
    DOUBLE,
};

enum class FeatureType{
    None = 0,
    SCALAR = 1,
    VEC2,
    VEC3,
    VEC4,
};

enum class TextureType {
    Texture1D,
    Texture2DDefault,
    Texture3D,
};

enum class MouseState {
    ViewPort,
    Manipulate,
};

/*
 * custom color
 */
const glm::vec3 RED_COLOR{1.0f,0.0f,0.0f};
const glm::vec3 YELLOW_COLOR{1.0f,1.0f,0.0f};
const glm::vec3 GREEN_COLOR{0.0f,0.5f,0.0f};
const glm::vec3 PURPLE_COLOR{0.5f,0.0f,0.5f};
const glm::vec3 BLUE_COLOR{0.0f,0.0f,1.0f};
const glm::vec3 CYAN_COLOR{0.0f,1.0f,1.0f};
const glm::vec3 GOLD_COLOR{1.0f,0.84f,0.0f};
const glm::vec3 ORANGE_COLOR{1.0f,0.64f,0.0f};
const glm::vec3 WHITE_COLOR{1.0f,1.0f,1.0f};
const glm::vec3 BLACK_COLOR{0.0f,0.0f,0.0f};

const glm::vec3 UpDirection{0.0f, 0.0f, 1.0f};
const glm::vec3 DownDirection{0.0f, 0.0f, -1.0f};
const glm::vec3 LeftDirection{-1.0f, 0.0f, 0.0f};
const glm::vec3 RightDirection{1.0f, 0.0f, 0.0f};
const glm::vec3 ForwardDirection{0.0f, 1.0f, 0.0f};
const glm::vec3 BackDirection{0.0f, -1.0f, 0.0f};

#define pai 3.141592f
#define two_pai 6.283184f
#define half_pai 1.57079f
//#define degree (pai/180.0f)
#define degree 0.017453f
#define float_limit 0.000001f

#define one_byte 1
#define one_vec2 8
#define one_vec3 12
#define one_vec4 16
#define one_matrix 64

#define defaultModelCount 20

#define CaptureBoxVertexCount 36

#define thread_num 10

#define model_num 100

#define SHADOWMAP_DIM 4096

#define SHADOW_MAP_CASCADE_COUNT 4

#define CascadeShadowSize 256

#define numObjectsPerThread 2

#define TreeSize 8

#define UNLOADED  0
#define LOADING  1
#define PARSING  2
#define LOADED  3
#define FAILED  4

#define ObjectCatalog      "Engine/Object/object.h"
#define CObjectCatalog     "Engine/Object/cobject.h"
#define DObjectCatalog     "Engine/Object/dobject.h"
#define GObjectCatalog     "Engine/Object/gobject.h"
#define SceneCatalog       "Engine/Object/scene.h"
#define MPoolCatalog       "Engine/Pool/mpool.h"
#define BPoolCatalog       "Engine/Pool/bpool.h"
#define PPoolCatalog       "Engine/Pool/ppool.h"
#define DeviceCatalog      "Engine/Object/device.h"
#define SetPoolCatalog     "Engine/Pool/setpool.h"
#define SlbPoolCatalog     "Engine/Pool/slbpool.h"
#define ModelSetCatalog    "Engine/Pool/modelsetpool.h"
#define TaskCatalog        "Engine/Event/taskpool.h"
#define ShaderCatalog      "Engine/Pool/shaderpool.h"
#define TexturePoolCatalog "Engine/Pool/TexturePool.h"
#define GLTFCatalog        "Engine/Object/VulkanglTFModels.h"
#define CameraCatalog      "Engine/Object/camera.h"
#define SGroupCatalog      "Engine/Pool/shadergrouppool.h"
#define OffScreenCatalog   "Engine/Object/offscreen.h"
#define ListenerCatalog    "Engine/Object/listener.h"
#define CaptureCatalog     "Engine/Object/CaptureObject.h"
#define MathCatalog        "Engine/Item/shatter_math.h"
#define MemPoolCatalog     "Engine/Pool/mempool.hpp"
#define InputTypeCatalog   "Engine/Object/inputtype.h"
#define InputActionCatalog "Engine/Object/inputaction.h"
#define PrecompiledCatalog "precompiledhead.h"
#define AABBCatalog        "Engine/Object/aabb.h"
#define BufferCatalog      "Engine/Buffer/shatterbufferinclude.h"
#define PipelineCatalog    "Engine/Renderer/pipeline.h"
#define RenderCatalog      "Engine/Renderer/shatter_render_include.h"
#define PCatalog           "Engine/Renderer/pipeline.h"
#define AppCatalog         "Engine/App/shatterapp.h"
#define ManipulateCatalog  "Engine/Base/Manipulate.h"

#define ItemCatalog        "Engine/Item/shatter_item.h"
#define MacroCatalog       "Engine/Item/shatter_macro.h"
#define ConfigCatalog      "Engine/Item/configs.h"
#define UniformCatalog     "Engine/Item/uniform.h"

#define GuiCatalog         "Engine/Base/GUI.h"
#define DPointCatalog      "Engine/Base/points.h"
#define DLineCatalog       "Engine/Base/lines.h"
#define HandleCatalog      "Engine/Base/handle.h"
//#define RayCatalog       "Engine/Base/Raytracing.h"

//#define ShaderFilePath      "C:/Users/AnWell/Desktop/Particle/ShatterInut/assets/shaders/"
//#define ModelFilePath       "C:/Users/AnWell/Desktop/ASSET/models/"
//#define TextureFilePath     "C:/Users/AnWell/Desktop/ASSET/textures/"
//#define B3DMPath            "C:/Users/AnWell/Desktop/ASSET/b3dm/"
//#define I3DMPath            "C:/Users/AnWell/Desktop/ASSET/i3dm/"
#define AnimationPath       "C:/Users/AnWell/Desktop/ASSET/animation/"
#define ShaderFilePath      "C:/Users/jonnxie/ClionProjects/Particle/ShatterInut/assets/shaders/"
#define ModelFilePath       "C:/Users/jonnxie/ClionProjects/data/models/"
#define TextureFilePath     "C:/Users/jonnxie/ClionProjects/data/textures/"
#define B3DMPath            "C:/Users/jonnxie/ClionProjects/data/b3dm/"
#define I3DMPath            "C:/Users/jonnxie/ClionProjects/data/i3dm/"


#endif //SHATTER_ENGINE_SHATTER_ENUM_H
