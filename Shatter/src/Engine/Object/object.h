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
#include "dobject.h"
#include "Engine/Pool/mpool.h"
#include SetPoolCatalog
#include DeviceCatalog
#include RenderCatalog
#include "Engine/Pool/modelsetpool.h"
#include TaskCatalog
#include BufferCatalog

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
    Object()
    {
        m_capture_id = mallocCaptureId();
        auto aabbPool = MPool<AABB>::getPool();
        m_aabbIndex = aabbPool->malloc();

        SingleBPool.createUniformBuffer(tool::combine("Capture",m_capture_id), 4);
        auto buffer = SingleBPool.getBuffer(tool::combine("Capture",m_capture_id),Buffer_Type::Uniform_Buffer);
        buffer->map();
        memcpy(buffer->mapped, &m_capture_id, 4);
        buffer->unmap();
        SingleSetPool.AllocateDescriptorSets({"CaptureVal"}, &(*aabbPool)[m_aabbIndex]->m_capture_set);

        VkDescriptorBufferInfo descriptorBufferInfos{buffer->getBuffer(), 0, 4};
        VkWriteDescriptorSet writeDescriptorSets = tool::writeDescriptorSet((*aabbPool)[m_aabbIndex]->m_capture_set,
                                                                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                            0,
                                                                            &descriptorBufferInfos);

        vkUpdateDescriptorSets(SingleDevice(), 1, &writeDescriptorSets, 0, nullptr);
    };
    virtual ~Object()
    {
        release();
    };
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
                SingleRender.getDObjects()->push_back(_id);
                break;
            }
            case DrawObjectType::OffScreen:
            {
                SingleRender.getOffDObjects()->push_back(_id);
                break;
            }
            case DrawObjectType::Transparency:
            {
                SingleRender.getTObjects()->push_back(_id);
                break;
            }
            case DrawObjectType::Normal:
            {
                SingleRender.getNObjects()->push_back(_id);
                break;
            }
        }
    };

    void insertRenderObject(DrawObjectType _type, int _id)
    {
        switch (_type) {
            case DrawObjectType::Default:
            {
                SingleRender.getDObjects()->push_back(_id);
                break;
            }
            case DrawObjectType::OffScreen:
            {
                SingleRender.getOffDObjects()->push_back(_id);
                break;
            }
            case DrawObjectType::Transparency:
            {
                SingleRender.getTObjects()->push_back(_id);
                break;
            }
            case DrawObjectType::Normal:
            {
                SingleRender.getNObjects()->push_back(_id);
                break;
            }
        }
    };
    void addGPUCaptureComponent(const glm::vec3& _min, const glm::vec3& _max, int _dId) const
    {
        int model_index = ModelSetPool::getPool().malloc();
        int d = _dId;
        TaskPool::pushUpdateTask(tool::combine("Capture", m_aabbIndex),[&, model_index, d](float _abs_time){
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + model_index, &(*SingleDPool)[d]->m_matrix, one_matrix);
        });
        auto aabbPool = MPool<AABB>::getPool();
        (*aabbPool)[m_aabbIndex]->addInternalPoint(_min);
        (*aabbPool)[m_aabbIndex]->addInternalPoint(_max);
        (*aabbPool)[m_aabbIndex]->m_model_index = model_index;
        SingleRender.aabb_map[int(m_capture_id)] = m_aabbIndex;

        std::vector<glm::vec3> aabbBuffer{};
        genFaceVertexBufferFromAABB(*(*SingleAABBPool)[m_aabbIndex], aabbBuffer);
        SingleBPool.createVertexBuffer(tool::combine("Capture", m_aabbIndex), aabbBuffer.size() * one_vec3, aabbBuffer.data());
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
    };
    std::vector<int> m_dobjs{};
    std::vector<int> m_gobjs{};
    std::vector<int> m_cobjs{};
    int m_aabbIndex;
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
    ClassProtectedReferenceElement(m_plane, TargetPlane, TargetPlane);
    ClassProtectedReferenceElement(m_center, glm::vec3, WorkCenter);
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
