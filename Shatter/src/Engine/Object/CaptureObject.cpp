//
// Created by jonnxie on 2022/4/22.
//

#include "precompiledhead.h"
#include "CaptureObject.h"
#include "Engine/Base/lines.h"
#include "Engine/Object/object.h"
#include "Engine/Object/aabb.h"

std::unique_ptr<CaptureObject>
CaptureObject::mallocCapture(Object *_parent, const glm::vec3 &_min, const glm::vec3 &_max, int _drawId) {
    auto pool = MPool<AABB>::getPool();
    int boxIndex = pool->malloc();
    uint32_t captureId = boxIndex + 1;
    int modelIndex = ModelSetPool::getPool().malloc();
    auto bufferId = tool::combine("Capture", captureId);
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
    SingleBPool.createVertexBuffer(bufferId, vertexBuffer.size() * one_vec3, vertexBuffer.data());

    SingleBPool.createUniformBuffer(bufferId, 4);
    auto buffer = SingleBPool.getBuffer(bufferId, Buffer_Type::Uniform_Buffer);
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
//        auto ptr = new CaptureObject(_parent, captureId, boxIndex);
    return std::move(std::make_unique<CaptureObject>(_parent, captureId, boxIndex));
}

CaptureObject::CaptureObject(Object *_parent, int _boxId, int _drawId):
        parent(_parent),
        captureId(_boxId + 1),
        boxId(_boxId) {
    bufferId = tool::combine("Capture", captureId);
    auto pool = MPool<AABB>::getPool();
    int modelIndex = ModelSetPool::getPool().malloc();
    TaskPool::pushUpdateTask(tool::combine("Capture", boxId),[&, modelIndex, _drawId](float _abs_time){
        glm::mat4* ptr = SingleBPool.getModels();
        memcpy(ptr + modelIndex, &(*SingleDPool)[_drawId]->m_matrix, one_matrix);
    });
    (*pool)[boxId]->m_model_index = modelIndex;
    SingleRender.aabb_map[captureId] = boxId;
    std::vector<glm::vec3> vertexBuffer{};
    genFaceVertexBufferFromAABB(*(*SingleAABBPool)[boxId], vertexBuffer);
    SingleBPool.createVertexBuffer(bufferId, vertexBuffer.size() * one_vec3, vertexBuffer.data());

    auto vertex_buffer = SingleBPool.getBuffer(bufferId, Buffer_Type::Vertex_Buffer);
    SingleBPool.createUniformBuffer(bufferId, 4);
    auto buffer = SingleBPool.getBuffer(bufferId, Buffer_Type::Uniform_Buffer);
    buffer->map();
    memcpy(buffer->mapped, &captureId, 4);
    buffer->unmap();
    SingleSetPool.AllocateDescriptorSets({"CaptureVal"}, &(*pool)[boxId]->m_capture_set);

    VkDescriptorBufferInfo descriptorBufferInfos{buffer->getBuffer(), 0, 4};
    VkWriteDescriptorSet writeDescriptorSets = tool::writeDescriptorSet((*pool)[boxId]->m_capture_set,
                                                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                        0,
                                                                        &descriptorBufferInfos);

    vkUpdateDescriptorSets(SingleDevice(), 1, &writeDescriptorSets, 0, nullptr);
}

CaptureObject::CaptureObject(Object *_parent, uint32_t _captureId, int _boxId):
        parent(_parent),
        captureId(_captureId),
        boxId(_boxId) {
    bufferId = tool::combine("Capture", captureId);
}

CaptureObject::~CaptureObject() {
    vkQueueWaitIdle(SingleRender.graphics_queue);
    std::cout << "Capture Object: " << captureId << " released" << std::endl;
    /*
     * Box vertex buffer
     */
    SingleBPool.freeBuffer(bufferId, Buffer_Type::Vertex_Buffer);
    /*
     * Capture value uniform buffer
     */
    SingleBPool.freeBuffer(bufferId, Buffer_Type::Uniform_Buffer);
    SingleRender.releaseObject(int(captureId),DrawObjectType::AABB);
    MPool<AABB>::getPool()->free(boxId);
    SingleRender.normalChanged = true;
}

void CaptureObject::drawBox() {
    line = std::make_unique<AABBLine>(boxId, captureId, color);
}
