//
// Created by jonnxie on 2022/1/24.
//

#ifndef GAME_LINES_H
#define GAME_LINES_H

#include "../Shatter_Item/shatter_item.h"
#include "../Shatter_Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "../Shatter_Object/VulkanglTFModels.h"

static int initIdVal = 0;
static std::mutex idLock;

static int mallocId()
{
    std::lock_guard<std::mutex> lockGuard(idLock);
    return initIdVal++;
}

class DLines : public Object{
public:
    explicit DLines(size_t _initCount);
    explicit DLines(const std::vector<Line>& _lines);
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void update(float) override {};
    void pushLine(const Line& _line);
    void pushLines(const std::vector<Line>& _lines);

public:
    std::vector<Line>   lines;
    bool                changed = true;
    int                 id;
};

#endif //GAME_LINES_H
