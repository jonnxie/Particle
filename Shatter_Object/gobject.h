//
// Created by maybe on 2021/5/26.
//

#ifndef SHATTER_ENGINE_GOBJECT_H
#define SHATTER_ENGINE_GOBJECT_H


#include "../Shatter_Item/shatter_enum.h"
#include "../Shatter_Item/shatter_item.h"
#include <vulkan/vulkan.h>

class GObject {
public:
    virtual void genBuffer(const B_id&);
    virtual void genVBuffer(const B_id&);
    virtual void genIBuffer(const B_id&);
    GObject() = default;
    ~GObject() = default;
public:
    void* m_vertex;
    VkDeviceSize m_vertex_size;
    void* m_index;
    VkDeviceSize m_index_size;
};


#endif //SHATTER_ENGINE_GOBJECT_H
