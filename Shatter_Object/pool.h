//
// Created by AnWell on 2021/6/17.
//

#ifndef SHATTER_ENGINE_POOL_H
#define SHATTER_ENGINE_POOL_H
#include <cassert>
#include<map>
#include <mutex>

template<class ID_Type,class Value_Type>
class Pool {
public:
    virtual void init() = 0;

    virtual void release() = 0;

    Value_Type Get(const ID_Type& _id){
        return m_map[_id];
    }

    Value_Type operator[](const ID_Type& _id){
        return m_map[_id];
    }

    bool Set(const ID_Type& _id, Value_Type _value){
        std::lock_guard<std::mutex> guard(m_mutex);
        for(auto & i : m_map){
            if(i.first == _id){
                assert(0);
                return false;
            }
        }
        m_map[_id] = _value;
        return true;
    }

    std::map<ID_Type,Value_Type> m_map;
    std::mutex m_mutex;
};


#endif //SHATTER_ENGINE_POOL_H
