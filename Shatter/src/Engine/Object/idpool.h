//
// Created by maybe on 2021/6/5.
//

#ifndef SHATTER_ENGINE_IDPOOL_H
#define SHATTER_ENGINE_IDPOOL_H

#include "Engine/Item/shatter_enum.h"
#include "shatter_math.h"
#include <vector>
#include <random>
#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>



class IDPool {
public:
    uint32_t genDID();
    uint32_t genUID();

    std::mutex m_d_mutex;
    std::mutex m_u_mutex;
    std::vector<D_id> m_did_vec;
    std::vector<U_id> m_uid_vec;
};


#endif //SHATTER_ENGINE_IDPOOL_H
