//
// Created by maybe on 2021/6/2.
//

#ifndef SHATTER_ENGINE_DOBJECT_H
#define SHATTER_ENGINE_DOBJECT_H

#include <glm.hpp>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <future>
#include <type_traits>
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"

class DObject {
public:
    DObject();
     ~DObject() = default;
public:
    void draw(VkCommandBuffer _cb);
    void draw(VkCommandBuffer _cb,int _imageIndex);
    void update();
    void prepare(const glm::mat4& _matrix,int _model_index,DrawType _draw_type,VkDeviceSize _vertex_offset,
                 const std::string& _vertex_buffer,uint32_t _vertex_num,const std::string& _index_buffer,uint32_t _index_num,
                 VkDeviceSize _index_offset,const std::string& _pipeline,std::vector<Set_id>& _set);

    void insertPre(const std::function<void(VkCommandBuffer)>&);

    void insertLater(const std::function<void(VkCommandBuffer)>&);

    void loopPre(VkCommandBuffer _cb);

    void loopLater(VkCommandBuffer _cb);

public:
    void drawDepth(VkCommandBuffer _cb);
    std::function<void(VkCommandBuffer)> m_drawDepth;
    /*
    * depth graphics pipeline
    */
    std::string m_depthGP = "ShadowDepth";
    /*
    * descriptor set
    */
    std::vector<Set_id> m_depthDescriptorSet{"Cascade"};

public:
    void newDraw(VkCommandBuffer _cb);

    std::function<void(VkCommandBuffer)> m_newDraw;

    VkCommandBuffer drawCMB;
public:
    void g(VkCommandBuffer _cb);

    P_id m_gGP = "Build";

    std::vector<Set_id> m_gDescriptorSet{"Camera"};

    std::function<void(VkCommandBuffer)> m_gGraphics;

    VkCommandBuffer gCMB;
public:
    /*
     * index buffer
     */
    std::string m_index_buffer;
    uint32_t m_index_num{};
    VkDeviceSize m_index_offsets = 0;

    glm::mat4 m_matrix{};

    bool state_changed = true;
    /*
     * 获取模型矩阵的偏移索引
     */
    int m_model_index{};

    DrawType m_drawType = DrawType::Index;

    VkDeviceSize m_offsets = 0;

    /*
    * vertex buffer
    */
    std::string m_vertex_buffer;
    uint32_t m_vertex_num{};


    /*
    * graphics pipeline
    */
    std::string m_gp;

    /*
     * descriptor set
     */
    std::vector<Set_id> m_descriptorSet{};

    D_id m_id{0};

    /*
     * task used in pre draw
     */
    std::vector<std::function<void(VkCommandBuffer)>> m_pre_tasks;
    std::vector<std::function<void(VkCommandBuffer)>> m_later_tasks;

    std::function<void(VkCommandBuffer,int)> m_ray_task;
    std::function<void(VkCommandBuffer)> m_instance_task;

    DType m_type = DType::Normal;

    bool m_visible = true;
public:
    [[nodiscard]] D_id getMId() const;
};

#endif //SHATTER_ENGINE_DOBJECT_H
