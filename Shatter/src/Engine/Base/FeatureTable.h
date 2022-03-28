//
// Created by jonnxie on 2022/3/28.
//

#ifndef MAIN_FEATURETABLE_H
#define MAIN_FEATURETABLE_H

#include "json.hpp"
#include "Engine/Item/shatter_enum.h"
using namespace nlohmann;

struct FeatureTable{
    FeatureTable() = default;
    FeatureTable(std::vector<unsigned char> _buffer, int _start, int _headLength, int _binLength){
        buffer = std::move(_buffer);
        binOffset = _start + _headLength;
        binLength = _binLength;
        std::vector<unsigned char> head_buffer(_headLength - _start);
        head_buffer.assign(buffer.begin() + _start, buffer.begin() + _headLength);
        header = json::parse(head_buffer);
    }
    std::vector<std::string> getKeys() {
        std::vector<std::string> keys;
        for(json::iterator it = header.begin(); it != header.end(); it++)
        {
            keys.emplace_back(it.key());
        }
        return keys;
    }

    template<class ResultType = json::reference,
            ComponentType componentType = ComponentType::None,
            FeatureType featureType = FeatureType::None>
    ResultType getData(const std::string& _key, int length = 1)
    {
        if(header.count(_key) == 0)
        {
            WARNING("No Such Json Key!");
        }
        auto feature = header[_key];
        if constexpr (std::is_same_v<ResultType, json::reference>)
        {
            return header[_key];
        }else if constexpr(std::is_same_v<ResultType, std::vector<glm::vec3>>){
            int byteOffset = feature["byteOffset"];
            constexpr int stride = static_cast<const int>(featureType);
            int arrayStart = binOffset + byteOffset;
            int arrayLength = length * stride;
            std::vector<glm::vec3> result(length);
            auto dataEnd = arrayStart + arrayLength * 4;
            if ( dataEnd > binOffset + binLength ) {
                throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
            }
            memcpy(result.data(), &buffer[arrayStart], arrayLength);
            return result;
        }
    };

    std::vector<unsigned char> buffer{};
    int binOffset;
    int binLength;
    json header;
};

struct BatchTable : FeatureTable{
    BatchTable() = default;
    BatchTable(std::vector<unsigned char> _buffer, int _batchSize, int _start, int _headLength, int _binLength)
            : FeatureTable(std::move(_buffer), _start, _headLength, _binLength) {
        batchSize = _batchSize;
    };
    json::reference getData(const std::string& _key, ComponentType _component, FeatureType _feature){
        return FeatureTable::getData(_key);
    }
    int batchSize{};
};

#endif //MAIN_FEATURETABLE_H
