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
    void releaseMem();
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
    void releaseMem();
    void batch();
    void loadFile(const std::string& _filename) override;
    void drawNPlane();
    void destroy() const override;
public:
    ClassPointerElement(m_listener, DrawNPlaneHandle*, Listener);
    ClassReferenceElement(planes, std::deque<std::unique_ptr<DPlane>>, Planes);
private:
    bool appendState = true;
    bool batched = false;
    int drawId{};
    int modelId{};
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
    void releaseMem();
    void constructG() override;
    void constructD() override;
    void constructC() override{};

public:
    Cube                cube;
    bool                changed = true;
    int                 id;
};

class DrawCubeHandle;

class DCubeHandle :public Handle {
public:
    DCubeHandle();
    ~DCubeHandle() override;
    DefineUnCopy(DCubeHandle);
public:
    Cube& operator[](size_t _index);
    void pushCube(const Cube& _cube);
    void pushDCube(std::unique_ptr<DCube> _cube);
    void pushUI() override;
    int  getCubeCount();
    void releaseMem();
    void batch();
    void combine(std::vector<glm::vec3>& _posVec,
                 std::vector<glm::vec3>& _normalVec,
                 std::vector<glm::vec2>& _uvVec,
                 std::vector<uint32_t>& _index);
    void combineIndex(std::vector<uint32_t>& _index);
    void loadFile(const std::string& _filename) override;
    void drawCube();
    void destroy() const override;
public:
    ClassPointerElement(m_listener, DrawCubeHandle*, Listener);
    ClassReferenceElement(cubes, std::deque<std::unique_ptr<DCube>>, Cubes);
private:
    bool appendState = true;
    bool batched = false;
    int  drawId{};
    int  modelId{};
};

class DrawCubeHandle : public Shatter::Listener {
public:
    explicit DrawCubeHandle(DCubeHandle* _handle);
    ~DrawCubeHandle() override;
private:
    DCubeHandle* handle;
};

class DrawCube : public Shatter::Listener{
public:
    DrawCube();
    ~DrawCube() override;
private:
    std::deque<DCube*> cubes;
};


#endif //GAME_TRIS_H
