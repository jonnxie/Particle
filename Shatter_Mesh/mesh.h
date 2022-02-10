//
// Created by AnWell on 2022/2/8.
//

#ifndef CAD_MESH_H
#define CAD_MESH_H

#include <glm.hpp>
#include <vector>
#include "../Shatter_Object/listener.h"

using namespace glm;

class HalfEdge;

struct FlowVertex{
    vec3 pos;
    HalfEdge& edge;
};
class FlowFace;

struct HalfEdge{
    HalfEdge* opp{nullptr};
    FlowVertex   &end;
    FlowFace     &left;
    HalfEdge* next{nullptr};
};

struct FlowFace{
    HalfEdge* edge;
};

class Mesh{
public:
    void pushVertex(const FlowVertex& _v)
    {
        vertices.push_back(_v);
    }

    void pushEdge(HalfEdge& _h,HalfEdge& _g)
    {
        size_t hh = halfedges.size();
        halfedges.push_back(_h);
        size_t gg = halfedges.size();
        halfedges.push_back(_g);
        _h.opp = halfedges.data() + gg;
        _g.opp = halfedges.data() + hh;
    }

    void pushEdges(HalfEdge& _h){
        pushEdge(_h,*_h.opp);
    }

    void pushFace(FlowFace& _f){
        faces.push_back(_f);
    }

    void popVertex(){
        vertices.pop_back();
    }

    void popEdges(){
        halfedges.pop_back();
        halfedges.pop_back();
    }

    void popFace(){
        faces.pop_back();
    }

    void normalizeBorder(){
        borderHalfEdgesNum = 0;
        borderEdgesNum = 0;
        borderHalfEdges = halfedges.size();
        size_t ll = 0;

    }

protected:
    void updateOpposite(size_t h){
        halfedges[h].opp = halfedges.data() + h + 1;
        halfedges[h + 1].opp = halfedges.data() + h;
    }

public:
    std::vector<FlowVertex> vertices;
    std::vector<HalfEdge>   halfedges;
    std::vector<FlowFace>   faces;

    size_t  borderHalfEdgesNum;
    size_t  borderEdgesNum;
    size_t  borderHalfEdges;
};



#endif //CAD_MESH_H
