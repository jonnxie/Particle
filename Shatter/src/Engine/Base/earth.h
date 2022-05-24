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
          double _radius = 5.0f,
          std::string  _pipeline = "EarthTex",
          std::string _textureId = "default");
    ~Earth() override;
    DefineUnCopy(Earth);
    void constructG() override;
    void constructD() override;
public:
    uint32_t                    m_id;
    uint32_t                    m_longitudeResolution{};
    uint32_t                    m_latitudeResolution{};
    std::vector<glm::dvec2>     m_vertices;
    std::string                 m_pipeline;
    std::string                 m_texture;
    double                      m_radius{};
};


#endif //MAIN_EARTH_H
