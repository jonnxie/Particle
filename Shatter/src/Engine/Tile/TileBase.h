//
// Created by jonnxie on 2022/4/19.
//

#ifndef MAIN_TILEBASE_H
#define MAIN_TILEBASE_H

#include <utility>

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_macro.h"

class TileBase {
public:
    explicit TileBase(std::string  _url):m_url(std::move(_url)) {

    };
public:
    std::string m_url;
};


#endif //MAIN_TILEBASE_H
