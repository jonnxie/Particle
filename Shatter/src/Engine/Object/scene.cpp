//
// Created by jonnxie on 2022/3/21.
//
#include "precompiledhead.h"
#include "scene.h"
#include <utility>
#include "Engine/Renderer/renderer.h"

Scene::~Scene() {
    for (int i : m_compute_id_vec) {
        SingleRender.releaseComputeObject(i);
    }

    for (int i : m_offscreen_id_vec) {
        SingleRender.releaseObject(i, DrawObjectType::OffScreen);
    }

    for (int i : m_default_id_vec) {
        SingleRender.releaseObject(i, DrawObjectType::Default);
    }

    for (int i : m_normal_id_vec) {
        SingleRender.releaseObject(i, DrawObjectType::Normal);
    }

    for (int i : m_transparency_id_vec) {
        SingleRender.releaseObject(i, DrawObjectType::Transparency);
    }

    for (auto i : aabb_map) {
        SingleRender.releaseObject(i.first, DrawObjectType::AABB);
    }

}

void Scene::save() {

}

Scene *Scene::loadFile(std::string &_filename) {
    return nullptr;
}

void Scene::pushObject(int _id, std::shared_ptr<Object> _obj) {
    GuardMutex(object_lock);
    if (object_map.count(_id) == 0) {
        std::cout << "Event: Insert Object, Id: " << _id << std::endl;
    } else {
        object_map[_id] = std::move(_obj);
    }
}

void Scene::releaseObject(int _id) {
    GuardMutex(object_lock);
    if (object_map.count(_id) == 0) {
        std::cout << "Event: Release Object, Id: " << _id << std::endl;
    } else {
        object_map.erase(_id);
    }
}
