//
// Created by jonnxie on 2021/10/25.
//

#ifndef SHATTER_ENGINE_BASIC_H
#define SHATTER_ENGINE_BASIC_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"

class Basic : public Object{
public:
    explicit Basic(const std::string& _files,
                   glm::vec3 _pos,
                   glm::vec3 _rotationAxis,
                   float _angle,
                   glm::vec3 _scale,
                   int _id,
                   std::string  _pipeline = "Build",
                   std::vector<std::string>  _sets = {"Camera"});
    Basic(Basic&&) = delete;
    Basic(const Basic&) = delete;
    Basic& operator&(const Basic&) = delete;
    Basic& operator&(Basic&&) = delete;
    ~Basic(){
        delete m_model;
    }
    void constructG() override;
    void constructD() override;
    void constructC() override {};
    void update(float) override {};

private:
    vkglTF::Model*              m_model;
    std::string                 m_pipeline;
    std::vector<std::string>    m_sets;

    int                         m_id;
};


#endif //SHATTER_ENGINE_BASIC_H
