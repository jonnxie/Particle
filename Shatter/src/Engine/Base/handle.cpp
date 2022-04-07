//
// Created by jonnxie on 2022/4/7.
//

#include "handle.h"
#include MPoolCatalog

glm::vec3& Handle::getWorkCenter() const {
    return (*MPool<Target>::getPool())[m_localCoordiante]->center;
}

TargetPlane& Handle::getTargetPlane() const {
    return (*MPool<Target>::getPool())[m_localCoordiante]->plane;
}

Handle::~Handle() {
    MPool<Target>::getPool()->free(m_localCoordiante);
}
