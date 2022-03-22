//
// Created by jonnxie on 2022/3/21.
//

#include "scene.h"
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
