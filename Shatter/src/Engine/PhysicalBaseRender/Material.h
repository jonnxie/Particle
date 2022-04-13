//
// Created by jonnxie on 2022/4/13.
//

#ifndef MAIN_MATERIAL_H
#define MAIN_MATERIAL_H

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include "glm.hpp"

struct PBRMaterial {
    glm::vec3 baseColor;
    float     ambient;

    glm::vec3 emissive;
    float     roughness;
};

struct StandardMaterial : public PBRMaterial {
    float     metallic;
    float     reflectance;
};

struct ClearCoatMaterial : public StandardMaterial {
    float     clearCoat;
    float     clearCoatRoughness;
};

struct AnisotropicMaterial : public StandardMaterial {
    float     anisotropy;
};

struct ClothMaterial : public PBRMaterial {
    glm::vec3 subsurfaceColor;
    float     sheenColor;
};

#endif //MAIN_MATERIAL_H
