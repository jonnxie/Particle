//
// Created by jonnxie on 2021/9/28.
//

#ifndef SHATTER_ENGINE_SKYBOX_H
#define SHATTER_ENGINE_SKYBOX_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"

class Skybox : public Object{
public:
    explicit Skybox(const std::vector<std::string>& _files);
    ~Skybox(){
        m_model->release();
    }
    void constructG() override;
    void constructD() override;
    void constructC() override {};
    void update(float) override {};
private:
    vkglTF::Model* m_model;
    std::vector<GltfPoint> m_point;
    std::vector<uint32_t> m_index;
    std::vector<std::string> m_cube;
};


#endif //SHATTER_ENGINE_SKYBOX_H
