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
#include "Engine/pool/bpool.h"

template<class CommandBuffer>
class DObjectBase{
public:
    DObjectBase(){
        m_drawDepth =[this](CommandBuffer _cb){
            drawDepth(_cb);
        };

        m_newDraw = [this](CommandBuffer _cb){
            draw(_cb);
        };

        m_gGraphics = [this](CommandBuffer _cb){
            g(_cb);
        };

        m_update = [this](){
            update();
        };
    };
    ~DObjectBase() = default;
public:
    virtual void draw(CommandBuffer _cb,int _imageIndex){};
    void update(){
        if(state_changed)
        {
            if(m_type == DType::Normal)
            {
                glm::mat4* ptr = SingleBPool.getModels();
                memcpy(ptr + m_model_index,&m_matrix,one_matrix);
            }
            state_changed = false;
        }
    };

    void insertPre(const std::function<void(CommandBuffer)>& _func)
    {
        m_pre_tasks.push_back(_func);
    };

    void insertLater(const std::function<void(CommandBuffer)>& _func)
    {
        m_later_tasks.push_back(_func);
    };

    void loopPre(CommandBuffer _cb)
    {
        for(auto &task : m_pre_tasks){
            task(_cb);
        }
    };

    void loopLater(CommandBuffer _cb){
        for(auto &task : m_later_tasks){
            task(_cb);
        }
    };

    void prepare(const glm::mat4& _matrix,int _model_index,DrawType _draw_type,uint64_t _vertex_offset,
                 const std::string& _vertex_buffer,uint32_t _vertex_num,const std::string& _index_buffer,uint32_t _index_num,
                 uint64_t _index_offset,const std::string& _pipeline,std::vector<Set_id>& _set){

    };
public:
    virtual void drawDepth(CommandBuffer _cb){};
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
    virtual void draw(CommandBuffer _cb) {  };

    std::function<void(CommandBuffer)> m_newDraw;

    CommandBuffer drawCMB;
public:
    virtual void g(CommandBuffer _cb) { };

    P_id m_gGP = "Build";

    std::vector<Set_id> m_gDescriptorSet{"Camera"};

    std::function<void(CommandBuffer)> m_gGraphics;

    CommandBuffer gCMB;
public:
    /*
     * index buffer
     */
    std::string m_index_buffer;
    uint32_t m_index_num{};
    uint64_t m_index_offsets = 0;

    glm::mat4 m_matrix{};

    bool state_changed = true;
    /*
     * 获取模型矩阵的偏移索引
     */
    int m_model_index{};

    DrawType m_drawType = DrawType::Index;

    uint64_t m_offsets = 0;

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
    std::vector<std::function<void(CommandBuffer)>> m_pre_tasks;
    std::vector<std::function<void(CommandBuffer)>> m_later_tasks;

    std::function<void(CommandBuffer,int)> m_ray_task;
    std::function<void(CommandBuffer)> m_instance_task;

    std::function<void()> m_update;

    DType m_type = DType::Normal;

    bool m_visible = true;
};

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
                 VkDeviceSize _index_offset,const std::string& _pipeline,std::vector<Set_id>& _set,const std::string& _gPipeline = "Build",std::vector<Set_id> _gSet = {"Camera"});

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
