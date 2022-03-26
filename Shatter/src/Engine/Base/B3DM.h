//
// Created by AnWell on 2022/3/26.
//

#ifndef MAIN_B3DM_H
#define MAIN_B3DM_H

#include <vector>
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"

class B3DMLoaderBase{
public:
    explicit B3DMLoaderBase(const std::string &_file);
    ~B3DMLoaderBase() = default;
    DefineUnCopy(B3DMLoaderBase);
public:
    ClassProtectedReferenceElement(data, std::vector<unsigned char>, Data);
private:
    char magic[4]{};
    int  version{};
    int  byteLength{};
    int  featureTableJSONByteLength{};
    int  featureTableBinaryByteLength{};
    int  batchTableJSONByteLength{};
    int  batchTableBinaryByteLength{};
};

class B3DMLoader {

};


#endif //MAIN_B3DM_H
