//
// Created by AnWell on 2022/2/26.
//

#ifndef MAIN_TERRAINFACE_H
#define MAIN_TERRAINFACE_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog

class TerrainFace {
public:
    TerrainFace(uint32_t _resolution,
                glm::vec3 _localUp):
                m_resolution(_resolution),
                m_localUp(_localUp){
        m_axisA = glm::vec3(_localUp.y, _localUp.z, _localUp.x);
        m_axisB = glm::cross(_localUp, m_axisA);
    }
    void generateMesh(std::vector<glm::vec3>& _points,
                       std::vector<glm::vec2>& _coordinates,
                       std::vector<uint32_t>& _indices)
    {
        _points.resize(m_resolution * m_resolution);
        _indices.resize((m_resolution - 1) * (m_resolution - 1) * 6);
        _coordinates.resize(_points.size());
        int triIndex = 0;
        for (int y = 0; y < m_resolution; y++)
        {
            for (int x = 0; x < m_resolution; x++) {
                int i = x + y * m_resolution;
                glm::vec2 percent = glm::vec2(x / (m_resolution - 1), y / (m_resolution - 1));
                glm::vec3 cube_point =
                        m_localUp + (percent.x - 0.5f) * 2.0f * m_axisA + (percent.y - 0.5f) * 2.0f * m_axisB;
                glm::vec3 sphere_point = glm::normalize(cube_point) * 5.0f;
                _points[i] = sphere_point;

                if (x != m_resolution - 1 && y != m_resolution - 1) {
                    _indices[triIndex] = i;
                    _indices[triIndex + 1] = i + m_resolution + 1;
                    _indices[triIndex + 2] = i + m_resolution;

                    _indices[triIndex + 3] = i;
                    _indices[triIndex + 4] = i + 1;
                    _indices[triIndex + 5] = i + m_resolution + 1;
                    triIndex += 6;
                }
            }
        }

    }
public:
    uint32_t m_resolution;
    glm::vec3 m_localUp;
    glm::vec3 m_axisA;
    glm::vec3 m_axisB;
};


#endif //MAIN_TERRAINFACE_H
