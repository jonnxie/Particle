//
// Created by jonnxie on 2022/5/23.
//

#ifndef MAIN_EARTH_H
#define MAIN_EARTH_H

#include "Engine/Item/shatter_enum.h"
#include ItemCatalog
#include MacroCatalog
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog

class Earth : public Object{
public:
    Earth(glm::vec3 _pos,
          uint32_t _longitudeResolution,
          uint32_t _latitudeResolution,
          float _radius = 5.0f,
          std::string  _pipeline = "Earth_Face",
          std::vector<std::string>  _sets = {"Camera", "Planet"},
          DrawObjectType _type = DrawObjectType::Normal);
    ~Earth();
    DefineUnCopy(Earth);
    void constructG() override;
    void constructD() override;
    void constructC() override;
public:
    uint32_t            m_id;
    uint32_t            m_longitudeResolution{};
    uint32_t            m_latitudeResolution{};
    float               m_radius{};
};


#endif //MAIN_EARTH_H
