//
// Created by AnWell on 2022/2/10.
//

#ifndef PARTICLE_POINTS_H
#define PARTICLE_POINTS_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"
#include <vector>

static int initPointIdVal = 0;
static std::mutex pointIdLock;

static int mallocPointId()
{
    std::lock_guard<std::mutex> lockGuard(pointIdLock);
    return initPointIdVal++;
}

class DPoints : public Object{
public:
    explicit DPoints(size_t _initCount);
    explicit DPoints(const std::vector<Point3dColorSize>& _points);
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void update(float) override {};
    void pushPoint(const Point3dColorSize& _point);
    void pushPoint(const std::vector<Point3dColorSize>& _points);

public:
    std::vector<Point3dColorSize>   points;
    bool                            changed = true;
    int                             id;
};


#endif //PARTICLE_POINTS_H