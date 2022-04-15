//
// Created by maybe on 2021/6/2.
//
#include "precompiledhead.h"

#include "shatter_math.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Object/aabb.h"
#include <random>
#include "Engine/Object/camera.h"
#include "Engine/Pool/mpool.h"
#include "Engine/Object/inputaction.h"
#include "Engine/App/shatterapp.h"

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
    float dot = 0, s1, s2, om, sinom;
#ifdef SHATTER_SIMD
    const auto& inA = reinterpret_cast<const __m128&>(*_inA);
    const auto& inB = reinterpret_cast<const __m128&>(*_inB);
    __m128 mul = _mm_mul_ps(inA, inB);
    dot = mul[0] + mul[1] + mul[2] + mul[3];

    __m128 C = inA;
    if (dot < 0) {
        C = -inA;
        dot = -dot;
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

    float simdS1[4] = {s1,s1,s1,s1};
    float simdS2[4] = {s2,s2,s2,s2};
    __m128 S1 = _mm_load_ps(simdS1);
    __m128 S2 = _mm_load_ps(simdS2);
    __m128 mul1 = _mm_mul_ps(C, S1);
    __m128 mul2 = _mm_mul_ps(inB, S2);
    __m128 add = _mm_add_ps(mul1, mul2);
    *_out = reinterpret_cast<glm::vec4&>(add);
#else
    for (int i = 0; i < 4; i++) {
        dot += (*_inA)[i] * (*_inB)[i];
    }

    glm::vec4 C;
    if (dot < 0) {
        C = -(*_inA);
        dot = -dot;
    } else {
        C = *_inA;
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
#endif
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
    assert(glm::length(_f) >= float_limit);
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
    auto c = glm::cross(a,_z);
    _x = c / glm::length(c);
    _y = glm::cross(_z,_x);
}

void genFaceVertexBufferFromAABB(const AABB& _box, std::vector<glm::vec3>& _buffer){
    glm::vec3 minPos = _box.m_min_edgy;
    glm::vec3 maxPos = _box.m_max_edgy;
    //bottom
    _buffer.emplace_back(minPos);
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, minPos.z));

    _buffer.emplace_back(minPos);
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, minPos.z));

    //top
    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));

    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, maxPos.z));

    //left
    _buffer.emplace_back(minPos);
    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, maxPos.z));

    _buffer.emplace_back(minPos);
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, minPos.z));

    //right
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));

    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, minPos.z));

    //back
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));

    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, minPos.z));

    //front
    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, maxPos.z));

    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, minPos.z));
}

void genLineVertexBuffer(const glm::vec3& _min, const glm::vec3& _max, std::vector<glm::vec3>& _buffer)
{
    glm::vec3 minPos = _min;
    glm::vec3 maxPos = _max;
    //bottom
    _buffer.emplace_back(minPos);
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, minPos.z));

    _buffer.emplace_back(minPos);
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, minPos.z));

    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, minPos.z));

    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, minPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, minPos.z));

    //top
    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, maxPos.z));

    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, maxPos.z));

    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, maxPos.z));

    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, maxPos.z));

    //other
    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, minPos.y, minPos.z));

    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(minPos.x, maxPos.y, minPos.z));

    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, minPos.y, minPos.z));

    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, maxPos.z));
    _buffer.emplace_back(glm::vec3(maxPos.x, maxPos.y, minPos.z));
}

void genPlane(const glm::vec3& _min, const glm::vec3& _max, NPlane& _plane){
    new ((void*)&_plane.points[0])NPlane::Point{_min, {.0f,.0f}};
    new ((void*)&_plane.points[1])NPlane::Point{{_max.x, _min.y, _min.z}, {1.0f,.0f}};
    new ((void*)&_plane.points[2])NPlane::Point{_max, {1.0f,1.0f}};
    new ((void*)&_plane.points[3])NPlane::Point{{_min.x, _max.y, _min.z}, {.0f,1.0f}};
}

void genCube(const glm::vec2& _min, const glm::vec2& _max, float _height, Cube& _cube)
{
    /*
     * front
     */
    new ((void*)&_cube.planes[int(CubePlane::Front)].points[0]) Cube::Plane::Point{glm::vec3{_max.x, _max.y, .0f},
                                                                       glm::vec3{.0f, 1.0f, .0f},
                                                                       glm::vec2{0.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Front)].points[1]) Cube::Plane::Point{glm::vec3{_min.x, _max.y, .0f},
                                                                       glm::vec3{.0f, 1.0f, .0f},
                                                                       glm::vec2{1.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Front)].points[2]) Cube::Plane::Point{glm::vec3{_min.x, _max.y, _height},
                                                                       glm::vec3{.0f, 1.0f, .0f},
                                                                       glm::vec2{1.0f, 1.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Front)].points[3]) Cube::Plane::Point{glm::vec3{_max.x, _max.y, _height},
                                                                       glm::vec3{.0f, 1.0f, .0f},
                                                                       glm::vec2{0.0f, 1.0f}};

    /*
    * back
    */
    new ((void*)&_cube.planes[int(CubePlane::Back)].points[0]) Cube::Plane::Point{glm::vec3{_min.x, _min.y, .0f},
                                                                       glm::vec3{.0f, -1.0f, .0f},
                                                                       glm::vec2{0.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Back)].points[1]) Cube::Plane::Point{glm::vec3{_max.x, _min.y, .0f},
                                                                       glm::vec3{.0f, -1.0f, .0f},
                                                                       glm::vec2{1.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Back)].points[2]) Cube::Plane::Point{glm::vec3{_max.x, _min.y, _height},
                                                                       glm::vec3{.0f, -1.0f, .0f},
                                                                       glm::vec2{1.0f, 1.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Back)].points[3]) Cube::Plane::Point{glm::vec3{_min.x, _min.y, _height},
                                                                       glm::vec3{.0f, -1.0f, .0f},
                                                                       glm::vec2{0.0f, 1.0f}};

    /*
    * left
    */
    new ((void*)&_cube.planes[int(CubePlane::Left)].points[0]) Cube::Plane::Point{glm::vec3{_min.x, _max.y, .0f},
                                                                      glm::vec3{-1.0f, .0f, .0f},
                                                                      glm::vec2{0.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Left)].points[1]) Cube::Plane::Point{glm::vec3{_min.x, _min.y, .0f},
                                                                      glm::vec3{-1.0f, .0f, .0f},
                                                                      glm::vec2{1.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Left)].points[2]) Cube::Plane::Point{glm::vec3{_min.x, _min.y, _height},
                                                                      glm::vec3{-1.0f, .0f, .0f},
                                                                      glm::vec2{1.0f, 1.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Left)].points[3]) Cube::Plane::Point{glm::vec3{_min.x, _max.y, _height},
                                                                      glm::vec3{-1.0f, .0f, .0f},
                                                                      glm::vec2{0.0f, 1.0f}};

    /*
    * right
    */
    new ((void*)&_cube.planes[int(CubePlane::Right)].points[0]) Cube::Plane::Point{glm::vec3{_max.x, _min.y, .0f},
                                                                      glm::vec3{1.0f, .0f, .0f},
                                                                      glm::vec2{0.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Right)].points[1]) Cube::Plane::Point{glm::vec3{_max.x, _max.y, .0f},
                                                                      glm::vec3{1.0f, .0f, .0f},
                                                                      glm::vec2{1.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Right)].points[2]) Cube::Plane::Point{glm::vec3{_max.x, _max.y, _height},
                                                                      glm::vec3{1.0f, .0f, .0f},
                                                                      glm::vec2{1.0f, 1.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Right)].points[3]) Cube::Plane::Point{glm::vec3{_max.x, _min.y, _height},
                                                                      glm::vec3{1.0f, .0f, .0f},
                                                                      glm::vec2{0.0f, 1.0f}};

    /*
    * bottom
    */
    new ((void*)&_cube.planes[int(CubePlane::Top)].points[0]) Cube::Plane::Point{glm::vec3{_max.x, _min.y, .0f},
                                                                      glm::vec3{.0f, .0f, -1.0f},
                                                                      glm::vec2{0.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Top)].points[1]) Cube::Plane::Point{glm::vec3{_min.x, _min.y, .0f},
                                                                     glm::vec3{.0f, .0f, -1.0f},
                                                                     glm::vec2{1.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Top)].points[2]) Cube::Plane::Point{glm::vec3{_min.x, _max.y, .0f},
                                                                     glm::vec3{.0f, .0f, -1.0f},
                                                                     glm::vec2{1.0f, 1.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Top)].points[3]) Cube::Plane::Point{glm::vec3{_max.x, _max.y, .0f},
                                                                     glm::vec3{.0f, .0f, -1.0f},
                                                                     glm::vec2{0.0f, 1.0f}};

    /*
    * top
    */
    new ((void*)&_cube.planes[int(CubePlane::Bottom)].points[0]) Cube::Plane::Point{glm::vec3{_min.x, _min.y, _height},
                                                                        glm::vec3{.0f, .0f, 1.0f},
                                                                        glm::vec2{0.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Bottom)].points[1]) Cube::Plane::Point{glm::vec3{_max.x, _min.y, _height},
                                                                        glm::vec3{.0f, .0f, 1.0f},
                                                                        glm::vec2{1.0f, 0.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Bottom)].points[2]) Cube::Plane::Point{glm::vec3{_max.x, _max.y, _height},
                                                                        glm::vec3{.0f, .0f, 1.0f},
                                                                        glm::vec2{1.0f, 1.0f}};

    new ((void*)&_cube.planes[int(CubePlane::Bottom)].points[3]) Cube::Plane::Point{glm::vec3{_min.x, _max.y, _height},
                                                                        glm::vec3{.0f, .0f, 1.0f},
                                                                        glm::vec2{0.0f, 1.0f}};
}

void computeLocalCoordinate(glm::vec3& _coordinate, int _targetPlane) {
    Target* target = (*MPool<Target>::getPool())[_targetPlane];
    glm::vec3& ray = input::getCursorRay();
    float height = glm::dot(SingleCamera.getPos() - target->center, target->plane.z_coordinate);
    float angle = glm::dot(target->plane.z_coordinate, -ray);
    glm::vec3 pos = SingleCamera.getPos() + (1.0f / angle) * height * ray;
    _coordinate = glm::vec3(1.0f, 0.0f, 0.0f) * glm::dot(target->plane.x_coordinate, pos - target->center) +
                  glm::vec3(0.0f, 1.0f, 0.0f) * glm::dot(target->plane.y_coordinate, pos - target->center);
}

void computeLocalCoordinate(glm::vec3& _coordinate) {
    glm::vec3& ray = input::getCursorRay();
    float height = glm::dot(SingleCamera.getPos() - SingleAPP.getWorkTargetCenter(), SingleAPP.getWorkTargetPlane().z_coordinate);
    float angle = glm::dot(SingleAPP.getWorkTargetPlane().z_coordinate, -ray);
    glm::vec3 pos = SingleCamera.getPos() + (1.0f / angle) * height * ray;
    _coordinate = glm::vec3(1.0f, 0.0f, 0.0f) * glm::dot(SingleAPP.getWorkTargetPlane().x_coordinate, pos - SingleAPP.getWorkTargetCenter()) +
                  glm::vec3(0.0f, 1.0f, 0.0f) * glm::dot(SingleAPP.getWorkTargetPlane().y_coordinate, pos - SingleAPP.getWorkTargetCenter());
}

float computeHeight(const glm::vec3& _center) {
    glm::vec3& ray = input::getCursorRay();
    glm::vec3 z_axis = glm::cross(SingleCamera.m_targetPlane.x_coordinate, ray);
    return glm::dot(z_axis, (SingleCamera.center + SingleCamera.eye) - _center);
}

void decomposeTransform(const glm::mat4 &_transform, glm::vec3 &_transition, glm::vec3 &_rotation,  glm::vec3 &_scale) {
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

#ifdef SHATTER_SIMD
void SIMDMatrixMultipleVec(__m128 matrix[4],__m128 vector){

}
#endif

