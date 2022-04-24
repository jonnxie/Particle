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
#include "Engine/Base/GeoPool.h"

class SkinBasic : public Object {
public:
    explicit SkinBasic(const std::string& _files,
                       glm::vec3 _pos,
                       glm::vec3 _rotationAxis,
                       float _angle,
                       glm::vec3 _scale,
                       std::string  _pipeline = "GSkin",
                       std::vector<std::string>  _sets = {"Camera", "Skin"},
                       DrawObjectType _type = DrawObjectType::Normal);
    DefineUnCopy(SkinBasic);
    ~SkinBasic() override {
        delete m_model;
    }
    void constructG() override;
    void constructD() override;
    void constructC() override {};
    std::string getName() override {
        return "Skin";
    }
    ClassElement(m_animation_index, int, AnimationIndex);
    ClassElementInitial(m_update, bool, Update, true);
    ClassElement(m_localTime, float, LocalTime);
    ClassPointerElement(m_model, vkglTF::Model*, Model);
    ClassReferenceElement(m_pipeline, std::string, Pipeline);
    ClassReferenceElement(m_sets, std::vector<std::string>, Sets);
private:
    int                         m_id;
};

class SkinBasicInstance : public Object, GeoPool<glm::vec3>{
public:
    explicit SkinBasicInstance(const std::string& _files,
                               const std::vector<glm::vec3>& _instances,
                               glm::vec3 _rotationAxis,
                               float _angle,
                               glm::vec3 _scale,
                               std::string  _pipeline = "GSkinInstance",
                               std::vector<std::string>  _sets = {"Camera"},
                               DrawObjectType _type = DrawObjectType::Normal);

    SkinBasicInstance(SkinBasic* _skin,const std::vector<glm::vec3>& _instances);

    ~SkinBasicInstance() override {
        TaskPool::popUpdateTask(tool::combine("SkinBasicInstance", m_id));
        delete m_model;
    }

    void constructG() override;
    void constructD() override;
    void constructC() override {};

    void push(const glm::vec3& _element) override {
        GeoPool<glm::vec3>::push(_element);
        SingleRender.normalChanged = true;
    };

    void push(std::vector<glm::vec3>& _elements) override {
        GeoPool<glm::vec3>::push(_elements);
        SingleRender.normalChanged = true;
    };

    void reallocate() override;
    ClassElementInitial(m_animation_index, int, AnimationIndex, 0);
    ClassElementInitial(m_update, bool, Update, true);
    ClassElement(m_localTime, float, LocalTime);
private:
    vkglTF::Model*              m_model{};
    std::string                 m_pipeline{};
    std::vector<std::string>    m_sets{};

    int                         m_id;
};





#endif //MAIN_SKINBASIC_H
