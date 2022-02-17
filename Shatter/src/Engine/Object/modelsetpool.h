//
// Created by maybe on 2021/7/25.
//

#ifndef SHATTER_ENGINE_MODELSETPOOL_H
#define SHATTER_ENGINE_MODELSETPOOL_H

#include <mutex>
#include <vector>
#include <vulkan/vulkan.h>


class ModelSetPool {
public:
    static ModelSetPool& getPool();
    ModelSetPool(const ModelSetPool&) = delete;
    ModelSetPool& operator=(const ModelSetPool&) = delete;
    void init();
    void reallocate();
    void update();
    int  malloc();
    void free(int _index);
private:
    int m_model_count = 20;
    std::vector<int> m_idle;
    std::mutex m_mutex;
private:
    ModelSetPool() = default;
};

#define SingleModelSetPool ModelSetPool::getPool()

#endif //SHATTER_ENGINE_MODELSETPOOL_H
