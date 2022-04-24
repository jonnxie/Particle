//
// Created by jonnxie on 2022/4/24.
//

#ifndef MAIN_GEOPOOL_H
#define MAIN_GEOPOOL_H

#include "Engine/Item/shatter_enum.h"
#include ItemCatalog
#include MacroCatalog
#include <vulkan/vulkan.h>
#include <glm.hpp>
#include <array>
#include <vector>
#include <iterator>
#include <iostream>

template<typename T>
class GeoPool{
public:
    GeoPool() = default;
    explicit GeoPool(size_t _reserveCount) {
        geos.resize(_reserveCount);
        reserveCount = _reserveCount;
    }
    explicit GeoPool(std::vector<T> _geos) {
        geos.assign(_geos.begin(), _geos.end());
        count = _geos.size();
        reserveCount = count * 2;
    }
    DefineUnCopy(GeoPool);
    T& operator[](size_t _index) {
        return geos[_index];
    }
public:
    virtual void push(const T& _element) {
        geos[count] = _element;
        count ++;
        if (count >= reserveCount) {
            reallocate();
        }
    }

    virtual void push(std::vector<T>& _elements) {
        count += _elements.size();
        if (count >= reserveCount) {
            reallocate();
        }
        geos.insert(geos.end(), _elements.begin(), _elements.end());
    }

    virtual void reallocate() {
        reserveCount *= 2;
        geos.resize(reserveCount);
    }

protected:
    size_t count = 0;
    size_t reserveCount = 100;
    std::vector<T> geos{};
};

#endif //MAIN_GEOPOOL_H
