//
// Created by maybe on 2021/6/3.
//

#ifndef SHATTER_ENGINE_OBJECT_H
#define SHATTER_ENGINE_OBJECT_H

#include <vulkan/vulkan.h>
#include <vector>
#include <Engine/Item/configs.h>
#include "aabb.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_enum.h"
#include "dobject.h"
#include "Engine/Pool/mpool.h"
#include SetPoolCatalog
#include DeviceCatalog
#include RenderCatalog
#include "Engine/Pool/modelsetpool.h"
#include TaskCatalog
#include BufferCatalog
#include CaptureCatalog
#include "Engine/Base/Manipulate.h"

static int initCaptureIdVal = 1;
static std::mutex captureIdLock;

class Object;

class CaptureObject;

//class Manipulate;

static int mallocCaptureId()
{
    std::lock_guard<std::mutex> lockGuard(captureIdLock);
    return initCaptureIdVal++;
}

class Object {
public:
    Object() = default;
    virtual ~Object();
    DefineUnCopy(Object);
public:
    void init()
    {
        construct();
    };
    void insertRenderObject(int _id)
    {
        switch (m_draw_type) {
            case DrawObjectType::Default:
            {
                SingleRender.pushDObjects(_id);
                break;
            }
            case DrawObjectType::OffScreen:
            {
                SingleRender.pushOObjects(_id);
                break;
            }
            case DrawObjectType::Transparency:
            {
                SingleRender.pushTObjects(_id);
                break;
            }
            case DrawObjectType::Normal:
            {
                SingleRender.pushNObjects(_id);
                break;
            }
        }
    };

    void insertRenderObject(DrawObjectType _type, int _id)
    {
        switch (_type) {
            case DrawObjectType::Default:
            {
                SingleRender.pushDObjects(_id);
                break;
            }
            case DrawObjectType::OffScreen:
            {
                SingleRender.pushOObjects(_id);
                break;
            }
            case DrawObjectType::Transparency:
            {
                SingleRender.pushTObjects(_id);
                break;
            }
            case DrawObjectType::Normal:
            {
                SingleRender.pushNObjects(_id);
                break;
            }
        }
    };

    void insertDObject(int _obj)
    {
        m_dobjs.emplace_back(_obj);
    };
    void insertGObject(int _obj)
    {
        m_gobjs.emplace_back(_obj);
    };
    void insertCObject(int _obj)
    {
        m_cobjs.emplace_back(_obj);
    };
    void release()
    {
        if (!m_memReleased && !Config::getConfig("RendererReleased")) {
            auto& model_pool = ModelSetPool::getPool();
            auto dpool = MPool<DObject>::getPool();
            for(auto i : m_dobjs){
                dpool->free(i);
                model_pool.free((*dpool)[i]->m_model_index);
            }
            auto gpool = MPool<GObject>::getPool();
            for(auto i: m_gobjs){
                gpool->free(i);
            }
            auto cpool = MPool<CObject>::getPool();
            for(auto i: m_cobjs){
                cpool->free(i);
            }
        }
    };
    std::vector<int> m_dobjs{};
    std::vector<int> m_gobjs{};
    std::vector<int> m_cobjs{};
    int m_boxIndex;
public:
    void construct()
    {
        constructG();
        constructC();
        constructD();
    };

    virtual void constructG(){};

    virtual void constructD(){};

    virtual void constructC(){};

    virtual std::string getName() {
        return "Object";
    };

    virtual SP(Object) copy(const glm::vec3& _position);

    virtual void move(const glm::vec3& _movement);

    virtual void update(const glm::vec3& _movement);

    virtual void gui();

    virtual void show(){};

    virtual void hide(){};

    TargetPlane &getTargetPlane();
    glm::vec3 &getWorkCenter();
    glm::mat4 getRotate();
    glm::mat4 getScale();
    ClassElement(m_draw_type, DrawObjectType, DrawType);
    Manipulate& getManipulate();

    ClassPointerElementInitial(m_captureObject, SP(CaptureObject), Capture, nullptr);
protected:
    std::unique_ptr<Manipulate> m_manipulate{nullptr};
    uint32_t            m_capture_id{};
    bool                m_memReleased = false;
};


#endif //SHATTER_ENGINE_OBJECT_H
