//
// Created by jonnxie on 2022/1/28.
//

#ifndef GAME_TRIS_H
#define GAME_TRIS_H

#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include ObjectCatalog
#include DObjectCatalog
#include MPoolCatalog
#include DeviceCatalog
#include ListenerCatalog
#include HandleCatalog
#include "tiny_gltf.h"
#include "Engine/Object/VulkanglTFModels.h"

static int initTriIdVal = 0;
static std::mutex triIdLock;

static int mallocTriId()
{
    std::lock_guard<std::mutex> lockGuard(triIdLock);
    return initTriIdVal++;
}

class DTris : public Object{
public:
    explicit DTris(size_t _initCount, MeshDrawType _type = MeshDrawType::Line);
    explicit DTris(const std::vector<Tri>& _tris, MeshDrawType _type = MeshDrawType::Line);
    void constructG() override;
    void constructD() override;
    void constructC() override{};
    void pushTri(const Tri& _line);
    void pushTris(const std::vector<Tri>& _tris);

public:
    std::vector<Tri>    tris;
    MeshDrawType        type;
    bool                changed = true;
    int                 id;
};

class DPlane : public Object{
public:
    explicit DPlane(const NPlane& _plane);
    ~DPlane() override;
    void constructG() override;
    void constructD() override;
    void constructC() override{};
public:
    NPlane              plane;
    bool                changed = true;
    int                 id;
};

class DrawNPlaneHandle;

class DPlaneHandle :public Handle {
public:
    DPlaneHandle();
    ~DPlaneHandle() override;
    DefineUnCopy(DPlaneHandle);
public:
    NPlane& operator[](size_t _index);
    void pushNPlane(const NPlane& _plane);
    void pushDPlane(std::unique_ptr<DPlane> _plane);
    void pushUI() override;
    int  getNPlaneCount();
    void loadFile(const std::string& _filename) override;
    void drawNPlane();
    void destroy() const override;
public:
    ClassPointerElement(m_listener, DrawNPlaneHandle*, Listener);
    ClassReferenceElement(planes, std::deque<std::unique_ptr<DPlane>>, Planes);
private:
    bool appendState = true;
};

class DrawNPlaneHandle : public Shatter::Listener {
public:
    explicit DrawNPlaneHandle(DPlaneHandle* _handle);
    ~DrawNPlaneHandle() override;
private:
    DPlaneHandle* handle;
};

class DrawNPlane : public Shatter::Listener{
public:
    DrawNPlane();
    ~DrawNPlane() override;
private:
    std::deque<std::unique_ptr<DPlane>> planes;
};


class DCube : public Object{
public:
    explicit DCube(const Cube& _cube);
    ~DCube() override;
    void constructG() override;
    void constructD() override;
    void constructC() override{};

public:
    Cube                cube;
    bool                changed = true;
    int                 id;
};

class DrawCube : public Shatter::Listener{
public:
    DrawCube();
    ~DrawCube() override;
private:
    std::deque<DCube*> cubes;
};


#endif //GAME_TRIS_H
