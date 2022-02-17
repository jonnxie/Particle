//
// Created by maybe on 2021/6/5.
//
#include "precompiledhead.h"

#include "idpool.h"
#include <algorithm>
#include <mutex>

uint32_t IDPool::genDID(){
    uint32_t val = uint32_t(genRandom(100));
    std::lock_guard<std::mutex> guard(m_d_mutex);
    if(m_did_vec.size() == 0){
        m_did_vec.push_back(val);
        return val;
    }
    while(std::find(m_did_vec.begin(),m_did_vec.end(),val) != m_did_vec.begin()){
        val = uint32_t(genRandom(100));
    }
    m_did_vec.push_back(val);
    return val;
}


uint32_t IDPool::genUID(){
    uint32_t val = uint32_t(genRandom(100));
    std::lock_guard<std::mutex> guard(m_u_mutex);
    if(m_uid_vec.size() == 0){
        m_uid_vec.push_back(val);
        return val;
    }
    while(std::find(m_uid_vec.begin(),m_uid_vec.end(),val) != m_uid_vec.begin()){
        val = uint32_t(genRandom(100));
    }
    m_uid_vec.push_back(val);
    return val;
}


