//
// Created by jonnxie on 2022/3/28.
//

#ifndef MAIN_FEATURETABLE_H
#define MAIN_FEATURETABLE_H

#include "json.hpp"

using namespace nlohmann;

struct FeatureTable{
    FeatureTable() = default;
    FeatureTable(std::vector<unsigned char> _buffer, int _start, int _headLength, int _binLength){
        buffer = std::move(_buffer);
        binOffset = _start + _headLength;
        binLength = _binLength;
        header = json::parse(buffer);
    }
    std::vector<std::string> getKeys() {
        std::vector<std::string> keys;
        for(json::iterator it = header.begin(); it != header.end(); it++)
        {
            keys.emplace_back(it.key());
        }
        return keys;
    }
    json::reference getData(const std::string& _key)
    {
        if(header.count(_key) == 0)
        {
            WARNING("No Such Json Key!");
        }
        return header[_key];
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
