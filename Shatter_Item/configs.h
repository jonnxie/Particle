//
// Created by jonnxie on 2021/10/4.
//

#ifndef SHATTER_ENGINE_CONFIGS_H
#define SHATTER_ENGINE_CONFIGS_H

#include <iostream>
#include <unordered_map>
#include <string>


class Config{
    void init();
public:
    static Config& config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    std::unordered_map<std::string,int> config_map;
    static int getConfig(const std::string& _id);
    static void setConfig(const std::string& _id,int _val);
private:
    Config() = default;
    ~Config() = default;
};


#endif //SHATTER_ENGINE_CONFIGS_H
