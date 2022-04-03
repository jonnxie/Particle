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
#include "LRUCache.hpp"
#include "PriorityQueue.h"

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

namespace std {
    template <>
    class hash<Tile>{
    public:
        size_t operator()(const Tile& tile) const
        {
            return std::hash<float>()(tile.depth) ^
                   std::hash<float>()(tile.distanceFromCamera);
        }
    };

    template<>
    struct equal_to<Tile>{
    public:
        bool operator()(const Tile& t1, const Tile& t2) const
        {
            return t1.parent == t2.parent&&
            t1.depth == t2.depth&&
            t1.error == t2.error&&
            t1.distanceFromCamera == t2.distanceFromCamera&&
            t1.active == t2.active&&
            t1.used == t2.used&&
            t1.inFrustum == t2.inFrustum&&
            t1.depthFromRenderedParent == t2.depthFromRenderedParent;
        }
    };
}

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
    explicit TilesRendererBase(const std::string& _url);
    DefineUnCopy(TilesRendererBase);
public:
    TileSet& rootTileSet();
    TileBase* root();
    void traverse(std::function<void(Tile tile, Tile parent, float depth)> beforeCb,
                  std::function<void(Tile tile, Tile parent, float depth)> afterCb);
    void update();
    virtual void parseTile(std::vector<unsigned char> buffer,
                           const Tile& tile,
                           const std::string& extension);
    void disposeTile(const Tile& tile);
    void preprocessNode(const Tile& tile, const Tile& parentTile, const std::string& tileSetDir);
    void setTileActive(const Tile& tile,bool state);
    void setTileVisible(const Tile& tile,bool state);
    int calculateError(const Tile& tile);
    bool tileInView(const Tile& tile);
protected:
    std::unordered_map<std::string, TileSet> tileSets{};
    ClassElement(rootUrl, std::string, RootURL);
//    this.fetchOptions = {};
//
//    this.preprocessURL = null;
    LRUCache<Tile> lruCache{};
    PriorityQueue<Tile> downloadQueue{};
    PriorityQueue<Tile> parseQueue{};
    struct State {
        int parsing = 0;
        int downloading = 0;
        int failed = 0;
        int inFrustum = 0;
        int used = 0;
        int active = 0;
        int visible = 0;
    }stats;
    int frameCount = 0;
    int errorTarget = 6.0;
    bool loadSiblings = true;
    bool displayActiveTiles = false;
    float maxDepth;
    bool stopAtEmptyTiles = true;
};


#endif //MAIN_TILE_H
