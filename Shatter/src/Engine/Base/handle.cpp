//
// Created by jonnxie on 2022/4/7.
//

#include "handle.h"
#include MPoolCatalog
#include "Engine/App/shatterapp.h"

glm::vec3& Handle::getWorkCenter() const {
    return (*MPool<Target>::getPool())[m_localCoordiante]->center;
}

TargetPlane& Handle::getTargetPlane() const {
    return (*MPool<Target>::getPool())[m_localCoordiante]->plane;
}

Handle::~Handle() {
    MPool<Target>::getPool()->free(m_localCoordiante);
}

Handle::Handle() {
    {
        m_localCoordiante = MPool<Target>::getPool()->malloc();
        new ((Target*)(*MPool<Target>::getPool())[m_localCoordiante]) Target{SingleAPP.getWorkTargetPlane(),
                                                                             SingleAPP.getWorkTargetCenter()};
    }
}
