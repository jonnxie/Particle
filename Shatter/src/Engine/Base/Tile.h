//
// Created by AnWell on 2022/3/29.
//

#ifndef MAIN_TILE_H
#define MAIN_TILE_H

#include <vector>
#include "Engine/Object/aabb.h"
#include <functional>
#include "json.hpp"
#include "Engine/Item/shatter_macro.h"

using namespace nlohmann;

struct TileBase {
    union {
    /**
     * An array of 12 numbers that define an oriented bounding box. The first three elements define the x, y, and z
     * values for the center of the box. The next three elements (with indices 3, 4, and 5) define the x axis
     * direction and half-length. The next three elements (indices 6, 7, and 8) define the y axis direction and
     * half-length. The last three elements (indices 9, 10, and 11) define the z axis direction and half-length.
     */
     int box[12];
    /**
     * An array of four numbers that define a bounding sphere. The first three elements define the x, y, and z
     * values for the center of the sphere. The last element (with index 3) defines the radius in meters.
     */
     int sphere[4];
    } boundingVolume;

    int geometricError;

    std::vector<TileBase*> children;

    struct Content{
        json extensions;
        json extras;
        std::string url;
    } content;

    json extensions;
    json extras;
    std::string refine;
    std::vector<int> transform;
};

struct Tile : public TileBase{
    Tile* parent{};
    float depth{};
    int   error{};
    float distanceFromCamera{};
    bool  active{};
    bool  used{};
    bool  inFrustum{};
    float depthFromRenderedParent{};
};

int priorityCallback(const Tile& a, const Tile& b);

float lruPriorityCallback(const Tile& _tile) {
    return 1.0f / (_tile.depthFromRenderedParent + 1.0f);
}

struct TileInternal : public Tile {
    // tile description
    bool externalTileSet;
    bool contentEmpty;
    bool isLeaf;

    // resource tracking
    bool usedLastFrame;
    bool used;

    // Visibility tracking
    bool allChildrenLoaded;
    bool childrenWereVisible;
    bool inFrustum;
    bool wasSetVisible;

    // download state tracking
    /**
     * This tile is currently active if:
     *  1: Tile content is loaded and ready to be made visible if needed
     */
    bool active;
    int loadIndex;
    std::function<void()> loadAbort;
    int loadingState;
    bool wasSetActive;
};

struct TileSet {
    struct {
        std::string version;
        std::string tilesetVersion;
        json        extensions;
    } asset;

    int geometricError;

    TileBase* root;

    std::vector<std::string> extensionUsed;
    std::vector<std::string> extensionRequired;
    json                     properties;
    json                     extensions;
    json                     extras;
};

class TilesRendererBase {
public:
    TilesRendererBase(const std::string& _url);
    DefineUnCopy(TilesRendererBase);
public:
    TileSet& rootTileSet();
    TileBase* root();
protected:
    std::unordered_map<std::string, TileSet> tileSets{};
    ClassElement(rootUrl, std::string, RootURL);
};


#endif //MAIN_TILE_H
