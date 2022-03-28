//
// Created by AnWell on 2022/3/26.
//

#ifndef MAIN_B3DM_H
#define MAIN_B3DM_H

#include <utility>
#include <vector>
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include <array>
#include <unordered_map>
#include "Engine/Object/VulkanglTFModels.h"
#include "Engine/Object/object.h"
#include "FeatureTable.h"

using namespace nlohmann;

class B3DMLoaderBase{
public:
    explicit B3DMLoaderBase(const std::string &_file);
    ~B3DMLoaderBase();
    DefineUnCopy(B3DMLoaderBase);

private:
    void init();

protected:
    std::vector<unsigned char> glbBytes{};
private:
    int  version{};
    unsigned char *binaryData{};
    int binaryData_index = 0;
    FeatureTable featureTable;
    BatchTable batchTable;
    int glbStart{};
};

class B3DMBasic;

class B3DMLoader : public B3DMLoaderBase{
public:
    explicit B3DMLoader(const std::string &_file);
    ~B3DMLoader(){
        delete m_model;
    };
    DefineUnCopy(B3DMLoader);
public:
    void init(const std::string& _file);
    void loadB3DMFile(const std::string& _file);
    ClassElementInitial(m_pos, glm::vec3, Pos, );
private:
    glm::vec3 m_rotationAxis{glm::vec3(1.0f,0.0f,0.0f)};
public:
    void setRotationAxis(const glm::vec3 &_in) { m_rotationAxis = _in; }
    glm::vec3 getRotationAxis() { return m_rotationAxis; }
    ClassElementInitial(m_angle, float, Angle, );
    ClassElementInitial(m_scale, glm::vec3, Scale, 1.0f);
    ClassElementInitial(m_pipeline, std::string, Pipeline, "AGBasic");
private:
    std::vector<std::string> m_sets{"Camera", "Planet"};
public:
    void setSets(const std::vector<std::string> &_in) { m_sets = _in; }
    std::vector<std::string> getSets() { return m_sets; }
    ClassElementInitial(m_drawType, DrawObjectType, DrawType, DrawObjectType::Default);
private:
    vkglTF::Model*              m_model{nullptr};
    std::unique_ptr<B3DMBasic>  m_basic{nullptr};
};

class B3DMBasic : public Object{
public:
    explicit B3DMBasic(vkglTF::Model* _model,
                       glm::vec3 _pos,
                       glm::vec3 _rotationAxis,
                       float _angle,
                       glm::vec3 _scale,
                       int _id,
                       std::string   _pipeline = "AGBasic",
                       std::vector<std::string>   _sets = {"Camera", "Planet"});
    ~B3DMBasic() override;
    void constructG() override {};
    void constructD() override;
    void constructC() override {};

private:
    vkglTF::Model*              m_model{};
    std::string                 m_pipeline;
    std::vector<std::string>    m_sets;

    int                         m_id;
};


//    unsigned char* getData(const std::string& _key, int _count, ComponentType _component = ComponentType::BYTE, FeatureType _feature = FeatureType::SCALAR){
//        if(header.count(_key) == 0)
//        {
//            return nullptr;
//        }
//        auto feature = header[_key];
//
//        {
//            int byteOffset = feature.byteOffset | 0;
//            auto featureType = static_cast<FeatureType>(int(feature.type) | int(_feature));
//            auto featureComponentType = static_cast<ComponentType>(int(feature.componentType) | int(_component));
//            int stride;
//
//            switch (featureType) {
//                case FeatureType::SCALAR: {
//                    stride = 1;
//                    break;
//                }
//                case FeatureType::VEC2: {
//                    stride = 2;
//                    break;
//                }
//                case FeatureType::VEC3: {
//                    stride = 3;
//                    break;
//                }
//                case FeatureType::VEC4: {
//                    stride = 4;
//                    break;
//                }
//            }
//
////            unsigned char* data;
//            int arrayStart = binOffset + byteOffset;
//            int arrayLength = _count * stride;
//
//            switch (featureComponentType) {
//                case ComponentType::BYTE: {
//                    if(arrayStart + arrayLength * 1 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//                case ComponentType::UNSIGNED_BYTE: {
//                    if(arrayStart + arrayLength * 1 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//                case ComponentType::SHORT:{
//                    if(arrayStart + arrayLength * 2 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//                case ComponentType::UNSIGNED_SHORT:{
//                    if(arrayStart + arrayLength * 2 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//                case ComponentType::INT:{
//                    if(arrayStart + arrayLength * 4 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//                case ComponentType::UNSIGNED_INT:{
//                    if(arrayStart + arrayLength * 4 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//                case ComponentType::FLOAT:{
//                    if(arrayStart + arrayLength * 4 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//                case ComponentType::DOUBLE:{
//                    if(arrayStart + arrayLength * 8 > binOffset + binLength) {
//                        throw std::runtime_error("FeatureTable: Feature data read outside binary body length.");
//                    };
//                    break;
//                }
//            }
//            return &buffer[arrayStart];
//        }
//    }

#endif //MAIN_B3DM_H
