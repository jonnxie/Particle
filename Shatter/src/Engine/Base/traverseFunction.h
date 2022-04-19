//
// Created by jonnxie on 2022/4/16.
//

#ifndef MAIN_TRAVERSEFUNCTION_H
#define MAIN_TRAVERSEFUNCTION_H

#include <functional>

struct Tile;
using TileCallback = std::function<bool(const Tile& tile, const Tile& parent, int depth)>;

static TileCallback nilTileCallback = [](const Tile& tile, const Tile& parent, int depth)->bool {
    return false;
};

void traverseSet(const Tile& _tile,
                 const TileCallback& _beforeCb = nilTileCallback,
                 const TileCallback& _afterCb = nilTileCallback,const Tile* _parent = nullptr, int _depth = 0);

class TilesRendererBase;

bool determineFrustumSet( Tile& _tile, TilesRendererBase* renderer);

void resetFrameState(Tile& _tile, int _frameCount);

template<class Item>
class LRUCache;

void recursivelyMarkUsed(Tile& _tile, int _frameCount, LRUCache<Tile>& _lruCache);

void markUsedSetLeaves(Tile& _tile, TilesRendererBase* _renderer );

void skipTraversal(Tile& _tile, TilesRendererBase* _renderer );

void toggleTiles(Tile& _tile, TilesRendererBase* _renderer );

bool recursivelyLoadTiles(Tile& _tile, int _depthFromRenderedParent, TilesRendererBase* _renderer);

bool isUsedThisFrame(const Tile& _tile, int _frameCount);

bool isDownloadFinished(int value);

#endif //MAIN_TRAVERSEFUNCTION_H
