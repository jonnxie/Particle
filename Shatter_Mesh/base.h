//
// Created by jonnxie on 2022/1/28.
//

#ifndef GAME_BASE_H
#define GAME_BASE_H

#include <glm.hpp>
#include <stb.h>

using namespace glm;

//template<uint d, typename T> bool vec_is_nan (const T & vec);
//template<uint d, typename T> bool vec_is_inf (const T & vec);
//template<uint d, typename T> bool vec_is_deg (const T & vec);

template<length_t L, typename T, qualifier Q>
bool isNan(vec<L, T, Q> const& _x){
//    return vec_is_nan<L,vec<L, T, Q>>(_x);

    for(uint i=0; i<L; ++i)
    {
        if(std::isnan(_x[i])) return true;
    }
    return false;
}

template<length_t L, typename T, qualifier Q>
bool isInf(vec<L, T, Q> const& _x){
//    return vec_is_inf<L,vec<L, T, Q>>(_x);
    for(uint i=0; i<L; ++i)
    {
        if(std::isinf(_x[i])) return true;
    }
    return false;
}

template<length_t L, typename T, qualifier Q>
bool isDeg(vec<L, T, Q> const& _x){
//    return vec_is_deg<L,vec<L, T, Q>>(_x);
    for(uint i=0; i<L; ++i)
    {
        if(std::isinf(_x[i]) || std::isnan(_x[i])) return true;
    }
    return isNan<L,T,Q>(_x);
}



#endif //GAME_BASE_H
