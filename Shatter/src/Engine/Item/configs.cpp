//
// Created by jonnxie on 2021/10/4.
//
#include "precompiledhead.h"

#include "configs.h"
#include "shatter_macro.h"
#include <mutex>

//Config* Config::m_config = new Config;

static std::mutex lock;
static bool initial = false;

void Config::init()
{
    config_map["enableRayTracing"]              = 1;
    config_map["enableMeshShader"]              = 1;
    config_map["enableOffscreenDebug"]          = 0;
    config_map["enableShadowMap"]               = 0;
    config_map["enableScreenGui"]               = 1;
    config_map["enableDockSpace"]               = 1;
    config_map["enableFullScreenPersistant"]    = 1;
    config_map["enableMultipleComputeQueue"]    = 0;
    config_map["width"]                         = 1200;
    config_map["height"]                        = 900;
    config_map["LightInitCount"]                = 64;
    config_map["PointDefaultSize"]              = 4;
    config_map["SwapChainImageCount"]           = 3;
}

int Config::getConfig(const std::string& _id)
{
    auto& c = config();
    if(c.config_map.count(_id) == 0)
    {
        WARNING("no such configuration!");
        return false;
    }else{
        return c.config_map[_id];
    }
}

void Config::setConfig(const std::string& _id,int _val)
{
    auto& c = config();
    c.config_map[_id] = _val;
}

Config &Config::config() {
    static Config config;
    lock.lock();
    if(!initial)
    {
        config.init();
        initial = true;
    }
    lock.unlock();
    return config;
}







