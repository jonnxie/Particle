//
// Created by jonnxie on 2021/11/6.
//

#ifndef SHATTER_ENGINE_PBRBASIC_H
#define SHATTER_ENGINE_PBRBASIC_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"

class PBRBasic : public Object{
public:
    explicit PBRBasic(const std::string& _files,int _id);
    ~PBRBasic(){
        m_model->release();
    }
    void constructG() override;
    void constructD() override;
    void constructC() override {};
    void update(float) override {};
private:
    vkglTF::Model* m_model;
    std::vector<Point3d_Normal> m_point;
    std::vector<uint32_t> m_index;
    int             m_id;
    glm::vec4       m_lights[4];
};


#endif //SHATTER_ENGINE_PBRBASIC_H
