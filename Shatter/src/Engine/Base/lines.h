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
#include HandleCatalog


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
    explicit DLines(const std::vector<Line>& _lines, bool _updateFunc = true, int _modelIndex = -1, bool _capture = true);
    ~DLines() override;
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void hide() override;
    void show() override;
    SP(Object) copy(const glm::vec3& _pos) override;
    void move(const glm::vec3& _movement) override;
    void update(const glm::vec3& _movement) override;
    std::string getName() override {
        return "DLines";
    }

    void destroy();
    void pushLine(const Line& _line);
    void pushLines(const std::vector<Line>& _lines);

public:
    std::vector<Line>   lines;
    int                 m_modelIndex;
    bool                updateFunc;
    bool                capture;
    bool                changed = true;
    bool                showed = true;
    int                 id;
};

class AABBLine {
public:
    explicit AABBLine(int _aabbIndex, int _captureIndex, glm::vec3 _color);
    ~AABBLine();
    void show() const;
    void hide() const;
private:
    ClassPointerElement(aabbIndex, int, Index);
    ClassPointerElement(captureIndex, int, CaptureIndex);
    std::unique_ptr<DLines> line;
};

class AABBLine;
class CaptureObjectListener : public Shatter::Listener{
public:
    CaptureObjectListener();
    ~CaptureObjectListener() override;
    void pushUI();
private:
    std::shared_ptr<CaptureObject> captureObject{nullptr};
    int preCaptureId = 0;
};

class DrawLineHandle;

class LineHandle : public Handle{
public:
    LineHandle();
    ~LineHandle() override;
    DefineUnCopy(LineHandle);
public:
    Line& operator[](size_t _index);
    void pushLine(const glm::vec3& _begin, const glm::vec3& _end);
    void pushLine(const Line& _line);
    void pushLines(const std::vector<std::pair<glm::vec3, glm::vec3>>& _lines);
    void pushUI() override;
    int getLineCount();
    void loadFile(const std::string& _filename) override;
    void drawLine();
    void destroy() const override;
public:
    ClassPointerElement(m_listener, DrawLineHandle*, Listener);
private:
    bool appendState = true;
    std::unique_ptr<DLinePool> m_lines{nullptr};
};

class DrawLineHandle : public Shatter::Listener{
public:
    explicit DrawLineHandle(LineHandle* _handle);
    ~DrawLineHandle() override;
private:
    LineHandle* handle;
};


#endif //GAME_LINES_H
