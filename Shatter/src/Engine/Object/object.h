//
// Created by maybe on 2021/6/3.
//

#ifndef SHATTER_ENGINE_OBJECT_H
#define SHATTER_ENGINE_OBJECT_H

#include <vulkan/vulkan.h>
#include <vector>
#include "aabb.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_enum.h"

static int initCaptureIdVal = 1;
static std::mutex captureIdLock;

static int mallocCaptureId()
{
    std::lock_guard<std::mutex> lockGuard(captureIdLock);
    return initCaptureIdVal++;
}

struct CaptureObject{
    int model_index;
    int aabb_box;
};

class Object {
public:
    Object();
    ~Object();
    DefineUnCopy(Object);
public:
    void draw(VkCommandBuffer _cb);
    void init();
    virtual void update(float){};
    virtual void imguiRender(){};
    void insertRenderObject(int _id);
    void insertRenderObject(DrawObjectType _type, int _id);
    void insertDObject(int);
    void insertGObject(int);
    void insertCObject(int);
    virtual void release();
    std::vector<int> m_dobjs{};
    std::vector<int> m_gobjs{};
    std::vector<int> m_cobjs{};
    int m_aabbIndex;
public:
    void construct();
    virtual void constructG(){};
    virtual void constructD(){};
    virtual void constructC(){};
    ClassElement(m_draw_type, DrawObjectType, DrawType);
protected:
    glm::mat4   m_world{};
    glm::mat4   m_scale{};
    glm::mat4   m_rotate{};
    glm::mat4   m_translation{};
    uint32_t    m_capture_id{};
//    VkDescriptorSet m_capture_set = VK_NULL_HANDLE;
};


#endif //SHATTER_ENGINE_OBJECT_H
