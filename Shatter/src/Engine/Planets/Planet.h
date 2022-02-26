//
// Created by AnWell on 2022/2/26.
//

#ifndef MAIN_PLANET_H
#define MAIN_PLANET_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "TerrainFace.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "TerrainFace.h"

class Planet : public Object{
public:
    Planet(uint32_t _resolution,
           glm::vec3 _pos,
           glm::vec3 _rotationAxis,
           float _angle,
           glm::vec3 _scale,
           uint32_t _id,
           std::string _pipeline = "Planet",
           std::vector<std::string> _sets = {"Camera"});
    ~Planet();
    DefineUnCopy(Planet);
    void generateMesh();
    void constructG() override;
    void constructD() override;
    void constructC() override;
    void update(float) override {};

    uint32_t       m_id;
    uint32_t  m_resolution{};
    std::vector<glm::vec3> m_points{};
    std::vector<glm::vec3> m_normals{};
    std::vector<glm::vec2> m_coordinates{};
    std::vector<TerrainFace>  m_faces{};
};


#endif //MAIN_PLANET_H
