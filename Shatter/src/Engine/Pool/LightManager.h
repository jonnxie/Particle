//
// Created by jonnxie on 2022/1/13.
//

#ifndef SHATTER_ENGINE_LIGHTMANAGER_H
#define SHATTER_ENGINE_LIGHTMANAGER_H

#include "Engine/Item/shatter_enum.h"
#include UniformCatalog
#include MacroCatalog
#include <vector>
#include BPoolCatalog
#include BufferCatalog
#include SetPoolCatalog
#include DeviceCatalog

class LightManager {
public:
    static LightManager& getLight();
    DefineUnCopy(LightManager);
    PassLight& operator[](size_t _index);
    void init(size_t _size);
    void initLight();
    void addLight(const PassLight& _light);
    void reallocated(size_t _count);
private:
    std::vector<PassLight> lights;
    size_t size{};
    bool changed = true;
    size_t index{0};
private:
    LightManager() = default;
};

#define PassLightSize 32

#define SingleLM LightManager::getLight()

#endif //SHATTER_ENGINE_LIGHTMANAGER_H
