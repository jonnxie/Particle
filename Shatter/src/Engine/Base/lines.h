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

static int initIdVal = 0;
static std::mutex idLock;

static int mallocId()
{
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

class DLinePool : public Object{
public:
    explicit DLinePool(const std::vector<Line>& _lines, bool _updateFunc = true);
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void update(float) override {};
    void pushLine(const Line& _line);
    void pushLines(const std::vector<Line>& _lines);
    void reallocated();

public:
    std::vector<Line>   lines;
    bool                updateFunc;
    bool                changed = true;
    int                 lineCount;
    int                 lineResolveCount;
    int                 id;
};

class DLines : public Object{
public:
    explicit DLines(const std::vector<Line>& _lines, bool _updateFunc = true);
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void update(float) override {};
    void pushLine(const Line& _line);
    void pushLines(const std::vector<Line>& _lines);

public:
    std::vector<Line>   lines;
    bool                updateFunc;
    bool                changed = true;
    int                 id;
};

#endif //GAME_LINES_H
