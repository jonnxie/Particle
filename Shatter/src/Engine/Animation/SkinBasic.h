//
// Created by jonnxie on 2022/4/8.
//

#ifndef MAIN_SKINBASIC_H
#define MAIN_SKINBASIC_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"

class SkinBasic : public Object{
public:
    explicit SkinBasic(const std::string& _files,
                   glm::vec3 _pos,
                   glm::vec3 _rotationAxis,
                   float _angle,
                   glm::vec3 _scale,
                   int _id,
                   std::string  _pipeline = "GSkin",
                   std::vector<std::string>  _sets = {"Camera", "Skin"});
    DefineUnCopy(SkinBasic);
    ~SkinBasic() {
        delete m_model;
    }
    void constructG() override;
    void constructD() override;
    void constructC() override {};

    ClassElement(m_animation_index, int, AnimationIndex);
    ClassElementInitial(m_update, bool, Update, true);
    ClassElement(m_localTime, float, LocalTime);
private:
    vkglTF::Model*              m_model;
    std::string                 m_pipeline;
    std::vector<std::string>    m_sets;

    int                         m_id;
};


#endif //MAIN_SKINBASIC_H
