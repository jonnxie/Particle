//
// Created by maybe on 2021/6/5.
//
#include <Engine/App/shatterapp.h>
#include "precompiledhead.h"

#include "bpool.h"
#include "Engine/Buffer/shatterbufferinclude.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Object/device.h"
#include "mpool.h"
#include "modelsetpool.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Item/configs.h"
#include UniformCatalog
static bool created = false;
static std::mutex pool_mutex;


BPool &BPool::getPool() {
    static BPool pool;
    std::lock_guard<std::mutex> guard_mutex(pool_mutex);
    if(!created){
        created = true;
        pool.init();
    }
    return pool;
}

void BPool::pushBuffer(const B_id& _id, VkDeviceSize _size, ShatterBuffer* _buffer, Buffer_Type _type){
    switch (_type) {
        case Buffer_Type::Vertex_Buffer:{
            std::lock_guard<std::mutex> guard(m_v_mutex);
            checkMapPrint(m_vertex_map)
            m_vertex_map[_id] = _buffer;
            m_size_map[_id] = _size;
            break;
        }
        case Buffer_Type::Index_Buffer:{
            std::lock_guard<std::mutex> guard(m_i_mutex);
            checkMapPrint(m_index_map)
            m_index_map[_id] = _buffer;
            m_size_map[_id] = _size;
            break;
        }
        default:{
            WARNING("dont support such buffer type");
            break;
        }
    }
    return;
}

void BPool::pushTexture(const B_id& _id,ShatterTexture* _texture)
{
    std::lock_guard<std::mutex> guard(m_t_mutex);
    checkMapPrint(m_texture_map)
    m_texture_map[_id] = _texture;
}


void BPool::createVertexBuffer(const B_id& _id, VkDeviceSize _size, void *_data) {
    std::lock_guard<std::mutex> guard(m_v_mutex);
    checkMapPrint(m_vertex_map)
    m_vertex_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Vertex_Buffer, _data);
    m_size_map[_id] = _size;
}

void BPool::createVertexHostBuffer(const B_id &_id, VkDeviceSize _size) {
    std::lock_guard<std::mutex> guard(m_v_mutex);
    checkMapPrint(m_vertex_map)
    m_vertex_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Vertex_Host_Buffer, nullptr);
    m_size_map[_id] = _size;
}

void BPool::createVertexHostBuffer(const B_id& _id,VkDeviceSize _size,void* _data)
{
    std::lock_guard<std::mutex> guard(m_v_mutex);
    checkMapPrint(m_vertex_map)
    m_vertex_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Vertex_Host_Buffer, _data);
    m_size_map[_id] = _size;
}


void BPool::createVSBuffer(const B_id& _id,VkDeviceSize _size,void* _data) {
    std::lock_guard<std::mutex> guard_s(m_s_mutex);
    checkMapPrint(m_storage_map)
    m_storage_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::VS_Buffer, _data);
    m_size_map[_id] = _size;
}

void BPool::createIndexHostBuffer(const B_id& _id,VkDeviceSize _size,void* _data){
    std::lock_guard<std::mutex> guard(m_i_mutex);
    checkMapPrint(m_index_map)
    m_index_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Index_Host_Buffer, _data);
    m_size_map[_id] = _size;
}


void BPool::createIndexBuffer(const B_id& _id, VkDeviceSize _size, void *_data) {
    std::lock_guard<std::mutex> guard(m_i_mutex);
    checkMapPrint(m_index_map)
    m_index_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Index_Buffer, _data);
    m_size_map[_id] = _size;
}

void BPool::createStorageBuffer(const B_id& _id, VkDeviceSize _size, void *_data) {
    std::lock_guard<std::mutex> guard(m_s_mutex);
    checkMapPrint(m_storage_map)
    m_storage_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Storage_Buffer, _data);
    m_size_map[_id] = _size;
}

void BPool::createStorageHostBuffer(const B_id& _id, VkDeviceSize _size, void *_data) {
    std::lock_guard<std::mutex> guard(m_s_mutex);
    checkMapPrint(m_storage_map)
    m_storage_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Storage_Host_Buffer, _data);
    m_size_map[_id] = _size;
}

void BPool::createUniformBuffer(const B_id& _id, VkDeviceSize _size) {
    std::lock_guard<std::mutex> guard(m_u_mutex);
    checkMapPrint(m_uniform_map)
    m_uniform_map[_id] = ShatterBuffer::createBuffer(_size, Buffer_Type::Uniform_Buffer, nullptr);
    m_size_map[_id] = _size;
}

void BPool::createMultipleUniformBuffer(const B_id& _id, int _count, VkDeviceSize _size) {
    std::lock_guard<std::mutex> guard(m_u_mutex);
    std::string id;
    for (int i = 0; i < _count; i++) {
        if (m_uniform_map.count(tool::combine(_id, _count)) != 0) {
            printf("Uniform Buffer Key is already exited\n");
            freeBuffer(tool::combine(_id, _count), Buffer_Type::Uniform_Buffer);
        }
        m_uniform_map[tool::combine(_id, _count)] = ShatterBuffer::createBuffer(_size, Buffer_Type::Uniform_Buffer, nullptr);
    }
    m_size_map[_id] = _size;
}

void BPool::createTexture(const B_id& _id,const std::string& _filename){
    std::lock_guard<std::mutex> guard(m_t_mutex);
    checkMapPrint(m_texture_map)
    m_texture_map[_id] = ShatterTexture::createTexture(_filename);
}

void BPool::createTexture(const B_id& _id,const std::string& _filename, ImageType _type)
{
    std::lock_guard<std::mutex> guard(m_t_mutex);
    checkMapPrint(m_texture_map)
    m_texture_map[_id] = ShatterTexture::createTexture(_type,_filename);
}

void BPool::createTexture(const B_id& _id,int _width,int _height,int _depth){
    std::lock_guard<std::mutex> guard(m_t_mutex);
    checkMapPrint(m_texture_map)
    m_texture_map[_id] = ShatterTexture::createTexture3d(_width, _height, _depth);
}

void BPool::createTexture(ImageType _type,const B_id& _id,const std::string& _file)
{
    std::lock_guard<std::mutex> guard(m_t_mutex);
    checkMapPrint(m_texture_map)
    m_texture_map[_id] = ShatterTexture::createTexture(_type, _file);
}

void BPool::createCubeTexture(const B_id& _id,const std::vector<std::string>& _files)
{
    std::lock_guard<std::mutex> guard(m_t_mutex);
    checkMapPrint(m_texture_map)
    m_texture_map[_id] = ShatterTexture::createCubeTexture(_files);
}

void BPool::createVolumeTexture(const B_id& _id, const std::string& _file, int _width, int _height, int _depth)
{
    std::lock_guard<std::mutex> guard(m_t_mutex);
    checkMapPrint(m_texture_map)
    m_texture_map[_id] = ShatterTexture::createVolumeTexture(_file,_width,_height,_depth);
}

ShatterBuffer* BPool::getBuffer(const B_id& _id, Buffer_Type _type){
    switch(_type){
        case Buffer_Type::Vertex_Host_Buffer:
        case Buffer_Type::Vertex_Buffer:{
            findMap(m_vertex_map, VertexBuffer);
            return m_vertex_map[_id];
        }
        case Buffer_Type::Index_Host_Buffer:
        case Buffer_Type::Index_Buffer:{
            findMap(m_index_map, IndexBuffer);
            return m_index_map[_id];
        }
        case Buffer_Type::Uniform_Buffer:{
            findMap(m_uniform_map, UniformBuffer);
            return m_uniform_map[_id];
        }
        case Buffer_Type::VS_Buffer:
        case Buffer_Type::Storage_Host_Buffer:
        case Buffer_Type::Storage_Buffer:{
            findMap(m_storage_map, StorageBuffer);
            return m_storage_map[_id];
        }
    }
    return nullptr;
}


void BPool::freeBuffer(const B_id& _id,Buffer_Type _type)
{
    switch(_type){
        case Buffer_Type::Vertex_Host_Buffer:
        case Buffer_Type::Vertex_Buffer:{
            if(m_vertex_map.count(_id) == 0)
            {
                return;
            }
            delete m_vertex_map[_id];
            m_vertex_map.erase(_id);
            break;
        }
        case Buffer_Type::Index_Buffer:{
            if(m_index_map.count(_id) == 0)
            {
                return;
            }
            delete m_index_map[_id];
            m_index_map.erase(_id);
            break;
        }
        case Buffer_Type::Uniform_Buffer:{
            if(m_uniform_map.count(_id) == 0)
            {
                return;
            }
            delete m_uniform_map[_id];
            m_uniform_map.erase(_id);
            break;
        }
        case Buffer_Type::Storage_Host_Buffer:
        case Buffer_Type::VS_Buffer:
        case Buffer_Type::Storage_Buffer:{
            if(m_storage_map.count(_id) == 0)
            {
                return;
            }
            delete m_storage_map[_id];
            m_storage_map.erase(_id);
            break;
        }
    }
}



void BPool::init() {
    m_model_count = Config::getConfig("DefaultModelCount");
    createUniformBuffer("CameraBuffer",sizeof(CameraBuffer));
    createUniformBuffer("Model",sizeof(glm::mat4) * m_model_count);
    createUniformBuffer("CameraPos",sizeof(glm::vec3));
    createUniformBuffer("AnimationUniform",sizeof(float));
    createUniformBuffer("CascadeShadow",CascadeShadowSize);
    createUniformBuffer("CascadeIndex",4);
    createUniformBuffer("ViewPort",8);
    createUniformBuffer("ViewSpaceDepth",  8);

    createTexture(ImageType::KTXDimension,"glass",tool::combineTexture("colored_glass_rgba.ktx"));
    createVertexHostBuffer("CameraTargetPlane",CameraBufferSize);

    m_uniform_map["Model"]->map();
    m_uniform_map["CameraPos"]->map();
    m_uniform_map["CascadeShadow"]->map();
    m_uniform_map["CascadeIndex"]->map();
    m_uniform_map["ViewSpaceDepth"]->map();
    m_vertex_map["CameraTargetPlane"]->map();

    m_uniform_map["ViewPort"]->map();
    float view[2] = {SingleAPP.getPresentViewPort().view.width, SingleAPP.getPresentViewPort().view.height};
    memcpy(m_uniform_map["ViewPort"]->mapped, view, 8);

    m_idle_model.resize(m_model_count);
    for(auto i = 0 ; i < m_model_count ; i++){
        m_idle_model[i] = i;
    }
    models = new glm::mat4[m_model_count];
    model_size = m_model_count * sizeof(glm::mat4);
    TaskPool::pushUpdateTask("ModelUpdate",[&](float _absTime){
        memcpy(m_uniform_map["Model"]->mapped, models, model_size);
    });
}

void BPool::release() {
    delete models;

    for(auto &i:m_vertex_map)
    {
        delete i.second;
    }
    m_vertex_map.clear();

    for(auto &i:m_index_map)
    {
        delete i.second;
    }
    m_index_map.clear();

    for(auto &i:m_uniform_map)
    {
        delete i.second;
    }
    m_uniform_map.clear();

    for(auto &j:m_storage_map)
    {
        auto _id = j.first;
        checkMapContinue(m_vertex_map);
        delete j.second;
    }
    m_storage_map.clear();

    for(auto &i:m_texture_map)
    {
        delete i.second;
    }
    m_texture_map.clear();
}

int BPool::mallocModel() {
//    std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
    int val;
    if(m_idle_model.empty())
    {
        reallocateModel();
    }
    val = m_idle_model.front();
    m_idle_model.erase(m_idle_model.begin());
    return val;
}

void BPool::freeModel(int _index) {
//    std::lock_guard<std::mutex> guard_mutex(m_idle_mutex);
    m_idle_model.insert(m_idle_model.end(),_index);
}

void BPool::reallocateModel() {
    freeBuffer("Model",Buffer_Type::Uniform_Buffer);
    createUniformBuffer("Model",sizeof(glm::mat4) * 2 * m_model_count);
    auto buffer = BPool::getPool().getBuffer("Model", Buffer_Type::Uniform_Buffer);
    buffer->map();
    memcpy(buffer->mapped, models, model_size);

    int old_num = m_model_count;
    m_model_count *= 2;
    model_size *= 2;
    int num = m_model_count - old_num;
    for(auto i = 0; i < num; i++){
        m_idle_model.emplace_back(old_num++);
    }
    auto oldPtr = models;
    models = new glm::mat4[m_model_count];
    memcpy(models, oldPtr, model_size / 2);
    delete oldPtr;
}

size_t BPool::getSize(const B_id &_id) {
    return m_size_map[_id];
}
