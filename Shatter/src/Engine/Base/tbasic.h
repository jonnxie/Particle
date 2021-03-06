//
// Created by jonnxie on 2022/1/16.
//

#ifndef GAME_TBASIC_H
#define GAME_TBASIC_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"

class TBasic : public Object{
public:
    explicit TBasic(const std::string& _files,glm::vec3 _pos,glm::vec3 _rotationAxis,float _angle,glm::vec3 _scale,int _id);
    ~TBasic(){
        delete m_model;
    }
    void constructG() override;
    void constructD() override;
    void constructC() override {};

private:
    vkglTF::Model*  m_model;
    int             m_id;
    VkDescriptorSet m_set;
};

#endif //GAME_TBASIC_H
