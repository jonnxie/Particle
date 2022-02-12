//
// Created by jonnxie on 2021/12/1.
//

#include "animation.h"

#include <utility>
#include MathCatalog
#include MemPoolCatalog
//#include "PathData.h"
#include BPoolCatalog
#include BufferCatalog
#include MPoolCatalog
#include DObjectCatalog
#include ModelSetCatalog
#include PPoolCatalog
#include SetPoolCatalog
#include PipelineCatalog
#include GLTFCatalog
#include TaskCatalog
#include ListenerCatalog
#include InputActionCatalog
#include CameraCatalog
#include AppCatalog
#include "../Shatter_Base/lines.h"

namespace animation {
    void currentPosition(const Vertex &_vertex,Avec3 &_val,bool _inout,Animation *_animation) {
        auto vecPool = Avec3Pool;
        if(_inout == STATE_IN){
            vecPool->set(_vertex.currPosition,_val);
        }else{
            vecPool->get(_vertex.currPosition,_val);
        }
    }

    void textureCoordinate(const Triangle &_tri, Avec2 &_val, int _index, bool _inout,Animation *_animation) {
        auto uvPool = Avec2Pool;
        if(_inout == STATE_IN){
            uvPool->set(_tri.uv[_index],_val);
        }else{
            uvPool->get(_tri.uv[_index],_val);
        }
    }

    void triangleNormal(const Triangle &_tri, Avec3 &_val, int _index, bool _inout,Animation *_animation) {
        auto norPool = Avec3Pool;
        if(_inout == STATE_IN){
            norPool->set(_tri.normals[_index],_val);
        }else{
            norPool->get(_tri.normals[_index],_val);
        }
    }

    void materialColor(const Material &_tri, Avec4 &_val, Color _color, bool _inout,Animation *_animation) {
        auto norPool = Avec4Pool;
        int index;
        switch (_color) {
            case Color::Ambient:{
                index = _tri.ambient_color;
            }
            case Color::Diffuse:{
                index = _tri.diffuse_color;
            }
            case Color::Specular:{
                index = _tri.specular_color;
            }
            case Color::Emissive:{
                index = _tri.emissive_color;
            }
            default:{
                assert(0);
            }
        }
        if(_inout == STATE_IN){
            norPool->set(index,_val);
        }else{
            norPool->get(index,_val);
        }
    }

    void keyFrameRotate(const KeyFrameRotate& _rotate,Avec4 & _val,bool _inout,Animation *_animation)
    {
        auto pool = Avec4Pool;
        if(_inout == STATE_IN){
            pool->set(_rotate.rotate,_val);
        }else{
            pool->get(_rotate.rotate,_val);
        }
    }

    void kyFrameTransition(const KeyFrameTransition& _transition,Avec3 & _val,bool _inout,Animation *_animation)
    {
        auto pool = Avec3Pool;
        if(_inout == STATE_IN){
            pool->set(_transition.transition,_val);
        }else{
            pool->get(_transition.transition,_val);
        }
    }

    void setTransition(int _des, int _src,Animation *_animation) {
        glm::mat4 mat;
        glm::vec3 vec;
        Amat4Pool->get(_des,mat);
        Avec3Pool->get(_src,vec);
        setTranslate((glm::mat4*)&mat, (glm::vec3*)&vec);
        Amat4Pool->set(_des,mat);
    }

    void multiple(int _a, int _b,int _des,Animation *_animation) {
        glm::mat4 left;
        glm::mat4 right;
        Amat4Pool->get(_a,left);
        Amat4Pool->get(_b,right);
        glm::mat4 dest;
        dest = left * right;
        Amat4Pool->set(_des,dest);
    }

    void transform(int _matrix/*mat4*/,int _in,int _out/*vec3*/,Animation *_animation){
        glm::mat4 matrix;
        glm::vec3 in,out;
        Amat4Pool->get(_matrix,matrix);
        Avec3Pool->get(_in,in);
        out = glm::vec3(matrix * glm::vec4(in,1.0f));
        Avec3Pool->set(_out,out);
    }

    void getRotateFromQuaternion(int _quaternion, int _rotate,Animation *_animation) {
        glm::mat4 mat;
        glm::vec4 vec;
//        Amat4Pool->get(_rotate,mat);
        Avec4Pool->get(_quaternion,vec);
        genRotateFromQuaternion(&vec, &mat);
        Amat4Pool->set(_rotate,mat);
    }

    void genRotationFromEulerAngle(int _angle/*vec3*/,int _mat/*matrix*/,Animation *_animation) {
        glm::vec3 angle;
        glm::mat4 matrix;
        Avec3Pool->get(_angle,angle);
        Amat4Pool->get(_mat,matrix);
        genRotateFromEulerAngle(&angle,&matrix);
        Amat4Pool->set(_mat,matrix);
    }

    /*
     * const glm::mat4* _mat,const glm::vec3* _in,glm::vec3* _out
     */
    void invTranAndRotate(int _mat,int _in,int _out,Animation *_animation){
        glm::mat4 mat;
        glm::vec3 in,out;
        Amat4Pool->get(_mat,mat);
        Avec3Pool->get(_in,in);
        invTransformAndRotate(&mat,&in,&out);
        Avec3Pool->set(_out,out);
    }

    void loadIdentity(int _matrix,Animation *_animation) {
        glm::mat4 mat;
        setIdentity(&mat);
        Amat4Pool->set(_matrix,mat);
    }

    void constructHeader(unsigned char *_data,int* binaryData_index, Header &_header) {
        _header.id = file::myReadString(_data, binaryData_index, 10);
        _header.version = file::myReadInt(_data, binaryData_index);
    }

    void constructVertex(unsigned char *_data,int* binaryData_index, Vertex &_vertex,int& _vec3index,Animation *_animation) {
        file::myReadByte(_data, binaryData_index);//读取顶点在编辑器中的状态，本案例中无用
        glm::vec3 pos;
        pos.x = file::myReadFloat(_data, binaryData_index);
        pos.y = file::myReadFloat(_data, binaryData_index);
        pos.z = file::myReadFloat(_data, binaryData_index);
        _vertex.initPosition = _vec3index++;
        _vertex.currPosition = _vec3index++;
        Avec3Pool->set(_vertex.initPosition,pos);
        _vertex.bone = file::myReadByte(_data, binaryData_index);
        file::myReadByte(_data, binaryData_index);
    }

    void constructTriangle(unsigned char *_data,int* binaryData_index,Triangle &_tri,int& _vec2index,int& _vec3index,Animation *_animation) {
        file::myReadUnsignedShort(_data, binaryData_index);//读取三角形在编辑器中的状态值
        //初始化三角形三个顶点索引的数组
        _tri.index[0] = file::myReadUnsignedShort(_data, binaryData_index); //读取第一个顶点的索引
        _tri.index[1] = file::myReadUnsignedShort(_data, binaryData_index); //读取第二个顶点的索引
        _tri.index[2] = file::myReadUnsignedShort(_data, binaryData_index); //读取第三个顶点的索引
        glm::vec3 nor;
        for (int j = 0; j<3; j++) { //对三角形中的3 个顶点进行遍历
            _tri.normals[j] = _vec3index++;
            nor.x = file::myReadFloat(_data, binaryData_index);//读取当前顶点法向量的X 分量
            nor.y = file::myReadFloat(_data, binaryData_index);//读取当前顶点法向量的Y 分量
            nor.z = file::myReadFloat(_data, binaryData_index);//读取当前顶点法向量的Z 分量
            Avec3Pool->set(_tri.normals[j],nor); //创建存储当前顶点法向量的复合数对象
             //将法向量复合数对象指针存入相应列表
        }
        float s1 = file::myReadFloat(_data, binaryData_index); //读取第一个顶点的纹理S 坐标
        //DebugFloat(s1);
        float s2 = file::myReadFloat(_data, binaryData_index); //读取第二个顶点的纹理S 坐标
        //DebugFloat(s2);
        float s3 = file::myReadFloat(_data, binaryData_index); //读取第三个顶点的纹理S 坐标
        //DebugFloat(s3);
        //创建存储三角形面三个顶点纹理S 坐标的复合数对象

        float t1 = file::myReadFloat(_data, binaryData_index); //读取第一个顶点的纹理T 坐标
        //DebugFloat(t1);
        float t2 = file::myReadFloat(_data, binaryData_index); //读取第二个顶点的纹理T 坐标
        //DebugFloat(t2);
        float t3 = file::myReadFloat(_data, binaryData_index); //读取第三个顶点的纹理T 坐标
        //DebugFloat(t3);

        //创建存储三角形面三个顶点纹理T 坐标的复合数对象
        _tri.uv[0] = _vec2index++;
        Avec2Pool->set(_tri.uv[0],glm::vec2(s1,t1));
        _tri.uv[1] = _vec2index++;
        Avec2Pool->set(_tri.uv[1],glm::vec2(s2,t2));
        _tri.uv[2] = _vec2index++;
        Avec2Pool->set(_tri.uv[2],glm::vec2(s3,t3));
        _tri.smoothingGroup = file::myReadByte(_data, binaryData_index);//读取三角形面所处平滑组编号
        _tri.groupIndex = file::myReadByte(_data, binaryData_index); //读取三角形面所处的组索引
    }

    void constructGroup(unsigned char *_data,int* binaryData_index, Group &_group) {
        file::myReadByte(_data, binaryData_index);//读取该组在编辑器中的状态，本案例中无用
        file::myReadString(_data, binaryData_index, 32); //读取组名称，本案例中无用
        _group.count_ind = file::myReadUnsignedShort(_data, binaryData_index);//读取组内三角形数量
        _group.indices = new int[_group.count_ind]; //初始化组内三角形索引数组
        for (int j = 0; j < _group.count_ind; j++) { //读取组内各个三角形的索引
            _group.indices[j] = file::myReadUnsignedShort(_data, binaryData_index);//读取一个三角形索引
        }
        _group.materialIndex = file::myReadByte(_data, binaryData_index); //读取组对应的材质索引
    }

    void constructMaterial(unsigned char *_data,int* binaryData_index, Material &_mat, int& _vec4index,Animation *_animation) {
        _mat.name = file::myReadString(_data, binaryData_index, 32); //读取材质的名称
        glm::vec4 color;
        //创建用于存储环境光数据的数组
        for (int j = 0; j<4; j++) { //循环获取环境光4 个色彩通道的值
            color[j] = //读取并记录环境光的每个色彩通道值
                    file::myReadFloat(_data, binaryData_index);
        }
        _mat.ambient_color = _vec4index++;
        Avec4Pool->set(_mat.ambient_color,color);
        //创建用于存储散射光数据的数组
        for (int j = 0; j<4; j++) { //循环获取散射光4 个色彩通道的值
            color[j] = //读取并记录散射光的每个色彩通道值
                    file::myReadFloat(_data, binaryData_index);
        }
        _mat.diffuse_color = _vec4index++;
        Avec4Pool->set(_mat.diffuse_color,color);
         //创建用于存储镜面光数据的数组
        for (int j = 0; j<4; j++) { //循环获取镜面光4 个色彩通道的值
            color[j] = //读取并记录镜面光的每个色彩通道值
                    file::myReadFloat(_data, binaryData_index);
        }
        _mat.specular_color = _vec4index++;
        Avec4Pool->set(_mat.specular_color,color);

        //创建用于存储自发光数据的数组
        for (int j = 0; j<4; j++) { //循环获取自发光4 个色彩通道的值
            color[j] = //读取并记录自发光的每个色彩通道值
                    file::myReadFloat(_data, binaryData_index);
        }
        _mat.emissive_color = _vec4index++;
        Avec4Pool->set(_mat.emissive_color,color);

        _mat.shininess = file::myReadFloat(_data, binaryData_index); //读取粗糙度信息
        _mat.transparency = file::myReadFloat(_data, binaryData_index); //读取透明度信息
        file::myReadByte(_data, binaryData_index); //此数据在本案例中无用
        std::string tn = file::myReadString(_data, binaryData_index, 128); //读取材质对应纹理图的路径
        _mat.textureName = format(tn); //从文件路径中摘取出纹理图的文件名
        file::myReadString(_data, binaryData_index, 128);//读取透明度贴图文件路径，本案例中无用

        SingleBPool.createTexture(ImageType::BntexDimension, _mat.textureName, TextureFilePath + _mat.textureName + ".bntex");

//        TextureManager::texNames. //将纹理图名称存入纹理名称列表
//                push_back(tp + texName2bntex(textureName));

    }

    std::string format(const std::string &_path) {
        int offset = _path.rfind("\\");
        int endset = _path.rfind("g");
        if(offset != std::string::npos && endset != std::string::npos)
        {
            return _path.substr(offset+1,endset-5);
        }
        return _path;
    }

    void constructKeyFrameRotate(unsigned char *_data,int* binaryData_index, KeyFrameRotate &_mat, int& _vec4index,Animation *_animation) {
        _mat.time = file::myReadFloat(_data, binaryData_index); //读取关键帧时间
        glm::vec3 angle; //创建存储旋转数据的复合数对象
        angle.x = file::myReadFloat(_data, binaryData_index); //读取旋转欧拉角的第1 个分量
        angle.y = file::myReadFloat(_data, binaryData_index); //读取旋转欧拉角的第2 个分量
        angle.z = file::myReadFloat(_data, binaryData_index); //读取旋转欧拉角的第3 个分量
         //将欧拉角形式的旋转数据转换为四元数形式（这是为了在关键帧之间进行插值计算）
        glm::vec4 quaternion;
        setFromEulerAngleToQuaternion(angle,quaternion);
        _mat.rotate = _vec4index++;
        Avec4Pool->set(_mat.rotate,quaternion);
    }

    void constructKeyFrameTransition(unsigned char *_data,int* binaryData_index, KeyFrameTransition &_mat, int& _vec3index,Animation *_animation) {
        _mat.time   = file::myReadFloat(_data, binaryData_index); //读取关键帧时间
        float x     = file::myReadFloat(_data, binaryData_index); //读取平移信息的x 分量
        float y     = file::myReadFloat(_data, binaryData_index); //读取平移信息的y 分量
        float z     = file::myReadFloat(_data, binaryData_index); //读取平移信息的z 分量
        glm::vec3 tran(x,y,z);
        _mat.transition = _vec3index++;
        Avec3Pool->set(_mat.transition,tran); //创建存储平移信息的复合数对象
    }

    void Joint::update(float _relativeTime){
        if (m_rotates.empty() && m_rotates.empty()) { //若没有旋转关键帧和平移关键帧数据
//            auto pool = BigPool<Amat4>::getAnimationPool();
            Amat4Pool->copy(m_real_abs, m_init_absolute);//将初始绝对矩阵的值复制进当前绝对矩阵
            return; //返回
        }
        rotation(_relativeTime); //获取当前时刻的旋转数据
        transition(_relativeTime);//将当前时刻的平移数据记录进矩阵
        multiple(m_init_relative,m_help,m_help,_animation);//与自身相对矩阵相乘
        if (!m_isolate) { //若有父关节
            multiple(m_parent->m_real_abs,m_help,m_real_abs,_animation);//乘以父关节的当前矩阵
        }
        else { //若无父关节
            Amat4Pool->copy(m_real_abs,m_help);//给当前绝对矩阵赋值
        }
    }

    void Joint::rotation(float _relativeTime){
        int index = 0; //初始化索引为0
        int size = m_rotation_count; //获取旋转关键帧的数量
        while (index < size && m_rotates[index].time < _relativeTime) { //根据当前时间计算用于插值的结束关键帧索引
            index++; //关键帧索引加1
        }
        if (index == 0) { //若结束关键帧索引为0
            Avec4Pool->copy(m_rotation_help , m_rotates[index].rotate);//获取第一帧的旋转数据
        }
        else if (index == size) { //若结束关键帧索引等于关键帧数量
            Avec4Pool->copy(m_rotation_help , m_rotates[index - 1].rotate);//获取最后一帧的旋转数据
        }
        else { //若结束关键帧索引既不为0 也不超过最终关键帧
            Avec4Pool->interpolateA(m_rotates[index - 1].rotate,//上一关键帧的旋转
                        m_rotates[index].rotate,//插值用结束关键帧的旋转
                        (_relativeTime - m_rotates[index - 1].time) / (m_rotates[index].time - m_rotates[index - 1].time),
                        m_rotation_help);//插值产生当前时刻的旋转（四元数格式）
        }
        getRotateFromQuaternion(m_rotation_help,m_help,_animation); //将四元数形式的旋转转换为矩阵形式
        //返回当前时刻的旋转数据
    }

    void Joint::transition(float _relativeTime){
        int index = 0; //初始化索引为0
        int size = m_transition_count; //得到平移关键帧的数量
        while (index < size && m_transitions[index].time < _relativeTime) { //根据当前时间计算用于插值的结束关键帧索引
            index++; //关键帧索引加1
        }
        if (index == 0) { //若结束关键帧索引为0
            Avec3Pool->copy(m_transition_help, m_transitions[index].transition);//获取第一帧的平移数据并返回
        }
        else if (index == size) { //若结束关键帧索引等于关键帧数量
            Avec3Pool->copy(m_transition_help, m_transitions[index - 1].transition); //获取最后一帧的平移数据并返回
        }
        else { //若结束关键帧索引既不为0 也不超过最终关键帧
            Avec3Pool->interpolateA( m_transitions[index - 1].transition,//上一关键帧的平移数据
                                     m_transitions[index].transition,//插值用结束关键帧的平移数据
                                    (_relativeTime - m_transitions[index - 1].time) / (m_transitions[index].time - m_transitions[index - 1].time),
                                    m_transition_help);//返回当前时刻的平移数据
        }
        setTransition(m_help, m_transition_help,_animation);
    }

    void Joint::getMatrix(glm::mat4& _out) const//Get now absolute matrix
    {
        Amat4Pool->get(m_real_abs,_out);
    }

    void Joint::getAbsolute(glm::mat4& _out) const//Get initial absolute matrix
    {
        Amat4Pool->get(m_init_absolute,_out);
    }

    Joint::~Joint() {
    }

    Joint::Joint() {
         m_name = "";
         m_parent_name = "";
         m_isolate = true;//if exist parent Joint
         m_init_rotation = 0;//vec3
         m_init_transition = 0;//vec3
         m_init_relative = 0;//Initial relative matrix
         m_init_absolute = 0;//Initial absolute matrix
         m_parent = nullptr;
         m_real_abs = 0;//Real time absolute matrix
         m_help = 0;//mat4
         m_rotates = std::vector<KeyFrameRotate>();
         m_rotation_help = 0;//vec4
         m_transitions = std::vector<KeyFrameTransition>();
         m_transition_help = 0;//vec3
         m_rotation_count = 0;
         m_transition_count = 0;
    }

    Animation::Animation(const std::string &_file,glm::vec3 _pos,glm::vec3 _rotationAxis,float _angle,glm::vec3 _scale, int _id) {
        m_binary_data = file::loadBinary(_file);
        m_id = _id;
        m_scale = glm::scale(glm::mat4(1.0f),_scale);
        m_rotate = glm::rotate(glm::mat4(1.0f),_angle,_rotationAxis);
        m_translation = glm::translate(glm::mat4(1.0f), _pos);
        m_world = m_translation * m_scale * m_rotate;
        init();
    }

    void Animation::constructG() {
        initAnimation();
        initVertexData();
        initVertexBuffer();

        TaskPool::pushUpdateTask(tool::combine("AnimationBright",m_id),[&](float _abs_time) {
            auto buffer = BPool::getPool().getBuffer("AnimationUniform",Buffer_Type::Uniform_Buffer);
            void *data;
            vkMapMemory(Device::getDevice()(),
                        buffer->getMemory(),
                        0,
                        sizeof(float),
                        0, &data);
            memcpy(data, &m_bright, sizeof(float));
            vkUnmapMemory(Device::getDevice()(),
                          buffer->getMemory());

        });
        std::vector<Line> lines{
                {
                        {glm::vec3(0.0f,0.0f,0.0f),
                         PURPLE_COLOR},
                        {glm::vec3(1.0f,0.0f,0.0f),
                         RED_COLOR}
                },
                {
                        {glm::vec3(0.0f,0.0f,0.0f),
                         PURPLE_COLOR},
                        {glm::vec3(0.0f,1.0f,0.0f),
                         CYAN_COLOR}
                },
                {
                        {glm::vec3(0.0f,0.0f,0.0f),
                         PURPLE_COLOR},
                        {glm::vec3(0.0f,0.0f,1.0f),
                         GREEN_COLOR}
                },
        };
        m_localCoordinate = new Lines(lines);
        m_localCoordinate->init();
        TaskPool::pushUpdateTask(tool::combine("Animation",m_id),[&](float _abs_time){
            animate(_abs_time);
            auto& camera = SingleCamera;
            if(checkKey(GLFW_KEY_UP)){
                m_translation = glm::translate(m_translation,0.01f * glm::vec3(camera.m_targetPlane.y_coordinate.x,camera.m_targetPlane.y_coordinate.y,0.0f));
                SingleCamera.center = m_translation * glm::vec4(m_cameraTarget,1.0f);
                shatter::app::ShatterApp::getApp().cameraChanged = true;
            }else if(checkKey(GLFW_KEY_DOWN)){
                m_translation = glm::translate(m_translation,-0.01f * glm::vec3(camera.m_targetPlane.y_coordinate.x,camera.m_targetPlane.y_coordinate.y,0.0f));
                SingleCamera.center = m_translation * glm::vec4(m_cameraTarget,1.0f);
                shatter::app::ShatterApp::getApp().cameraChanged = true;
            }else if(checkKey(GLFW_KEY_LEFT)){
                m_translation = glm::translate(m_translation,-0.01f * glm::vec3(camera.m_targetPlane.x_coordinate.x,camera.m_targetPlane.x_coordinate.y,0.0f));
                SingleCamera.center = m_translation * glm::vec4(m_cameraTarget,1.0f);
                shatter::app::ShatterApp::getApp().cameraChanged = true;
            }else if(checkKey(GLFW_KEY_RIGHT)){
                m_translation = glm::translate(m_translation,0.01f * glm::vec3(camera.m_targetPlane.x_coordinate.x,camera.m_targetPlane.x_coordinate.y,0.0f));
                SingleCamera.center = m_translation * glm::vec4(m_cameraTarget,1.0f);
                shatter::app::ShatterApp::getApp().cameraChanged = true;
            }
            m_world = m_translation * m_scale * m_rotate;

            auto dpool = MPool<DObject>::getPool();
            void *data;
            for(int index : m_dobjs)
            {
                glm::mat4* ptr = SingleBPool.getModels();
                memcpy(ptr + (*dpool)[index]->m_model_index,&m_world,one_matrix);
            }
            int d = m_localCoordinate->m_dobjs[0];
            {
                glm::mat4* ptr = SingleBPool.getModels();
                memcpy(ptr + (*dpool)[(*dpool)[d]->m_model_index]->m_model_index,&m_world,one_matrix);
            }
        });
    }

    void constructJoint(unsigned char *_data, int *binaryData_index, Joint &_joint, int &_vec3index, int &_vec4index,
                        int &_matindex,Animation *_animation) {
        _joint._animation = _animation;
        file::myReadByte(_data, binaryData_index);
        _joint.m_name = file::myReadString(_data, binaryData_index,32);
        _joint.m_parent_name = file::myReadString(_data, binaryData_index,32);

        _joint.m_init_relative = _matindex++;
        _joint.m_init_absolute = _matindex++;
        _joint.m_help = _matindex++;
        _joint.m_rotation_help = _vec4index++;
        _joint.m_transition_help = _vec3index++;

        float x = file::myReadFloat(_data, binaryData_index);
        float y = file::myReadFloat(_data, binaryData_index);
        float z = file::myReadFloat(_data, binaryData_index);
        _joint.m_init_rotation = _vec3index++;
        Avec3Pool->set(_joint.m_init_rotation,glm::vec3(x,y,z));

        x = file::myReadFloat(_data, binaryData_index);
        y = file::myReadFloat(_data, binaryData_index);
        z = file::myReadFloat(_data, binaryData_index);
        _joint.m_init_transition = _vec3index++;
        Avec3Pool->set(_joint.m_init_transition,glm::vec3(x,y,z));

        _joint.m_rotation_count = file::myReadUnsignedShort(_data, binaryData_index);
        _joint.m_rotates.resize(_joint.m_rotation_count);

        _joint.m_transition_count = file::myReadUnsignedShort(_data, binaryData_index);
        _joint.m_transitions.resize(_joint.m_transition_count);

        if(_joint.m_rotation_count > 0)
        {
            for(int i = 0; i < _joint.m_rotation_count; i++)
            {
                constructKeyFrameRotate(_data,binaryData_index,_joint.m_rotates[i],_vec4index,_animation);
            }
        }
        if(_joint.m_transition_count > 0)
        {
            for(int i = 0; i < _joint.m_transition_count; i++)
            {
                constructKeyFrameTransition(_data,binaryData_index,_joint.m_transitions[i],_vec3index,_animation);
            }
        }
        _joint.m_isolate = true;
        _joint.m_real_abs = _matindex++;
    }

    void Animation::constructD() {
        Group* group; //临时辅助用组信息对象指针
        Material* material; //临时辅助用材质信息对象指针
        VkDescriptorSet* desSetPointer; //描述集指针
        auto dpool = MPool<DObject>::getPool();
        for (int i = 0; i < m_groups.size(); i++) { //对模型中的所有组进行遍历
            auto d = dpool->malloc();
            int mc_index = ModelSetPool::getPool().malloc();

            group = &m_groups[i]; //获取当前组信息对象
            int triangleCount = group->count_ind; //获取组内三角形的数量
            if (group->materialIndex > -1) { //若有材质（即需要贴纹理）
                desSetPointer = &m_sets[group->materialIndex];
            }
            (*dpool)[d]->m_model_index = mc_index;
//            (*dpool)[d]->m_matrix = glm::mat4(1.0f);
            (*dpool)[d]->m_matrix = m_world;
            (*dpool)[d]->m_type = DType::Instance;
            (*dpool)[d]->m_instance_task = [=](VkCommandBuffer _cb){
                ShatterBuffer* buffer = SingleBPool.getBuffer(tool::combine(tool::combine("AnimationGroup",i),m_id),Buffer_Type::Vertex_Buffer);
                VkViewport tmp = getViewPort();
                vkCmdSetViewport(_cb,0,1,&tmp);

                VkRect2D scissor = getScissor();

                vkCmdSetScissor(_cb,0,1,&scissor);

                vkCmdBindPipeline(_cb, //将当前使用的命令缓冲与指定管线绑定
                                  VK_PIPELINE_BIND_POINT_GRAPHICS, PPool::getPool()["Animation"]->getPipeline());
                auto set_pool = MPool<VkDescriptorSet>::getPool();
                std::vector<VkDescriptorSet> sets{(*(*set_pool)[mc_index]),
                                                  SingleSetPool["Camera"],
                                                  SingleSetPool["AnimationUniform"],
                                                  m_sets[group->materialIndex]};
                vkCmdBindDescriptorSets(_cb,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        PPool::getPool()["Animation"]->getPipelineLayout(),
                                        0,
                                        sets.size(),
                                        sets.data(),
                                        0,
                                        VK_NULL_HANDLE);
                const VkDeviceSize offsetsVertex[1] = { 0 }; //顶点数据偏移量数组
                VkBuffer vb = (*buffer)();
                vkCmdBindVertexBuffers(_cb, //将当前组顶点数据与当前使用的命令缓冲绑定
                                       0, 1, &vb, offsetsVertex);
                vkCmdDraw(_cb, triangleCount * 3, 1, 0, 0); //绘制当前组
            };
            insertDObject(d);
        }
    }

    void Animation::initAnimation(){
        int mat4_count = 0;
        int vec4_count = 0;
        int vec3_count = 0;
        int vec2_count = 0;

        int mat4_index = 0;
        int vec4_index = 0;
        int vec3_index = 0;
        int vec2_index = 0;

        int binaryData_index = 0; //数据读取辅助索引（以字节计）
        constructHeader(m_binary_data,&binaryData_index,m_header);//创建ms3d 文件头信息对象
        m_vertex_count = file::myReadUnsignedShort(m_binary_data, &binaryData_index); //读取顶点数量
        m_vertexes.resize(m_vertex_count);
        vec3_count += m_vertex_count * 2;
        int vertexData_index = binaryData_index;

        binaryData_index += m_vertex_count * AVertexSize;
        int num_tri = file::myReadUnsignedShort(m_binary_data, &binaryData_index); //读取三角形面数量
        m_triangles.resize(num_tri);
        vec2_count += num_tri * 3;
        vec3_count += num_tri * 3;
        int triangleData_index = binaryData_index;

        binaryData_index += num_tri * ATriangleSize;
        m_group_count = file::myReadUnsignedShort(m_binary_data, &binaryData_index); //读取组数量
        m_groups.resize(m_group_count);
        int groupData_index = binaryData_index;
        int tmp;
        for (int i = 0; i < m_group_count; i++)
        {
            binaryData_index += 33;
            tmp = file::myReadUnsignedShort(m_binary_data, &binaryData_index);//读取组内三角形数量
            binaryData_index += tmp * 2;
            binaryData_index += 1;
        }

        int num_mat = file::myReadUnsignedShort(m_binary_data, &binaryData_index); //读取材质数量
        m_materials.resize(num_mat);
        vec4_count += num_mat * 4;
        int materialData_index = binaryData_index;

        binaryData_index += num_mat * AMaterialSize;
        m_fps = file::myReadFloat(m_binary_data, &binaryData_index); //读取帧速率信息
        m_current_time = file::myReadFloat(m_binary_data, &binaryData_index); //读取当前时间
        m_frame_count = file::myReadInt(m_binary_data, &binaryData_index); //读取关键帧总数
        m_total_time = m_frame_count / m_fps; //计算动画总时间
        m_joint_count = file::myReadUnsignedShort(m_binary_data, &binaryData_index); //读取关节数量
        m_joints.resize(m_joint_count);
        std::unordered_map<std::string, Joint*> mapp; //用于保存关节id 与关节信息对象对应关系的临时map
        int jointData_index = binaryData_index;
        int rot_count;
        int tran_count;
        for (int i = 0; i < m_joint_count; i++)
        {
            binaryData_index += 89;
            vec3_count += 2;
            mat4_count += 2;
            mat4_count += 2;
            rot_count = file::myReadUnsignedShort(m_binary_data, &binaryData_index);
            tran_count = file::myReadUnsignedShort(m_binary_data, &binaryData_index);
            vec4_count += rot_count;
            vec4_count += 1;
            vec3_count += tran_count;
            vec3_count += 1;
            binaryData_index += rot_count * AKeyFrameRotateSize;
            binaryData_index += tran_count * AKeyFrameTransitionSize;
        }

        m_vec2Pool = new BigPool<Avec2>(vec2_count);
        m_vec3Pool = new BigPool<Avec3>(vec3_count);
        m_vec4Pool = new BigPool<Avec4>(vec4_count);
        m_mat4Pool = new BigPool<Amat4>(mat4_count);

        for(auto &i: m_vertexes)
        {
            constructVertex(m_binary_data,&vertexData_index,i,vec3_index,this);
        }

        for(auto &i : m_triangles)
        {
            constructTriangle(m_binary_data,&triangleData_index,i,vec2_index,vec3_index,this);
        }

        for(auto &i : m_groups)
        {
            constructGroup(m_binary_data,&groupData_index,i);
        }

        for(auto &i : m_materials)
        {
            constructMaterial(m_binary_data,&materialData_index,i,vec4_index,this);
        }
        m_sets.resize(m_materials.size());
        SingleSetPool.AllocateDescriptorSets(std::vector<Set_id>(m_sets.size(),"AnimationTexture"),m_sets.data());

//        std::array<VkWriteDescriptorSet,m_sets.size()> mc_write = {};
        std::vector<VkWriteDescriptorSet> set_write(m_sets.size());
        VkWriteDescriptorSet tmp_set_write;
        tmp_set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        tmp_set_write.pNext = VK_NULL_HANDLE;
        tmp_set_write.dstBinding = 0;
        tmp_set_write.dstArrayElement = 0;
        tmp_set_write.descriptorCount = 1;
        tmp_set_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        tmp_set_write.pBufferInfo = VK_NULL_HANDLE;

        for (int i = 0; i < m_materials.size(); ++i) {
            tmp_set_write.dstSet = m_sets[i];
            SingleBPool.getTexture(m_materials[i].textureName)->genImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            tmp_set_write.pImageInfo = &SingleBPool.getTexture(m_materials[i].textureName)->m_ImageInfo;
            set_write[i] = tmp_set_write;
        }
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = (*SingleBPool.getBuffer("AnimationUniform",Buffer_Type::Uniform_Buffer))();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(float);
        tmp_set_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        tmp_set_write.dstSet = SingleSetPool["AnimationUniform"];
        tmp_set_write.pImageInfo = VK_NULL_HANDLE;
        tmp_set_write.pBufferInfo = &bufferInfo;

        set_write.push_back(tmp_set_write);

        vkUpdateDescriptorSets(Device::getDevice()(),
                               set_write.size(),
                               set_write.data(),
                               0,
                               nullptr);

        Joint* tmp_joint;
        for(int i = 0; i < m_joint_count; i++)
        {
            tmp_joint = new Joint;
            constructJoint(m_binary_data, &jointData_index,*tmp_joint,vec3_index,vec4_index,mat4_index,this);
            mapp[tmp_joint->m_name] = tmp_joint;
            for(auto &[name,joint] : mapp)
            {
                if(tmp_joint->m_parent_name == name){
                    tmp_joint->m_parent = joint;
                    tmp_joint->m_isolate = false;
                    break;
                }
            }
            loadIdentity(tmp_joint->m_init_relative,this);
            genRotationFromEulerAngle(tmp_joint->m_init_rotation,tmp_joint->m_init_relative,this);
            setTransition(tmp_joint->m_init_relative,tmp_joint->m_init_transition,this);
            loadIdentity(tmp_joint->m_init_absolute,this);
            if(!tmp_joint->m_isolate){
//                multiple(tmp_joint->m_parent->m_init_absolute,tmp_joint->m_parent->m_init_relative,tmp_joint->m_parent->m_init_absolute);
                multiple(tmp_joint->m_parent->m_init_absolute,tmp_joint->m_init_relative,tmp_joint->m_init_absolute,this);
            }
            else {
                m_mat4Pool->copy(tmp_joint->m_init_absolute,tmp_joint->m_init_relative);
            }
            m_joints[i] = tmp_joint;
        }
    }

    void Animation::initVertexData(){
        updateJoint(0.0f); //将关节更新到起始时间（时间为0 的时间）
        int triangleCount = 0; //表示组内三角形个数的辅助变量
        Triangle *triangle; //指向三角形组装信息对象的辅助指针
        int *indexs; //指向组内三角形索引数组首地址的指针
        int *vertexIndexs; //指向当前处理三角形顶点索引数组首地址的指针
        glm::vec3 position;
        glm::vec2 uv;
        auto pool = m_vec3Pool;
        auto uvPool = m_vec2Pool;
        for (auto &group : m_groups) { //对模型中的每个组进行遍历
            int count = 0; //辅助索引
            indexs = group.indices; //获取组内三角形索引数组
            triangleCount = group.count_ind; //获取组内三角形数量
            float *groupVdata = new float[triangleCount * 3 * 5]; //初始化本组顶点数据数组
            for (int j = 0; j < triangleCount; j++) { //遍历组内的每个三角形
                triangle = &m_triangles[indexs[j]]; //获取当前要处理的三角形
                vertexIndexs = triangle->index; //获取当前三角形的顶点索引数组
                for (int k = 0; k < 3; k++) { //循环对三角形中的3 个顶点进行处理
                    Vertex *mvt = &m_vertexes[vertexIndexs[k]]; //根据顶点索引获取当前顶点
                    pool->get(mvt->initPosition,position);
                    groupVdata[count++] = position.x; //顶点位置的X 坐标
                    groupVdata[count++] = position.y; //顶点位置的Y 坐标
                    groupVdata[count++] = position.z; //顶点位置的Z 坐标

                    uvPool->get(triangle->uv[k],uv);
                    groupVdata[count++] = uv.s;//顶点的纹理S 坐标
                    groupVdata[count++] = uv.t;//顶点的纹理T 坐标
                }
            }
            m_vertex_data.push_back(groupVdata); //将当前组的顶点数据添加进总数据列表
        }
    }

    void Animation::initVertexBuffer(){
        Group* group;
        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.offset = 0;
        for (int i = 0; i < m_groups.size();i++)
        {
            group = &m_groups[i];
            int triangleCount  = group->count_ind;
            int dataByteCount = triangleCount * 3 * 5 * sizeof(float);
            SingleBPool.createVertexHostBuffer(tool::combine(tool::combine("AnimationGroup",i),m_id),dataByteCount,m_vertex_data[i]);

            bufferInfo.buffer = (*SingleBPool.getBuffer(tool::combine(tool::combine("AnimationGroup",i),m_id),Buffer_Type::Vertex_Buffer))();
            bufferInfo.range = dataByteCount;

            m_vertex_buffer_info.push_back(bufferInfo);
        }
    }

    void Animation::updateJoint(float _time){
        int circle = int(_time / m_total_time);
        m_current_time = _time - circle * m_total_time; //设置当前时间
        if (m_current_time > m_total_time) { //若当前动画时间超过总动画时间
            m_current_time = 0.0f; //设置当前时间为0
        }
        for (auto& jt : m_joints) { //对模型中的每个关节进行遍历
            jt->update(m_current_time); //调用当前关节的更新方法
        }
    }

    void Animation::updateAllVertexes(){
        Joint* joint;
        for (auto vtHelper : m_vertexes) { //对模型中的所有顶点进行遍历
            if (vtHelper.bone == -1) { //若无关节控制
                m_vec3Pool->copy(vtHelper.currPosition,vtHelper.initPosition);//初始位置即为当前位置
            }
            else { //若有关节控制
                int it = vtHelper.bone; //获取对应关节索引
                joint = m_joints[it]; //获取对应的关节信息对象
                invTranAndRotate(joint->m_init_absolute,vtHelper.initPosition,joint->m_transition_help,this);//获取顶点在关节坐标系中的初始坐标
                transform(joint->m_real_abs/*mat4*/, joint->m_transition_help,vtHelper.currPosition/*vec3*/,this);
            }
        }
    }

    void Animation::animate(float _time){
        updateJoint(_time); //更新关节数据
        updateAllVertexes(); //更新顶点数据
        reAssemVertexData(); //重新组装数据
        updateBuffer();    //更新缓冲
    }

    void Animation::updateBuffer(){
        Group* group; //临时辅助用组信息对象指针
        for (int i = 0; i < m_groups.size(); i++) { //对模型中的所有组进行遍历
            ShatterBuffer* buffer = SingleBPool.getBuffer(tool::combine(tool::combine("AnimationGroup",i),m_id),Buffer_Type::Vertex_Buffer);
            group = &m_groups[i]; //获取当前组信息对象
            int triangleCount = group->count_ind; //获取组内三角形的数量
            float* groVdata = m_vertex_data[i]; //获取当前组顶点数据数组
            uint8_t *pData; //CPU 访问时的辅助指针
            VkResult result = vkMapMemory(SingleDevice(), //将设备内存映射为CPU可访问
                                          buffer->getMemory(),
                                          0,
                                          m_vertex_buffer_info[i].range,
                                          0,
                                          (void **)&pData);
            assert(result == VK_SUCCESS); //判断内存映射是否成功
            memcpy(pData, groVdata, triangleCount * 3 * 5 * sizeof(float)); //将数据拷贝进设备内存
            vkUnmapMemory(SingleDevice(), buffer->getMemory()); //解除内存映射
        }
    }

    void Animation::reAssemVertexData() //更新顶点数据后重新组装数据的方法
    {
        int triangleCount = 0;
        Group* group;
        Triangle* triangle;
        Vertex* vertex;
        int* indexs;
        int* vertexIndexs;
        glm::vec3 position;
        glm::vec2 uv;
        auto pool = m_vec3Pool;
        auto uvPool = m_vec2Pool;
        for(int i = 0; i < m_group_count; i++)
        {
            int count=0;
            group = &m_groups[i];
            triangleCount = group->count_ind;
            indexs = group->indices;
            float *groupVdata = m_vertex_data[i];
            for(int j=0;j<triangleCount;j++)
            {
                triangle = &m_triangles[indexs[j]];
                vertexIndexs = triangle->index;
                for(int k=0; k<3; k++)
                {
                    vertex=&m_vertexes[vertexIndexs[k]];
                    pool->get(vertex->currPosition,position);
                    groupVdata[count++] = position.x; //顶点位置的X 坐标
                    groupVdata[count++] = position.y; //顶点位置的Y 坐标
                    groupVdata[count++] = position.z; //顶点位置的Z 坐标
//                    std::cout << std::fixed << "debug position x:" << position.x << "y:" << position.y << "z:" << position.z << std::endl;

                    uvPool->get(triangle->uv[k],uv);
//                    std::cout << std::fixed << "debug uv s:" << uv.s << "t:" << uv.t << std::endl;
                    groupVdata[count++] = uv.s;//顶点的纹理S 坐标
                    groupVdata[count++] = uv.t;//顶点的纹理T 坐标
                }
            }
        }
    }

    Animation::~Animation() {
        std::for_each(m_joints.begin(), m_joints.end(),[](Joint* joint){
            delete joint;
        });

        std::for_each(m_vertex_data.begin(), m_vertex_data.end(), [](float* data){
            delete data;
        });

        delete m_vec2Pool;
        delete m_vec3Pool;
        delete m_vec4Pool;
        delete m_mat4Pool;
    }
}