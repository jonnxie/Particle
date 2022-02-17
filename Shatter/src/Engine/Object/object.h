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
    void draw(VkCommandBuffer _cb);
    void init();
    virtual void update(float){};

    ~Object();
    void insertDObject(int);
    void insertGObject(int);
    void insertCObject(int);
    void regenerateCommandBuffer();
    std::vector<int> m_dobjs;
    std::vector<int> m_gobjs;
    std::vector<int> m_cobjs;
    AABB m_aabb;
public:
    void construct();
    virtual void constructG(){};
    virtual void constructD(){};
    virtual void constructC(){};
};


#endif //SHATTER_ENGINE_OBJECT_H
