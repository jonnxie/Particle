//
// Created by maybe on 2021/6/2.
//
#include "precompiledhead.h"

#include "shatter_math.h"
#include "Engine/Item/shatter_enum.h"
#include <random>

std::default_random_engine rndEngine;

float genRandom(float _range){
     std::uniform_real_distribution<float> rndDist(0.0f, _range);
     return rndDist(rndEngine);
}

void genRotateFromQuaternion(const glm::vec4 *_in, glm::mat4 *_out) {
    float x=(*_in)[0];
    float y=(*_in)[1];
    float z=(*_in)[2];
    float w=(*_in)[3];

    (*_out)[0][0] = (1.0f - 2.0f * y * y - 2.0f * z * z);
    (*_out)[0][1] = (2.0f * (x * y + w * z));
    (*_out)[0][2] = (2.0f * (x * z - w * y));
    (*_out)[0][3] = 0.0f;

    (*_out)[1][0] = (2.0f * (x * y - w * z));
    (*_out)[1][1] = (1.0f - 2.0f * x * x - 2.0f * z * z);
    (*_out)[1][2] = (2.0f * (y * z + w * x));
    (*_out)[1][3] = 0.0f;

    (*_out)[2][0] = (2.0f * (x * z + w * y));
    (*_out)[2][1] = (2.0f * (y * z - w * x));
    (*_out)[2][2] = (1.0f - 2.0f * x * x - 2.0f * y * y);
    (*_out)[2][3] = 0.0f;

    (*_out)[3][0] = 0.0f;
    (*_out)[3][1] = 0.0f;
    (*_out)[3][2] = 0.0f;
    (*_out)[3][3] = 1.0f;

//    (*_out)[0][0] = (1.0f - 2.0f * y * y - 2.0f * z * z);
//    (*_out)[1][0] = (2.0f * (x * y + w * z));
//    (*_out)[2][0] = (2.0f * (x * z - w * y));
//    (*_out)[3][0] = 0.0f;
//
//    (*_out)[0][1] = (2.0f * (x * y - w * z));
//    (*_out)[1][1] = (1.0f - 2.0f * x * x - 2.0f * z * z);
//    (*_out)[2][1] = (2.0f * (y * z + w * x));
//    (*_out)[3][1] = 0.0f;
//
//    (*_out)[0][2] = (2.0f * (x * z + w * y));
//    (*_out)[1][2] = (2.0f * (y * z - w * x));
//    (*_out)[2][2] = (1.0f - 2.0f * x * x - 2.0f * y * y);
//    (*_out)[3][2] = 0.0f;
//
//    (*_out)[0][3] = 0.0f;
//    (*_out)[1][3] = 0.0f;
//    (*_out)[2][3] = 0.0f;
//    (*_out)[3][3] = 1.0f;

}

void genRotateFromEulerAngle(glm::vec3* _angle,glm::mat4* _matrix){
    float yaw = (*_angle)[0];
    float pitch = (*_angle)[1];
    float roll = (*_angle)[2];
    double cr = cos(yaw);
    double sr = sin(yaw);
    double cp = cos(pitch);
    double sp = sin(pitch);
    double cy = cos(roll);
    double sy = sin(roll);
    double srsp = sr * sp;
    double crsp = cr * sp;
    setIdentity(_matrix);
//    (*_matrix)[0][0] = (float) (cp * cy);
//    (*_matrix)[0][1] = (float) (cp * sy);
//    (*_matrix)[0][2] = (float) (-sp);
//    (*_matrix)[0][1] = (float) (srsp * cy - cr * sy);
//    (*_matrix)[1][1] = (float) (srsp * sy + cr * cy);
//    (*_matrix)[2][1] = (float) (sr * cp);
//    (*_matrix)[0][2] = (float) (crsp * cy + sr * sy);
//    (*_matrix)[1][2] = (float) (crsp * sy - sr * cy);
//    (*_matrix)[2][2] = (float) (cr * cp);
    (*_matrix)[0][0] = (float) (cp * cy);
    (*_matrix)[0][1] = (float) (cp * sy);
    (*_matrix)[0][2] = (float) (-sp);
    (*_matrix)[1][0] = (float) (srsp * cy - cr * sy);
    (*_matrix)[1][1] = (float) (srsp * sy + cr * cy);
    (*_matrix)[1][2] = (float) (sr * cp);
    (*_matrix)[2][0] = (float) (crsp * cy + sr * sy);
    (*_matrix)[2][1] = (float) (crsp * sy - sr * cy);
    (*_matrix)[2][2] = (float) (cr * cp);
}

void interpolate(const glm::vec4 *_inA, const glm::vec4 *_inB, float _alpha, glm::vec4 *_out) {
    double dot = 0, s1, s2, om, sinom;
    for (int i = 0; i < 4; i++) {
        dot += (*_inA)[i] * (*_inB)[i];
    }
    glm::vec4 C;
    if (dot < 0) {
        C[0] = -(*_inA)[0];
        C[1] = -(*_inA)[1];
        C[2] = -(*_inA)[2];
        C[3] = -(*_inA)[3];
        dot = -dot;
    } else {
        C[0] = (*_inA)[0];
        C[1] = (*_inA)[1];
        C[2] = (*_inA)[2];
        C[3] = (*_inA)[3];
    }
    if (dot > 0.999999) {
        s1 = 1.0 - _alpha;
        s2 = _alpha;
    } else {
        om = acos(dot);
        sinom = sin(om);
        s1 = sin((1.0 - _alpha) * om) / sinom;
        s2 = sin(_alpha * om) / sinom;
    }
    (*_out)[0] = (float) (s1 * C[0] + s2 * (*_inB)[0]);
    (*_out)[1] = (float) (s1 * C[1] + s2 * (*_inB)[1]);
    (*_out)[2] = (float) (s1 * C[2] + s2 * (*_inB)[2]);
    (*_out)[3] = (float) (s1 * C[3] + s2 * (*_inB)[3]);
}

void interpolate(const glm::vec3 *_inA, const glm::vec3 *_inB, float _alpha, glm::vec3 *_out) {
    (*_out)[0] = (1 - _alpha) * (*_inA)[0] + _alpha * (*_inB)[0];
    (*_out)[1] = (1 - _alpha) * (*_inA)[1] + _alpha * (*_inB)[1];
    (*_out)[2] = (1 - _alpha) * (*_inA)[2] + _alpha * (*_inB)[2];
}

void interpolate(const glm::vec2* _inA,const glm::vec2* _inB,float _alpha,glm::vec2* _out){
    (*_out)[0] = (1 - _alpha) * (*_inA)[0] + _alpha * (*_inB)[0];
    (*_out)[1] = (1 - _alpha) * (*_inA)[1] + _alpha * (*_inB)[1];
}

void setTranslate(glm::mat4 *_out, glm::vec3 *_transition) {
    (*_out)[3][0] = (*_transition)[0];
    (*_out)[3][1] = (*_transition)[1];
    (*_out)[3][2] = (*_transition)[2];
}

void setIdentity(glm::mat4* _matrix) {
    for(int i=0;i<16;i++)
    {
        (*_matrix) = glm::mat4(0.0f);
    }
    (*_matrix)[0][0] =  1.0f;
    (*_matrix)[1][1] =  1.0f;
    (*_matrix)[2][2] =  1.0f;
    (*_matrix)[3][3] =  1.0f;
}

void invTransformAndRotate(const glm::mat4 *_mat, const glm::vec3 *_in, glm::vec3 *_out) {
    float x = (*_in)[0] - (*_mat)[3][0];
    float y = (*_in)[1] - (*_mat)[3][1];
    float z = (*_in)[2] - (*_mat)[3][2];

    (*_out)[0] =  (*_mat)[0][0] * x + (*_mat)[0][1] * y + (*_mat)[0][2] * z;
    (*_out)[1] =  (*_mat)[1][0] * x + (*_mat)[1][1] * y + (*_mat)[1][2] * z;
    (*_out)[2] =  (*_mat)[2][0] * x + (*_mat)[2][1] * y + (*_mat)[2][2] * z;
}

void setFromEulerAngleToQuaternion(const glm::vec3 &_angle, glm::vec4& _quaternion) {
    float angle = 0.0f;
    float sr, sp, sy, cr, cp, cy;
    angle = _angle[0] * 0.5f;
    sr = (float) sin(angle);
    cr = (float) cos(angle);
    angle = _angle[1] * 0.5f;
    sp = (float) sin(angle);
    cp = (float) cos(angle);
    angle = _angle[2] * 0.5f;
    sy = (float) sin(angle);
    cy = (float) cos(angle);
    _quaternion.x = (sr * cp * cy - cr * sp * sy);
    _quaternion.y = (cr * sp * cy + sr * cp * sy);
    _quaternion.z = (cr * cp * sy - sr * sp * cy);
    _quaternion.w = (cr * cp * cy + sr * sp * sy);
}

void genLocalCoordinateFromZ(const glm::vec3& _f,glm::vec3& _x,glm::vec3& _y,glm::vec3& _z) {
    assert(glm::length(_z) <= float_limit);
    _z = glm::normalize(_f);
    glm::vec3 a;
    if(_z.y >= float_limit || _z.z >= float_limit)
    {
        a = glm::vec3{1,0,0};
    }else if(_z.x >= float_limit || _z.z >= float_limit)
    {
        a = glm::vec3{0,1,0};
    }else if(_z.x >= float_limit || _z.y >= float_limit)
    {
        a = glm::vec3{0,0,1};
    }else{
        assert(0);
    }
    _x = glm::cross(a,_z) / glm::length(glm::cross(a,_z));
    _y = glm::cross(_z,_x);
}

void decomposeTransform(const glm::mat4 &_transform, glm::vec3 &_transition, glm::vec3 &_rotation, glm::vec3 &_scale) {
    glm::mat4 LocalMatrix(_transform);

    if(glm::abs(LocalMatrix[3][3]) < float_limit) { assert(0);}

    if(glm::abs(LocalMatrix[0][3]) >= float_limit ||
       glm::abs(LocalMatrix[1][3]) >= float_limit ||
       glm::abs(LocalMatrix[2][3]) >= float_limit)
    {
        LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[1][3] = 0.0f;
        LocalMatrix[3][3] = 1.0f;
    }

    _transition = glm::vec3(LocalMatrix[3]);
    LocalMatrix[3] = glm::vec4(0, 0, 0, LocalMatrix[3].w);

    glm::vec3 Row[3], Pdum3;

    for(size_t i = 0; i < 3; i++)
    {
        for(size_t j = 0; j < 3; j++)
        {
            Row[i][j] = LocalMatrix[i][j];
        }
    }

    _scale.x = glm::length(Row[0]);
    Row[0] = glm::normalize(Row[0]);
    _scale.y = glm::length(Row[1]);
    Row[1] = glm::normalize(Row[1]);
    _scale.z = glm::length(Row[2]);
    Row[2] = glm::normalize(Row[2]);

//    Pdum3 = glm::cross(Row[1], Row[2]);
//    if (glm::dot(Row[0], Pdum3) < 0)
//    {
//        for (size_t i = 0; i < 3; i++)
//        {
//            _scale[i] *= -1.0f;
//            Rowp[i] *= -1.0f;
//        }
//    }

    _rotation.y = asin(-Row[0][2]);
    if (cos(_rotation.y) != 0) {
        _rotation.x = atan2(Row[1][2], Row[2][2]);
        _rotation.z = atan2(Row[0][1], Row[0][0]);
    }
    else {
        _rotation.x = atan2(-Row[2][0], Row[1][1]);
        _rotation.z = 0;
    }

}