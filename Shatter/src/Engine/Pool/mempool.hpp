//
// Created by jonnxie on 2021/12/2.
//

#ifndef SHATTER_ENGINE_MEMPOOL_HPP
#define SHATTER_ENGINE_MEMPOOL_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "glm.hpp"
#include <algorithm>
#include <condition_variable>
//#include <semaphore.h>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include ConfigCatalog
#include MacroCatalog
#include MathCatalog

namespace animation{
    class Joint;
    class Material;
    using Avec2 = glm::vec2;
    using Avec3 = glm::vec3;
    using Avec4 = glm::vec4;
    using Amat4 = glm::mat4;
}

using namespace animation;

template<class ObjectType>
class MemPool {
public:
    static MemPool<ObjectType>& getPool(){
        if constexpr (std::is_same_v<ObjectType,animation::Joint>)
        {
            static MemPool<animation::Joint> val(Config::getConfig("JointInitialCount"));
            return val;
        } else if constexpr (std::is_same_v<ObjectType,animation::Material>) {
            static MemPool<animation::Material> val(Config::getConfig("MaterialInitialCount"));
            return val;
        }
    }

public:
    MemPool() = delete;
    explicit MemPool(int _num):m_count(_num){
        m_ptr = new ObjectType[m_count];
        for(auto i = 0; i < m_count; i++)
        {
            m_idle_map[i] = IDLE;
        }
    }
public:
    ObjectType operator[](int _index) {
        ObjectType val;
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        if(m_idle_map[_index])
        {
            WARNING(index is idle);
        }
        val = m_ptr[_index];
        return val;
    }

    void set(int _index,const ObjectType& _val){
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        m_idle_map[_index] = ACTIVE;
        m_ptr[_index] = std::move(_val);
    }

    int malloc(){
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        for(auto& [index,val] : m_idle_map){
            if(val == IDLE){
                m_idle_map[index] = ACTIVE;
                return index;
            }
        }
        reallocated();
        return malloc();
    }

    void free(int _index) {
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        m_idle_map[_index] = IDLE;
    }

    void reallocated() {
        std::lock_guard<std::mutex> guard_mutex(m_mutex);
        auto* new_ptr = new ObjectType[m_count*2];
        int old_num = m_count;
        memcpy(new_ptr,m_ptr,m_count * sizeof(ObjectType));
        delete[] m_ptr;
        m_ptr = new_ptr;
        m_count *= 2;
        int num = m_count - old_num;
        for(auto i = 0; i < num; i++){
            m_idle_map[old_num++] = IDLE;
        }
    }


private:
    ObjectType* m_ptr;
    int m_count;
    std::unordered_map<int,bool> m_idle_map;
    std::mutex m_mutex;
    bool m_destroy = false;
};

template<class ObjectType>
class BigPool{
public:
    explicit BigPool(int _num):m_count(_num){
        m_capacity = m_count * sizeof(ObjectType);
        m_ptr = new ObjectType[_num];
    }
    ~BigPool()
    {
        release();
    }
    DefineUnCopy(BigPool);
    void get(int _index,ObjectType& _val){
        _val = m_ptr[_index];
    }
    void set(int _index,const ObjectType& _val)
    {
        m_ptr[_index] = _val;
    }
    void interpolateA(int _left,int _right,float _alpha,int _dest)
    {
        ObjectType left,right,dest;
        get(_left,left);
        get(_right,right);
        interpolate(&left, &right, _alpha, &dest);
        set(_dest,dest);
    }
    void copy(int _des,int _src)
    {
        memcpy(&m_ptr[_des], &m_ptr[_src], sizeof(ObjectType));
    }
    int malloc(int _count){
        assert(_count > 0);
        std::lock_guard<std::mutex> lockGuard(m_mutex);
        int val = m_cursor;
        m_cursor += _count;
        while(m_cursor >= m_count){
            reallocated();
        }
        return val;
    }
    void reallocated()
    {
        auto* new_ptr = new ObjectType[m_count*2];
        memcpy(new_ptr,m_ptr,m_count * sizeof(ObjectType));
        delete[] m_ptr;
        m_ptr = new_ptr;
        m_count *= 2;
    }
    void release()
    {
        delete[] m_ptr;
    }

private:
    ObjectType* m_ptr;
    int m_count;
    int m_cursor = 0;
    uint64_t m_capacity;
    std::mutex m_mutex;
};

#define Avec2Pool _animation->m_vec2Pool

#define Avec3Pool _animation->m_vec3Pool

#define Avec4Pool _animation->m_vec4Pool

#define Amat4Pool _animation->m_mat4Pool

#endif //SHATTER_ENGINE_MEMPOOL_HPP
