//
// Created by maybe on 2021/6/5.
//

#ifndef SHATTER_ENGINE_BPOOL_H
#define SHATTER_ENGINE_BPOOL_H

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <unordered_map>

namespace Shatter::buffer{
    class ShatterBuffer;
    class ShatterTexture;
}

class ModelSetPool;

using namespace Shatter::buffer;

class BPool {
public:
    static BPool& getPool();
    DefineUnCopy(BPool);
    void init();

    void release();

    void pushBuffer(const B_id& _id, VkDeviceSize _size, ShatterBuffer* _buffer, Buffer_Type _type);

    void pushTexture(const B_id& _id,ShatterTexture* _texture);

    void createVertexBuffer(const B_id& _id,VkDeviceSize _size,void* _data);

    void createVertexHostBuffer(const B_id& _id,VkDeviceSize _size);

    void createVertexHostBuffer(const B_id& _id,VkDeviceSize _size,void* _data);

    void createVSBuffer(const B_id& _id,VkDeviceSize _size,void* _data);

    void createIndexHostBuffer(const B_id& _id,VkDeviceSize _size,void* _data);

    void createIndexBuffer(const B_id& _id,VkDeviceSize _size,void* _data);

    void createStorageBuffer(const B_id& _id,VkDeviceSize _size,void* _data);

    void createStorageHostBuffer(const B_id& _id,VkDeviceSize _size,void* _data);

    void createUniformBuffer(const B_id& _id,VkDeviceSize _size);

    void createMultipleUniformBuffer(const B_id& _id, int _count, VkDeviceSize _size);

    void createTexture(const B_id& _id,const std::string& _filename);

    void createTexture(const B_id& _id,const std::string& _filename, ImageType _type);

    void createTexture(const B_id& _id,int _width,int _height,int _depth);

    void createTexture(ImageType _type,const B_id& _id,const std::string& _file);

    void createCubeTexture(const B_id& _id,const std::vector<std::string>& _files);

    void createVolumeTexture(const B_id& _id, const std::string& _file, int _width, int _height, int _depth);

    ShatterTexture *getTexture(const B_id &_id) { return m_texture_map[_id]; }

    ShatterBuffer* getBuffer(const B_id& _id, Buffer_Type _type);

    void freeBuffer(const B_id& _id,Buffer_Type _type);

    size_t getSize(const B_id& _id);

    glm::mat4 * getModels(){return models;};

private:
    friend class ModelSetPool;

public:
    int mallocModel();

    void freeModel(int);

    void reallocateModel();

private:
    std::map<B_id,ShatterBuffer*> m_vertex_map;
    std::mutex m_v_mutex;

    std::map<B_id,ShatterBuffer*> m_index_map;
    std::mutex m_i_mutex;

    std::map<B_id,ShatterBuffer*> m_uniform_map;
    std::mutex m_u_mutex;

    std::map<B_id,ShatterBuffer*> m_storage_map;
    std::mutex m_s_mutex;

    std::map<B_id,ShatterTexture*> m_texture_map;
    std::mutex m_t_mutex;

    std::map<B_id,size_t> m_size_map;

    BPool() = default;

    std::vector<int> m_idle_model;
    int m_model_count;
    size_t model_size;
    glm::mat4 * models{nullptr};
};

#define SingleBPool BPool::getPool()

#endif //SHATTER_ENGINE_BPOOL_H
