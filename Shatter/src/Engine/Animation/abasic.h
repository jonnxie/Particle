//
// Created by jonnxie on 2022/3/2.
//

#ifndef MAIN_ABASIC_H
#define MAIN_ABASIC_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"

class ABasic : public Object{
public:
    explicit ABasic(const std::string& _files,
                   glm::vec3 _pos,
                   glm::vec3 _rotationAxis,
                   float _angle,
                   glm::vec3 _scale,
                   int _id,
                   std::string  _pipeline = "ABasic",
                   std::vector<std::string>  _sets = {"Camera", "Planet"},
                   DrawObjectType _type = DrawObjectType::Normal);

    DefineUnCopy(ABasic);
    ~ABasic(){
        delete m_model;
    }
    void constructG() override;
    void constructD() override;
    void constructC() override {};
    void update(float) override {};

    ClassElement(m_animation_index, int, AnimationIndex);
//    ClassElement(m_draw_type, DrawObjectType, DrawType);
    ClassPointerElement(m_model, vkglTF::Model*, Model);
private:
    std::string                 m_pipeline;
    std::vector<std::string>    m_sets;
    int                         m_id;
};



#endif //MAIN_ABASIC_H
