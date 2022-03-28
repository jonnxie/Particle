//
// Created by jonnxie on 2022/3/28.
//

#ifndef MAIN_I3DM_H
#define MAIN_I3DM_H

#include <utility>
#include <vector>
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include <array>
#include <unordered_map>
#include "json.hpp"
#include "FeatureTable.h"
#include "Engine/Object/VulkanglTFModels.h"
#include "Engine/Object/object.h"

using namespace nlohmann;

class I3DMLoaderBase{
public:
    explicit I3DMLoaderBase(const std::string &_file, bool _url = false);
    ~I3DMLoaderBase();
    DefineUnCopy(I3DMLoaderBase);

private:
    void init();
    std::string resolveExternalURL(const std::string& _url);

protected:
    std::vector<unsigned char> glbBytes{};
private:
    int  version{};
    unsigned char *binaryData{};
    std::string workingPath{};
    int binaryData_index = 0;
    FeatureTable featureTable;
    BatchTable batchTable;
    int glbStart{};
};

class I3DMBasic;

class I3DMLoader : public I3DMLoaderBase{
public:
    explicit I3DMLoader(const std::string &_file);
    ~I3DMLoader(){
        delete m_model;
    };
    DefineUnCopy(I3DMLoader);
public:
    void init(const std::string& _file);
    void loadI3DMFile(const std::string& _file);
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
    std::unique_ptr<I3DMBasic>  m_basic{nullptr};
};

class I3DMBasic : public Object {
public:
    explicit I3DMBasic(vkglTF::Model* _model,
                       glm::vec3 _pos,
                       glm::vec3 _rotationAxis,
                       float _angle,
                       glm::vec3 _scale,
                       int _id,
                       std::string   _pipeline = "AGBasic",
                       std::vector<std::string>   _sets = {"Camera", "Planet"});
    ~I3DMBasic() override;
    void constructG() override {};
    void constructD() override;
    void constructC() override {};

private:
    vkglTF::Model*              m_model{};
    std::string                 m_pipeline;
    std::vector<std::string>    m_sets;

    int                         m_id;
};

#endif //MAIN_I3DM_H
