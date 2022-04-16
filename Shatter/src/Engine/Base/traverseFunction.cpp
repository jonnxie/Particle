//
// Created by jonnxie on 2022/4/16.
//

#include "traverseFunction.h"
#include "TileHead.h"
#include "LRUCache.hpp"
#include "Engine/Item/shatter_enum.h"

void traverseSet(const Tile &_tile, const TileCallback &_beforeCb, const TileCallback &_afterCb, const Tile *_parent,
                 int _depth) {
    if (_beforeCb(_tile, *_parent, _depth)) {
        _afterCb(_tile, *_parent, _depth);
        return;
    }

    for (int i = 0; i < _tile.children.size(); ++i) {
        traverseSet(*_tile.children[i], _beforeCb, _afterCb, &_tile, _depth + 1);
    }

    _afterCb(_tile, *_parent, _depth);
}

void resetFrameState(Tile& _tile, int _frameCount) {
    if (_tile.lastFrameVisited != _frameCount) {
        _tile.lastFrameVisited = _frameCount;
        _tile.used = false;
        _tile.inFrustum = false;
        _tile.isLeaf = false;
        _tile.visible = false;
        _tile.active = false;
        _tile.error = 0;
        _tile.distanceFromCamera = 0;
        _tile.childrenWereVisible = false;
        _tile.allChildrenLoaded = false;
    }
}

void recursivelyMarkUsed(Tile& _tile, int _frameCount, LRUCache<Tile>& _lruCache) {
    resetFrameState(_tile, _frameCount);
    _tile.used = true;
    _lruCache.markUsed(_tile);

    if (_tile.contentEmpty) {
        for (auto& i : _tile.children) {
            recursivelyMarkUsed(*i, _frameCount, _lruCache);
        }
    }
}

bool isUsedThisFrame(const Tile& _tile, int _frameCount) {
    return _tile.lastFrameVisited == _frameCount && _tile.used;
}

bool isDownloadFinished(int value) {
    return value == LOADED || value == FAILED;
}






