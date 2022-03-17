//
// Created by jonnxie on 2021/12/1.
//

#ifndef SHATTER_ENGINE_ANIMATION_H
#define SHATTER_ENGINE_ANIMATION_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <valarray>
#include <unordered_map>

#include ObjectCatalog
#include MemPoolCatalog

class Listener;

class DLines;

namespace animation {
    class Animation;

    using Avec3 = glm::vec3;

    using Amat4 = glm::mat4;

    using Avec4 = glm::vec4;

    using Avec2 = glm::vec2;

    void setTransition(int _des,int _src,Animation *_animation);//dest mat4 src vec3

    void multiple(int _a,int _b,int _des,Animation *_animation);//mat4

    void transform(int _matrix/*mat4*/,int _in,int _out/*vec3*/,Animation *_animation);

    void getRotateFromQuaternion(int _quaternion/* vec4 */,int _rotate/* mat4 */,Animation *_animation);

    void genRotationFromEulerAngle(int _angle/*vec3*/,int _mat/*matrix*/,Animation *_animation);

    /*
     * const glm::mat4* _mat,const glm::vec3* _in,glm::vec3* _out
     */
    void invTranAndRotate(int _mat,int _in,int _out,Animation *_animation);

    void loadIdentity(int _matrix,Animation *_animation);

    std::string format(const std::string& _path);

    struct Header {
        std::string id;
        int version;
    };
    void constructHeader(unsigned char *_data, int* binaryData_index, Header& _header);

    struct Vertex{
        int bone;
        int initPosition;//vec3
        int currPosition;//vec3
    };
    void currentPosition(const Vertex& _vertex,Avec3 & _val,bool _inout,Animation *_animation);
    void constructVertex(unsigned char *_data,int* binaryData_index,Vertex& _vertex,int& _vec3index,Animation *_animation);

    /*1 + 3*4 + 2*/
    #define AVertexSize 15


    struct Triangle{
        int index[3];
        int uv[3]; // vec2
        int smoothingGroup;
        int groupIndex;
        int normals[3]; // vec3
    };
    void textureCoordinate(const Triangle& _tri,Avec2 & _val,int _index,bool _inout,Animation *_animation);
    void triangleNormal(const Triangle& _tri,Avec3 & _val,int _index,bool _inout,Animation *_animation);
    void constructTriangle(unsigned char *_data, int* binaryData_index,Triangle& _tri,int& _vec2index,int& _vec3index,Animation *_animation);

    /*2 + 2*3 + 3*4*3 + 3*4 + 3*4 + 2*/
#define ATriangleSize 70

    struct Group{
        int* indices;    //对应池内的开始索引，指向组内三角形索引数组首地址的索引
        int materialIndex;
        int count_ind;
        ~Group(){
            delete indices;
        }
    };
    void constructGroup(unsigned char *_data,int* binaryData_index, Group& _group);

    /*1 + 32 + 2 + 2*count_ind + 1*/
#define AGroupSize

    struct Material{
        std::string name;
        std::string textureName;
        float shininess;
        float transparency;
        int ambient_color; // vec4
        int diffuse_color; // vec4
        int specular_color; // vec4
        int emissive_color; // vec4
    };
    void materialColor(const Material& _tri,Avec4 & _val,Color _color,bool _inout,Animation *_animation);
    void constructMaterial(unsigned char *_data,int* binaryData_index, Material& _mat, int& _vec4index,Animation *_animation);

    /* 32 + 4*4 + 4*4 + 4*4 + 4*4 +4 + 4 + 1 + 128 + 128*/
#define AMaterialSize 361

    struct KeyFrameRotate {
        float time;
        int rotate;//vec4
    };
    void keyFrameRotate(const KeyFrameRotate& _rotate,Avec4 & _val,bool _inout,Animation *_animation);
    void constructKeyFrameRotate(unsigned char *_data,int* binaryData_index, KeyFrameRotate& _mat, int& _vec4index,Animation *_animation);

    /*4 + 4 * 3*/
#define AKeyFrameRotateSize 16

    struct KeyFrameTransition {
        float time;
        int transition; //vec3
    };
    void kyFrameTransition(const KeyFrameTransition& _transition,Avec3 & _val,bool _inout,Animation *_animation);
    void constructKeyFrameTransition(unsigned char *_data,int* binaryData_index, KeyFrameTransition& _mat, int& _vec3index,Animation *_animation);

    /*4 + 3*4*/
#define AKeyFrameTransitionSize 16

    class Joint{
    public:
        Joint();

        ~Joint();

        void update(float _relativeTime);

        void rotation(float _relativeTime);

        void transition(float _relativeTime);

        void getMatrix(glm::mat4& _out) const;//Get now absolute matrix

        void getAbsolute(glm::mat4& _out) const;//Get initial absolute matrix
    public:
        std::string m_name{};
        std::string m_parent_name{};
        bool m_isolate = true;//if exist parent Joint
        int m_init_rotation{};//vec3
        int m_init_transition{};//vec3
        int m_init_relative{};//Initial relative matrix
        int m_init_absolute{};//Initial absolute matrix
        Joint *m_parent{};
    public:
        int m_real_abs{};//Real time absolute matrix
        int m_help{};//mat4
        std::vector<KeyFrameRotate> m_rotates{};
        int m_rotation_help{};//vec4
        std::vector<KeyFrameTransition> m_transitions{};
        int m_transition_help{};//vec3
        int m_rotation_count{};
        int m_transition_count{};
        Animation* _animation;
    };
    void constructJoint(unsigned char* _data,int* binaryData_index,Joint& _joint,int& _vec3index,int& _vec4index,int& _matindex,Animation *_animation);

    /*1 + 32 + 32 + 4*3 + 4*3 + 2*2 + count_rot * AKeyFrameRotateSize + count_pos * AKeyFrameTransitionSize */
#define AJointSize

    class Animation :public Object {
    public:
        Animation(const std::string &_file, glm::vec3 _pos,glm::vec3 _rotationAxis,float _angle,glm::vec3 _scale,int _id);

        ~Animation();

        void constructG() override;

        void constructD() override;

        void constructC() override{};

        void initAnimation();

        void initVertexData();

        void initVertexBuffer();

        void updateJoint(float _time);

        void updateAllVertexes();

        void animate(float _time);

        void updateBuffer(); //绘制方法

        void reAssemVertexData(); //更新顶点数据后重新组装数据的方法

    private:
        int m_id;
        DLines* m_localCoordinate;
    public:
        Header m_header;
        std::vector<Vertex> m_vertexes;
        std::vector<Triangle> m_triangles;
        std::vector<Group> m_groups;
        std::vector<Material> m_materials;
        std::vector<Joint*> m_joints;
        std::vector<float*> m_vertex_data;
        std::vector<VkDescriptorBufferInfo> m_vertex_buffer_info;
        std::vector<VkDescriptorSet> m_sets;
        BigPool<Avec2>* m_vec2Pool = nullptr;
        BigPool<Avec3>* m_vec3Pool = nullptr;
        BigPool<Avec4>* m_vec4Pool = nullptr;
        BigPool<Amat4>* m_mat4Pool = nullptr;
    public:
        glm::vec3       m_cameraTarget{1.0f,1.0f,1.0f};
        glm::mat4       m_world{};
        glm::mat4       m_scale{};
        glm::mat4       m_rotate{};
        glm::mat4       m_translation{};
        int m_vertex_count;
        /*
         * vertex group count
         */
        int     m_group_count;
        int     m_joint_count;
        float   m_fps;
        float   m_current_time;
        float   m_total_time;
        int     m_frame_count;
        /*
         * binary format file contents
         */
        unsigned char *m_binary_data;
        float m_bright = 1.0f;
    };

}

#endif //SHATTER_ENGINE_ANIMATION_H
