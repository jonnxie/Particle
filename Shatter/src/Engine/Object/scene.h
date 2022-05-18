//
// Created by jonnxie on 2022/3/21.
//

#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <vector>
#include <unordered_map>
#include "Engine/Item/shatter_macro.h"
#include <mutex>
#include <string>
#include <algorithm>

class Object;

class Scene {
public:
    Scene() = default;
    ~Scene();
    DefineUnCopy(Scene);
public:
    void save();
    Scene* loadFile(std::string& _filename);
public:
    SyncContainerOperation(compute);
    SyncContainerOperation(offscreen);
    SyncContainerOperation(default);
    SyncContainerOperation(normal);
    SyncContainerOperation(transparency);
    void addAABB(int _captureId, int _aabbIndex){
        std::lock_guard<std::mutex> lockGuard(aabb_lock);
        aabb_map[_captureId] = _aabbIndex;
    };
    void releaseAABB(int _captureId, int _aabbIndex){
        std::lock_guard<std::mutex> lockGuard(aabb_lock);
        if(aabb_map.count(_captureId) != 0)
        {
            aabb_map.erase(_captureId);
        }
    };

    void pushObject(int _id,SP(Object) _obj);

    void releaseObject(int _id);

private:
    std::mutex object_lock;
    std::unordered_map<int, SP(Object)> object_map;

    std::mutex aabb_lock;
    std::unordered_map<int, int> aabb_map{};

    std::vector<int> m_compute_id_vec{};
    bool m_compute_changed = false;

    std::vector<int> m_offscreen_id_vec{};
    bool m_offscreen_changed = false;

    std::vector<int> m_default_id_vec{};
    bool m_default_changed = false;

    std::vector<int> m_normal_id_vec{};
    bool m_normal_changed = false;

    std::vector<int> m_transparency_id_vec{};
    bool m_transparency_changed = false;
};


#endif //MAIN_SCENE_H
