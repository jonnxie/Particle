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
#include ListenerCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"
#include <vector>
#include HandleCatalog

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
    void pushPoint(const Point3dColorSize& _point);
    void pushPoint(const std::vector<Point3dColorSize>& _points);

public:
    std::vector<Point3dColorSize>   points;
    bool                            changed = true;
    int                             id;
};

class DPointPool : public Object{
public:
    explicit DPointPool(const std::vector<Point3dColorSize>& _points,
                       int _coordinate = -1,
                       bool _updateFunc = true,
                       std::string _pipeline = "Point",
                       std::vector<std::string> _sets = {"Camera", "ViewPort"});
    ~DPointPool() override;
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void pushPoint(const Point3dColorSize& _point);
    void pushPoints(const std::vector<Point3dColorSize>& _points);
    void reallocated();
    ClassElement(m_pipeline, std::string, Pipeline);
    ClassElement(m_sets, std::vector<std::string>, Sets);
    ClassElement(m_localCoordiante, int, Coordinate);
    ClassReferenceElement(m_points, std::vector<Point3dColorSize>, Points);
    ClassPointerElement(id, int, ID);
    ClassPointerElement(pointCount, int, PointCount);
private:
    bool                updateFunc;
    bool                changed = true;
    size_t              poolSize;
    int                 pointResolveCount;
};

class DrawPointHandle;

class PointsHandle :public Handle {
public:
    PointsHandle();
    ~PointsHandle() override;
    DefineUnCopy(PointsHandle);
public:
    Point3dColorSize& operator[](size_t _index);
    void pushPoint(const glm::vec3& _point, bool _textured = false);
    void pushPoint(const Point3dColorSize& _point, bool _textured = false);
    void pushPoints(const std::vector<glm::vec3>& _points);
    void pushUI() override;
    int  getPointCount();
    void loadFile(const std::string& _filename) override;
    void drawPoint();
    void drawPointTex();
    void destroy() const override;
public:
    ClassPointerElement(m_listener, DrawPointHandle*, Listener);
    ClassProtectedElement(m_size, float , Size);
private:
    bool appendState = true;
    bool appendTexState = true;
    std::unique_ptr<DPointPool> m_points{nullptr};
    std::unique_ptr<DPointPool> m_pointsTex{nullptr};
};

class DrawPointHandle : public Shatter::Listener{
public:
    explicit DrawPointHandle(PointsHandle* _handle);
    ~DrawPointHandle() override;

protected:
    friend PointsHandle;
    bool textured = false;
private:
    PointsHandle* handle;
};

#endif //PARTICLE_POINTS_H
