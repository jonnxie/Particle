//
// Created by AnWell on 2022/3/26.
//

#ifndef MAIN_B3DM_H
#define MAIN_B3DM_H

#include <utility>
#include <vector>
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include <array>
#include <unordered_map>

struct Feature{
    FeatureType type;
    ComponentType componentType;
    int         byteOffset;
    unsigned char*       data;
};

struct FeatureTable{
    FeatureTable() = default;
    FeatureTable(std::vector<unsigned char> _buffer, int _start, int _headLength, int _binLength){
        buffer = std::move(_buffer);
        binOffset = _start + _headLength;
        binLength = _binLength;
    }
    std::vector<std::string> getKeys() {
        std::vector<std::string> keys;
        for(auto& h : header){
            keys.emplace_back(h.first);
        }
        return keys;
    }
    unsigned char* getData(const std::string& _key, int _count, ComponentType _component = ComponentType::BYTE, FeatureType _feature = FeatureType::SCALAR){
        if(header.count(_key) == 0)
        {
            return nullptr;
        }
        auto feature = header[_key];

        {
            int byteOffset = feature.byteOffset | 0;
            auto featureType = static_cast<FeatureType>(int(feature.type) | int(_feature));
            auto featureComponentType = static_cast<ComponentType>(int(feature.componentType) | int(_component));
            int stride;

            switch (featureType) {
                case FeatureType::SCALAR: {
                    stride = 1;
                    break;
                }
                case FeatureType::VEC2: {
                    stride = 2;
                    break;
                }
                case FeatureType::VEC3: {
                    stride = 3;
                    break;
                }
                case FeatureType::VEC4: {
                    stride = 4;
                    break;
                }
            }

//            unsigned char* data;
            int arrayStart = binOffset + byteOffset;
            int arrayLength = _count * stride;

            switch (featureComponentType) {
                case ComponentType::BYTE: {
                    if(arrayStart + arrayLength * 1 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
                case ComponentType::UNSIGNED_BYTE: {
                    if(arrayStart + arrayLength * 1 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
                case ComponentType::SHORT:{
                    if(arrayStart + arrayLength * 2 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
                case ComponentType::UNSIGNED_SHORT:{
                    if(arrayStart + arrayLength * 2 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
                case ComponentType::INT:{
                    if(arrayStart + arrayLength * 4 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
                case ComponentType::UNSIGNED_INT:{
                    if(arrayStart + arrayLength * 4 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
                case ComponentType::FLOAT:{
                    if(arrayStart + arrayLength * 4 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
                case ComponentType::DOUBLE:{
                    if(arrayStart + arrayLength * 8 > binOffset + binLength) {
                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
                    };
                    break;
                }
            }
            return &buffer[arrayStart];
        }
    }
    std::vector<unsigned char> buffer{};
    int binOffset;
    int binLength;
    std::unordered_map<std::string, Feature> header{};
};

struct BatchTable : FeatureTable{
    BatchTable() = default;
    BatchTable(std::vector<unsigned char> _buffer, int _batchSize, int _start, int _headLength, int _binLength)
            : FeatureTable(std::move(_buffer), _start, _headLength, _binLength) {
        batchSize = _batchSize;
    };
    unsigned char* getData(const std::string& _key, ComponentType _component, FeatureType _feature){
        return FeatureTable::getData(_key, batchSize, _component, _feature);
    }
    int batchSize{};
};


class B3DMLoaderBase{
public:
    explicit B3DMLoaderBase(const std::string &_file);
    void init();
    ~B3DMLoaderBase();
    DefineUnCopy(B3DMLoaderBase);
public:
    ClassProtectedReferenceElement(data, std::vector<unsigned char>, Data);
private:
    int  version{};
    unsigned char *binaryData{};
    int binaryData_index = 0;
    FeatureTable featureTable;
    BatchTable batchTable;
    int glbStart{};
    std::vector<char> glbBytes{};
};

class B3DMLoader {

};


#endif //MAIN_B3DM_H
