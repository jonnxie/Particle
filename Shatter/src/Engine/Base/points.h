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
    explicit DPointPool(const std::vector<Line>& _lines,
                       int _coordinate = -1,
                       bool _updateFunc = true,
                       std::string _pipeline = "Polyline",
                       std::vector<std::string> _sets = {"Camera"});
    ~DPointPool() override;
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void pushLine(const Line& _line);
    void pushLines(const std::vector<Line>& _lines);
    void reallocated();
    ClassElement(m_pipeline, std::string, Pipeline);
    ClassElement(m_sets, std::vector<std::string>, Sets);
    ClassReferenceElement(m_lines, std::vector<Line>, Lines);
    ClassPointerElement(id, int, ID);
    ClassPointerElement(lineCount, int, LineCount);
    ClassElement(m_localCoordiante, int, Coordinate);
private:
    bool                updateFunc;
    bool                changed = true;
    size_t              poolSize;
    int                 lineResolveCount;
};

class DrawPointHandle;

class PointsHandle {
public:
    PointsHandle();
    ~PointsHandle();
    DefineUnCopy(PointsHandle);
public:
    Line& operator[](size_t _index);
    void pushLine(const glm::vec3& _begin, const glm::vec3& _end);
    void pushLine(const Line& _line);
    void pushLines(const std::vector<std::pair<glm::vec3, glm::vec3>>& _lines);
    void pushUI();
    int getLineCount();
    void loadFile(const std::string& _filename);
    void drawLine();
    void destroy() const;
    [[nodiscard]] glm::vec3& getWorkCenter() const;
    [[nodiscard]] TargetPlane& getTargetPlane() const;
public:
    ClassElementInitial(m_pipeline, std::string, Pipeline, "Polyline");
    ClassElementInitial(m_sets, std::vector<std::string>, Sets, "Camera");
    ClassElement(m_color, glm::vec3 , Color);
    ClassElement(m_localCoordiante, int, Coordinate);
    ClassPointerElement(m_listener, DrawPointHandle*, Listener);
private:
    bool appendState = true;
    std::unique_ptr<DPointPool> m_lines{nullptr};
};

class DrawPointHandle : public Shatter::Listener{
public:
    explicit DrawPointHandle(PointsHandle* _handle);
    ~DrawPointHandle() override;
private:
    PointsHandle* handle;
};

#endif //PARTICLE_POINTS_H
