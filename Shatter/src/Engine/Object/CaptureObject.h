//
// Created by jonnxie on 2022/4/22.
//

#ifndef MAIN_CAPTUREOBJECT_H
#define MAIN_CAPTUREOBJECT_H

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Base/lines.h"

template<class Base>
class CaptureObject {
public:
    static CaptureObject* mallocCapture(const glm::vec3& _min, const glm::vec3& _max, int _drawId) {
        auto pool = MPool<AABB>::getPool();
        int boxIndex = MPool<AABB>::getPool()->malloc();
        uint32_t captureId = boxIndex + 1;
        int modelIndex = ModelSetPool::getPool().malloc();
        TaskPool::pushUpdateTask(tool::combine("Capture", boxIndex),[&, modelIndex, _drawId](float _abs_time){
            glm::mat4* ptr = SingleBPool.getModels();
            memcpy(ptr + modelIndex, &(*SingleDPool)[_drawId]->m_matrix, one_matrix);
        });
        (*pool)[boxIndex]->addInternalPoint(_min);
        (*pool)[boxIndex]->addInternalPoint(_max);
        (*pool)[boxIndex]->m_model_index = modelIndex;
        SingleRender.aabb_map[captureId] = boxIndex;
        std::vector<glm::vec3> vertexBuffer{};
        genFaceVertexBufferFromAABB(*(*SingleAABBPool)[boxIndex], vertexBuffer);
        SingleBPool.createVertexBuffer(tool::combine("Capture", captureId), vertexBuffer.size() * one_vec3, vertexBuffer.data());

        SingleBPool.createUniformBuffer(tool::combine("Capture",captureId), 4);
        auto buffer = SingleBPool.getBuffer(tool::combine("Capture",captureId),Buffer_Type::Uniform_Buffer);
        buffer->map();
        memcpy(buffer->mapped, &captureId, 4);
        buffer->unmap();
        SingleSetPool.AllocateDescriptorSets({"CaptureVal"}, &(*pool)[boxIndex]->m_capture_set);

        VkDescriptorBufferInfo descriptorBufferInfos{buffer->getBuffer(), 0, 4};
        VkWriteDescriptorSet writeDescriptorSets = tool::writeDescriptorSet((*pool)[boxIndex]->m_capture_set,
                                                                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                            0,
                                                                            &descriptorBufferInfos);

        vkUpdateDescriptorSets(SingleDevice(), 1, &writeDescriptorSets, 0, nullptr);
    };
    CaptureObject(Base* _parent, int _captureId, int _boxId):
    parent(_parent),
    captureId(_captureId),
    boxId(_boxId) {
        bufferId = tool::combine("CaptureObject", captureId);
    };
    ~CaptureObject() {
        vkQueueWaitIdle(SingleRender.graphics_queue);
        SingleBPool.freeBuffer(bufferId, Buffer_Type::Vertex_Host_Buffer);
        SingleBPool.freeBuffer(bufferId, Buffer_Type::Uniform_Buffer);
        SingleRender.releaseObject(int(captureId),DrawObjectType::AABB);
        MPool<AABB>::getPool()->free(boxId);
        SingleRender.normalChanged = true;
    };
    DefineUnCopy(CaptureObject);
public:
    ClassElement(color, glm::vec3, Color);
    void drawBox() {
        line = std::make_unique<AABBLine>(boxId, captureId, color);
    };
private:
    Base* parent;
    std::unique_ptr<AABBLine> line;
    std::string bufferId;
    uint32_t captureId;
    int boxId;
};


#endif //MAIN_CAPTUREOBJECT_H
