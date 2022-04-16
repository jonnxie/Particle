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
#include "TileHead.h"
#include "traverseFunction.h"

using namespace nlohmann;


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

    Tile* root;

    std::vector<std::string> extensionUsed;
    std::vector<std::string> extensionRequired;
    json                     properties;
    json                     extensions;
    json                     extras;
};

using TileCallback = std::function<bool(const Tile& tile, const Tile& parent, int depth)>;

class TilesRendererBase {
public:
    explicit TilesRendererBase(const std::string& _url);
    DefineUnCopy(TilesRendererBase);
public:
    TileSet& rootTileSet();
    TileBase<>* root();
    void traverse(const std::function<bool(const Tile& tile, const Tile& parent, int depth)>& beforeCb,
                  const std::function<bool(const Tile& tile, const Tile& parent, int depth)>& afterCb);
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
    void loadRootTileSet(const std::string& _url);
    void requestTileContents(const Tile& tile){};
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
    int errorThreshold;
    bool loadSiblings = true;
    bool displayActiveTiles = false;
    float maxDepth;
    bool stopAtEmptyTiles = true;
};

bool determineFrustumSet(Tile &_tile, TilesRendererBase *renderer) {
    auto& stats = renderer->stats;
    auto& frameCount = renderer->frameCount;
    auto& errorTarget = renderer->errorTarget;
    auto& maxDepth = renderer->maxDepth;
    auto& loadSiblings = renderer->loadSiblings;
    auto& lruCache = renderer->lruCache;
    auto& stopAtEmptyTiles = renderer->stopAtEmptyTiles;
    resetFrameState(_tile, renderer->frameCount);
    if (!renderer->tileInView(_tile)) {
        return false;
    }
    _tile.used = true;
    renderer->lruCache.markUsed(_tile);
    _tile.inFrustum = true;
    renderer->stats.inFrustum++;

    if ( (stopAtEmptyTiles || ! _tile.contentEmpty) && !_tile.externalTileSet) {
        renderer->calculateError(_tile);

        auto error = _tile.error;
        if (error <= errorTarget) {
            return true;
        }

        if (renderer->maxDepth > 0 && _tile.depth + 1 >= maxDepth) {
            return true;
        }
    }
    bool anyChildrenUsed = false;
    for (auto & i : _tile.children) {
        auto r = determineFrustumSet(*i, renderer);
        anyChildrenUsed = anyChildrenUsed || r;
    }

    if (anyChildrenUsed && loadSiblings) {
        for (auto & i : _tile.children) {

        }
    }

}

void markUsedSetLeaves(Tile &_tile, TilesRendererBase *_renderer) {
    if (!isUsedThisFrame(_tile, _renderer->frameCount)) {
        return;
    }

    _renderer->stats.used++;

    bool anyChildrenUsed = false;
    for (auto& i : _tile.children) {
        anyChildrenUsed = anyChildrenUsed || isUsedThisFrame(*i, _renderer->frameCount);
    }

    if (!anyChildrenUsed) {
        _tile.isLeaf = true;
    } else {
        bool childrenWereVisible = false;
        bool allChildrenLoaded = true;
        for (int i = 0; i < _tile.children.size(); ++i) {
            markUsedSetLeaves(*_tile.children[i], _renderer);
            childrenWereVisible = childrenWereVisible ||
                    _tile.children[i]->wasSetVisible ||
                    _tile.children[i]->childrenWereVisible;

            if (isUsedThisFrame(*_tile.children[i],
                                _renderer->frameCount)) {
                bool childLoaded =
                        _tile.children[i]->allChildrenLoaded ||
                        ( ! _tile.children[i]->contentEmpty && isDownloadFinished( _tile.children[i]->loadingState ) ) ||
                        ( _tile.children[i]->externalTileSet && _tile.children[i]->loadingState == FAILED );
                allChildrenLoaded = allChildrenLoaded && childLoaded;
            }
        }
        _tile.childrenWereVisible = childrenWereVisible;
        _tile.allChildrenLoaded = allChildrenLoaded;
    }
}

void skipTraversal(Tile& _tile, TilesRendererBase* _renderer ) {
    auto stats = _renderer->stats;
    auto frameCount = _renderer->frameCount;
    if (!isUsedThisFrame(_tile, frameCount)) {
        return;
    }

    auto parent = _tile.parent;
    auto parentDepthToParent = parent ? parent->depthFromRenderedParent : - 1;
    _tile.depthFromRenderedParent = parentDepthToParent;

    auto& lruCache = _renderer->lruCache;
    if ( _tile.isLeaf ) {

        _tile.depthFromRenderedParent ++;

        if ( _tile.loadingState == LOADED ) {

            if ( _tile.inFrustum ) {

                _tile.visible = true;
                stats.visible ++;

            }
            _tile.active = true;
            stats.active ++;

        } else if ( ! lruCache.isFull() && ( ! _tile.contentEmpty || _tile.externalTileSet ) ) {
            _renderer->requestTileContents( _tile );
        }
        return;
    }
    auto errorRequirement = ( _renderer->errorTarget + 1 ) * _renderer->errorThreshold;
    auto meetsSSE = _tile.error <= errorRequirement;
    auto includeTile = meetsSSE || _tile.refine == "ADD";
    auto hasModel = ! _tile.contentEmpty;
    auto hasContent = hasModel || _tile.externalTileSet;
    auto loadedContent = isDownloadFinished( _tile.loadingState ) && hasContent;
    auto childrenWereVisible = _tile.childrenWereVisible;
    auto children = _tile.children;
    auto allChildrenHaveContent = _tile.allChildrenLoaded;

    if ( includeTile && hasModel) {
        _tile.depthFromRenderedParent++;
    }

    if ( includeTile && ! loadedContent && ! lruCache.isFull() && hasContent ) {
        _renderer->requestTileContents(_tile);
    }

    if (( meetsSSE && ! allChildrenHaveContent && ! childrenWereVisible && loadedContent )
            || ( _tile.refine == "ADD" && loadedContent )
            ) {

        if ( _tile.inFrustum ) {

            _tile.visible = true;
            stats.visible ++;

        }
        _tile.active = true;
        stats.active ++;
    }

    if ( _tile.refine != "ADD" && meetsSSE && ! allChildrenHaveContent && loadedContent ) {

        // load the child content if we've found that we've been loaded so we can move down to the next tile
        // layer when the data has loaded.
        for ( int i = 0, l = children.size(); i < l; i ++ ) {

            auto c = children[ i ];
            if ( isUsedThisFrame(*c, frameCount ) && ! lruCache.isFull() ) {
                c->depthFromRenderedParent =_tile.depthFromRenderedParent + 1;
                recursivelyLoadTiles( *c, c->depthFromRenderedParent, _renderer );
            }

        }

    } else {
        for ( int i = 0, l = children.size(); i < l; i ++ ) {

            auto c = children[ i ];
            if ( isUsedThisFrame( *c, frameCount ) ) {
                skipTraversal( *c, _renderer );
            }
        }
    }
}

bool recursivelyLoadTiles(Tile &_tile, int _depthFromRenderedParent, TilesRendererBase *_renderer) {
    return false;
}


#endif //MAIN_TILE_H
