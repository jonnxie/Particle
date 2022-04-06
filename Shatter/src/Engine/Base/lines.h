//
// Created by jonnxie on 2022/1/24.
//

#ifndef GAME_LINES_H
#define GAME_LINES_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"
#include ListenerCatalog

static int initIdVal = 0;
static std::mutex idLock;

static int mallocId()
{
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

class DLinePool : public Object{
public:
    explicit DLinePool(const std::vector<Line>& _lines,
                       int _coordinate = -1,
                       bool _updateFunc = true,
                       std::string _pipeline = "Polyline",
                       std::vector<std::string> _sets = {"Camera"});
    ~DLinePool() override;
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

class DLines : public Object{
public:
    explicit DLines(const std::vector<Line>& _lines, bool _updateFunc = true);
    ~DLines() override;
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void destroy();
    void pushLine(const Line& _line);
    void pushLines(const std::vector<Line>& _lines);

public:
    std::vector<Line>   lines;
    bool                updateFunc;
    bool                changed = true;
    int                 id;
};

class LineHandle {
public:
    LineHandle();
    ~LineHandle();
    DefineUnCopy(LineHandle);

    Line& operator[](size_t _index);
    void pushLine(const glm::vec3& _begin, const glm::vec3& _end);
    void pushLine(const Line& _line);
    void pushLines(const std::vector<std::pair<glm::vec3, glm::vec3>>& _lines);
    void pushUI();
public:
    ClassElement(m_pipeline, std::string, Pipeline);
    ClassElement(m_sets, std::vector<std::string>, Sets);
    ClassElement(m_color, glm::vec3 , Color);
    ClassElement(m_localCoordiante, int, Coordinate);
private:
    std::unique_ptr<DLinePool> m_lines{nullptr};
};

class DrawLineHandle : public Shatter::Listener{
public:
    DrawLineHandle();
    ~DrawLineHandle() override;
private:
    std::unique_ptr<LineHandle> handle;
};


#endif //GAME_LINES_H
