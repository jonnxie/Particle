//
// Created by jonnxie on 2022/4/22.
//

#include "precompiledhead.h"
#include "CaptureObject.h"
#include "Engine/Base/lines.h"
#include "Engine/Object/object.h"
#include "Engine/Object/aabb.h"
#include "Engine/App/shatterapp.h"
#include ConfigCatalog

std::shared_ptr<CaptureObject>
CaptureObject::mallocCapture(Object *_parent,
                             const glm::vec3 &_min,
                             const glm::vec3 &_max,
                             int _drawId,
                             const std::string& _name) {
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
    genFaceVertexBufferFromAABB(*(*SingleBoxPool)[boxIndex], vertexBuffer);
    SingleBPool.createVertexBuffer(bufferId, vertexBuffer.size() * one_vec3, vertexBuffer.data());

    SingleBPool.createUniformBuffer(bufferId, 4);
    auto buffer = SingleBPool.getBuffer(bufferId, Buffer_Type::Uniform_Buffer);
    buffer->map();
    memcpy(buffer->mapped, &captureId, 4);
    buffer->unmap();
    SingleSetPool.AllocateDescriptorSets({"CaptureVal"}, &(*pool)[boxIndex]->m_capture_set);

    std::cout << _name + "`s UniformBuffer: " << buffer->getBuffer() << std::endl;
    std::cout << _name + "`s DescriptorSet: " << (*pool)[boxIndex]->m_capture_set << std::endl;

    VkDescriptorBufferInfo descriptorBufferInfos{buffer->getBuffer(), 0, 4};
    VkWriteDescriptorSet writeDescriptorSets = tool::writeDescriptorSet((*pool)[boxIndex]->m_capture_set,
                                                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                        0,
                                                                        &descriptorBufferInfos);

    vkUpdateDescriptorSets(SingleDevice(), 1, &writeDescriptorSets, 0, nullptr);
    auto ptr = std::make_shared<CaptureObject>(_parent, captureId, boxIndex);
    SingleAPP.capturedPush(ptr);
    return ptr;
}

CaptureObject::CaptureObject(Object *_parent, int _boxId, int _drawId, const std::string& _name):
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
    genFaceVertexBufferFromAABB(*(*SingleBoxPool)[boxId], vertexBuffer);
    SingleBPool.createVertexBuffer(bufferId, vertexBuffer.size() * one_vec3, vertexBuffer.data());

    auto vertex_buffer = SingleBPool.getBuffer(bufferId, Buffer_Type::Vertex_Buffer);
    SingleBPool.createUniformBuffer(bufferId, 4);
    auto buffer = SingleBPool.getBuffer(bufferId, Buffer_Type::Uniform_Buffer);
    buffer->map();
    memcpy(buffer->mapped, &captureId, 4);
    buffer->unmap();
    SingleSetPool.AllocateDescriptorSets({"CaptureVal"}, &(*pool)[boxId]->m_capture_set);

//    std::cout << _name + "`s UniformBuffer: " << buffer->getBuffer() << std::endl;
//    std::cout << _name + "`s DescriptorSet: " << (*pool)[boxId]->m_capture_set << std::endl;

    VkDescriptorBufferInfo descriptorBufferInfos{buffer->getBuffer(), 0, 4};
    VkWriteDescriptorSet writeDescriptorSets = tool::writeDescriptorSet((*pool)[boxId]->m_capture_set,
                                                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                        0,
                                                                        &descriptorBufferInfos);

    vkUpdateDescriptorSets(SingleDevice(), 1, &writeDescriptorSets, 0, nullptr);
    line = std::make_unique<AABBLine>(boxId, captureId, color);
    line->hide();
}

CaptureObject::CaptureObject(Object *_parent, uint32_t _captureId, int _boxId):
        parent(_parent),
        captureId(_captureId),
        boxId(_boxId) {
    bufferId = tool::combine("Capture", captureId);
    line = std::make_unique<AABBLine>(boxId, captureId, color);
    line->hide();
}

CaptureObject::~CaptureObject() {
    if (!Config::getConfig("RendererReleased")) {
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
}

void CaptureObject::drawBox() {
    if (!line) {
        line = std::make_unique<AABBLine>(boxId, captureId, color);
    }else {
        line->show();
    }
}

void CaptureObject::hide() {
    if (line) {
        line->hide();
    }
}
