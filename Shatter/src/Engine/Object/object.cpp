//
// Created by maybe on 2021/6/3.
//
#include "precompiledhead.h"

#include "object.h"
#include "dobject.h"
#include "gobject.h"
#include BufferCatalog
#include "Engine/pool/mpool.h"
#include "Engine/pool/modelsetpool.h"
#include SetPoolCatalog
#include DeviceCatalog

Object::~Object(){
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

void Object::draw(VkCommandBuffer _cb) {
    auto pool = MPool<DObject>::getPool();
    for(auto i : m_dobjs){
        (*pool)()[i].draw(_cb);
    }
}

void Object::insertDObject(int _obj){
    m_dobjs.emplace_back(_obj);
//    regenerateCommandBuffer();
}

void Object::insertGObject(int _obj){
    m_gobjs.emplace_back(_obj);
//    regenerateCommandBuffer();
}

void Object::insertCObject(int _obj){
    m_cobjs.emplace_back(_obj);
}

void Object::construct() {
    constructG();
    constructC();
    constructD();
}

void Object::init() {
    construct();
}

Object::Object() {
    m_capture_id = mallocCaptureId();
    SingleBPool.createUniformBuffer(tool::combine("Capture",m_capture_id), 4);
    auto buffer = SingleBPool.getBuffer(tool::combine("Capture",m_capture_id),Buffer_Type::Uniform_Buffer);
    buffer->map();
    memcpy(buffer->mapped, &m_capture_id, 4);
    buffer->unmap();
    SingleSetPool.AllocateDescriptorSets({"CaptureVal"}, &m_capture_set);

    VkDescriptorBufferInfo descriptorBufferInfos{buffer->Get_Buffer(), 0, 4};
    VkWriteDescriptorSet writeDescriptorSets = tool::writeDescriptorSet(m_capture_set,
                                                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                        0,
                                                                        &descriptorBufferInfos);

    vkUpdateDescriptorSets(SingleDevice(), 1, &writeDescriptorSets, 0, nullptr);
}

