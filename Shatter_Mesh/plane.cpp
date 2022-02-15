//
// Created by jonnxie on 2022/1/28.
//

#include "plane.h"
#include "base.h"
#include "../Shatter_Item/shatter_enum.h"
#include "../Shatter_Base/lines.h"
#include "../Shatter_Base/tris.h"
#include "../Shatter_Object/shatter_math.h"

std::ostream & operator<<(std::ostream& _in,const Plane& _plane)
{
    _in << "[Plane]" << _plane.n.x << " * x + "
                    << _plane.n.y << " * y + "
                    << _plane.n.z << " * z = "
                    << _plane.d << "\n";
    return _in;
}

Plane::Plane(const vec3 &_point, const vec3 &_normal) {
    setPlane(_point, _normal);
    assert(fabs(operator[](_point)) < 1e-10);
}

Plane::Plane(const std::vector<vec3>& _samples) {
    vec3 c(0,0,0);
    for(auto point : _samples) c += point;
    c /= static_cast<float>(_samples.size());

    float xx = 0.0; float yy = 0.0;
    float xy = 0.0; float yz = 0.0;
    float xz = 0.0; float zz = 0.0;

    for(auto point : _samples)
    {
        vec3 pc = point - c;
        xx += pc.x * pc.x;
        xy += pc.x * pc.y;
        xz += pc.x * pc.z;
        yy += pc.y * pc.y;
        yz += pc.y * pc.z;
        zz += pc.z * pc.z;
    }

    float det_x   = yy*zz - yz*yz;
    float det_y   = xx*zz - xz*xz;
    float det_z   = xx*yy - xy*xy;
    float det_max = std::max(det_x, std::max(det_y, det_z));

    //if(fabs(det_max) <= 1e-5) std::cerr << "WARNING : the samples don't span a plane!" << std::endl;

    vec3 normal;
    if (det_max == det_x) normal = vec3(1.0, (xz*yz - xy*zz) / det_x, (xy*yz - xz*yy) / det_x); else
    if (det_max == det_y) normal = vec3((yz*xz - xy*zz) / det_y, 1.0, (xy*xz - yz*xx) / det_y); else
    if (det_max == det_z) normal = vec3((yz*xy - xz*yy) / det_z, (xz*xy - yz*xx) / det_z, 1.0);
    else assert(false);

    setPlane(c,normal);
}

void Plane::draw(){
    std::vector<Line> lines{
            {
                    {p,
                     color},
                    {p + n,
                     color}
            }
    };
    line = new DLines(lines);
    line->init();

    Point x0{},x1{},y0{},y1{},z0{},z1{};

    glm::vec3 x,y,z;
    genLocalCoordinateFromZ(glm::vec3(n),x,y,z);
    Point rt {glm::vec3(p) + x + y,color};
    Point rl {glm::vec3(p) + x - y,color};
    Point lt {glm::vec3(p) - x + y,color};
    Point ll {glm::vec3(p) - x - y,color};

//    if(fabs(n.x) > 1e-10 & fabs(n.y) > 1e-10 & fabs(n.z) > 1e-10)
//    {
//        x0 = {normalize(vec3(0,-n.z,n.y)) + p,color};
//        x1 = {normalize(vec3(0,n.z,-n.y)) + p,color};
//
//        y0 = {normalize(vec3(-n.z,0,n.x)) + p,color};
//        y1 = {normalize(vec3(n.z,0,-n.x)) + p,color};
//
//        z0 = {normalize(vec3(-n.y,n.x,0)) + p,color};
//        z1 = {normalize(vec3(n.y,-n.x,0)) + p,color};
//    }

    Tri t0 = {
            rt,
            rl,
            lt
    };

    Tri t1 = {
            rl,
            ll,
            lt
    };

    std::vector<Tri> tris{
            t0,
            t1
    };
    tri = new DTris(tris, MeshDrawType::Face);
    tri->init();
}

void Plane::setPlane(const vec3 &_point, const vec3 &_normal) {
    if(isNan(_point) || isInf(_point) || isDeg(_normal))
    {
        p = vec3(0,0,0);
        n = vec3(0,0,0);
        return;
    }
    p = _point;
    n = _normal;

    n = normalize(n);
    d = dot(n,_point);
    assert(fabs(operator[](_point)) < 1e-10);
}

Plane::Plane(const vec3 &_p0, const vec3 &_p1, const vec3 &_p2) {
    vec3 u = _p1 - _p0;
    vec3 v = _p2 - _p0;
    setPlane(_p0, cross(u,v));
    assert(fabs(operator[](_p0)) < 1e-10);
    assert(fabs(operator[](_p1)) < 1e-10);
    assert(fabs(operator[](_p2)) < 1e-10);
}

float Plane::operator[](const vec3 &_p) const {
    return dot(n,_p) - d;
}

float Plane::pointPlaneDistSigned(const vec3 &_p) const {
    assert(fabs(length(n)-1.0) < 1e-10);
    vec3 u = _p - this->p;
    return dot(u,n);
}

float Plane::pointPlaneDist(const vec3 &_p) const {
    return std::fabs(pointPlaneDistSigned(p));
}

vec3 Plane::projectOnto(const vec3 &_p) const {
    vec3 res = _p - n * pointPlaneDistSigned(_p);
    //auto  err = point_plane_dist(res);
    //assert(err < 1e-7);
    return res;
}

Plane::~Plane() {
    delete line;
    delete tri;
}

DrawPlane::DrawPlane() {
    m_action[Event::SingleClick] = [&]() {
        static bool draw = false;
        static glm::vec3 pre_pos;
        if (draw) {
            glm::vec3 pos;
            input::cursor(pos, STATE_OUT);
            auto plane = std::make_unique<Plane>(pre_pos,pos - pre_pos);
            plane->draw();
            planes.push_back(std::move(plane));
            draw = false;
            SingleRender.normalChanged = true;
        } else {
            input::cursor(pre_pos, STATE_OUT);
            draw = true;
        };
    };
}






