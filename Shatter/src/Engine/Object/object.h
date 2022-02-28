//
// Created by maybe on 2021/6/3.
//

#ifndef SHATTER_ENGINE_OBJECT_H
#define SHATTER_ENGINE_OBJECT_H

#include <vulkan/vulkan.h>
#include <vector>
#include "aabb.h"


class Object {
public:
    Object() = default;
    ~Object();
    DefineUnCopy(Object);
public:
    void draw(VkCommandBuffer _cb);
    void init();
    virtual void update(float){};
    void insertDObject(int);
    void insertGObject(int);
    void insertCObject(int);
    std::vector<int> m_dobjs{};
    std::vector<int> m_gobjs{};
    std::vector<int> m_cobjs{};
    AABB m_aabb{};
public:
    void construct();
    virtual void constructG(){};
    virtual void constructD(){};
    virtual void constructC(){};
protected:
    glm::mat4   m_world{};
    glm::mat4   m_scale{};
    glm::mat4   m_rotate{};
    glm::mat4   m_translation{};
};


#endif //SHATTER_ENGINE_OBJECT_H
