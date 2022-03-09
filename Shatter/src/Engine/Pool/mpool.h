//
// Created by maybe on 2021/7/8.
//

#ifndef SHATTER_ENGINE_MPOOL_H
#define SHATTER_ENGINE_MPOOL_H

#include <iostream>
#include <mutex>
#include <vector>
#include <vulkan/vulkan.h>
#include "glm.hpp"
#include <algorithm>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_macro.h"

class Line3d;
class Plane3d;
class GObject;
class DObject;
class CObject;
struct ThreadObject;
struct AABB;

template<class Object_Type>
class MPool {
public:
    explicit MPool(int _count ):m_count(_count) {
        init();
    }

    DefineUnCopy(MPool);
public:

    void malloc(int _count, std::vector<int> &_out)
    {
        std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
        while(_count > m_idle_list.size())
        {
            reallocated();
        }
        _out.assign(m_idle_list.begin(), m_idle_list.begin() + _count);
        m_idle_list.erase(m_idle_list.begin(), m_idle_list.begin() + _count);
    }

    void free(int _count, const std::vector<int> &_in) {
        std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
        m_idle_list.insert(m_idle_list.end(),_in.begin(), _in.begin() + _count);
    }


    void free(const std::vector<int> &_in) {
        std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
        m_idle_list.insert(m_idle_list.end(),_in.begin(), _in.end());
    }

    int malloc() {
        std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
        int val;
        if(m_idle_list.empty())
        {
            reallocated();
        }
        val = m_idle_list.front();
        m_idle_list.erase(m_idle_list.begin());
        return val;
    }

    void free(int _index) {
        std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
        m_idle_list.insert(m_idle_list.end(),_index);
    }


public:
    void init() {
        m_ptr = new Object_Type[m_count];
        m_idle_list.resize(m_count);
        for(auto i = 0 ; i < m_count ; i++){
            m_idle_list[i] = i;
        }
    }

    void release() {
        delete m_ptr;
        m_ptr = nullptr;
        if constexpr (std::is_same_v<Object_Type,Line3d>)
        {
            delete m_line3d_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,Plane3d>)
        {
            delete m_plane3d_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,DObject>)
        {
            delete m_dobject_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,GObject>)
        {
            delete m_gobject_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,VkDescriptorSet>)
        {
            delete m_set_pool;
        }
        else if constexpr(std::is_same_v<Object_Type,CObject>)
        {
            delete m_cobject_pool;
        }
        else if constexpr(std::is_same_v<Object_Type,ObjectBox>)
        {
            delete m_object_pool;
        }
        else if constexpr(std::is_same_v<Object_Type,AABB>)
        {
            delete m_aabb_pool;
        }
    }

    Object_Type* operator[](int _index) {
        std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
        std::lock_guard<std::mutex> guard_mutex2(m_mutex);
        auto index = std::find(m_idle_list.begin(),m_idle_list.end(),_index);
        if( index != m_idle_list.end()){
//            throwError("cant use idle object");
            m_idle_list.erase(index);
        }
        if (m_ptr == nullptr)
        {
            std::cout << "pointer free" << std::endl;
            return nullptr;
        }else{
            return &m_ptr[_index];
        }
    }

    Object_Type *operator()() {
        std::lock_guard<std::mutex> guard_mutex(m_mutex);

        return m_ptr;
    }

    void reallocated() {
        std::lock_guard<std::mutex> guard_mutex(m_mutex);
        Object_Type* new_ptr = new Object_Type[m_count*2];
        int old_num = m_count;
        memcpy(new_ptr, m_ptr, m_count * sizeof(Object_Type));
        delete m_ptr;
        m_ptr = new_ptr;
        m_count *= 2;
        int num = m_count - old_num;
        for(auto i = 0; i < num; i++){
            m_idle_list.emplace_back(old_num++);
        }
    }

    static MPool<Object_Type>* getPool() {
        if constexpr (std::is_same_v<Object_Type,Line3d>)
        {
            return m_line3d_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,Plane3d>)
        {
            return m_plane3d_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,DObject>)
        {
            return m_dobject_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,GObject>)
        {
            return m_gobject_pool;
        }
        else if constexpr (std::is_same_v<Object_Type,VkDescriptorSet>)
        {
            return m_set_pool;
        }
        else if constexpr(std::is_same_v<Object_Type,CObject>)
        {
            return m_cobject_pool;
        }
        else if constexpr(std::is_same_v<Object_Type,ObjectBox>)
        {
            return m_object_pool;
        }
        else if constexpr(std::is_same_v<Object_Type,AABB>)
        {
            return m_aabb_pool;
        }
        return nullptr;
    }

    static void initMPool();

public:
    Object_Type* m_ptr;
    int m_count;
    std::vector<int> m_idle_list;
    std::mutex m_mutex;
    std::mutex m_idle_mutex;
private:
    static MPool<Line3d>* m_line3d_pool;
    static MPool<Plane3d>* m_plane3d_pool;
    static MPool<DObject>* m_dobject_pool;
    static MPool<GObject>* m_gobject_pool;
    static MPool<CObject>* m_cobject_pool;
    static MPool<VkDescriptorSet>* m_set_pool; // default set Pool used for model matrix
    static MPool<glm::mat4>* m_model_matrix_pool;
    static MPool<ObjectBox>* m_object_pool;
    static MPool<AABB>* m_aabb_pool;
//    ObjectBox
};

#define SingleDPool MPool<DObject>::getPool()
#define SingleCPool MPool<CObject>::getPool()
#define SingleGPool MPool<GObject>::getPool()
#define SingleAABBPool MPool<AABB>::getPool()

#endif //SHATTER_ENGINE_MPOOL_H
