//
// Created by AnWell on 2022/3/29.
//

#include "precompiledhead.h"
#include "Tile.h"

TilesRendererBase::TilesRendererBase(const std::string &_url) {
    lruCache.unloadPriorityCallback = lruPriorityCallback;

    downloadQueue.maxJobs = 4;
    downloadQueue.priorityCallback = priorityCallback;

    parseQueue.maxJobs = 1;
    parseQueue.priorityCallback = priorityCallback;
}


TileSet &TilesRendererBase::rootTileSet() {
    if (tileSets.count(rootUrl) == 0)
    {
        WARNING("No root set");
    }
    return tileSets[rootUrl];
}

TileBase *TilesRendererBase::root() {
    return tileSets[rootUrl].root;
}

void TilesRendererBase::traverse(std::function<void(Tile, Tile, float)> beforeCb,
                                 std::function<void(Tile, Tile, float)> afterCb) {

}

void TilesRendererBase::update() {

}

void TilesRendererBase::parseTile(std::vector<unsigned char> buffer, const Tile &tile, const std::string &extension) {

}

void TilesRendererBase::disposeTile(const Tile& tile) {

}

void TilesRendererBase::preprocessNode(const Tile &tile, const Tile &parentTile, const std::string &tileSetDir) {

}

void TilesRendererBase::setTileActive(const Tile& tile,bool state) {

}

void TilesRendererBase::setTileVisible(const Tile& tile,bool state) {

}

int TilesRendererBase::calculateError(const Tile& tile) {
    return 0;
}

bool TilesRendererBase::tileInView(const Tile& tile) {
    return true;
}


int priorityCallback(const Tile &a, const Tile &b) {
    if ( a.depth != b.depth ){

        // load shallower tiles first
        return a.depth >b.depth ? - 1 : 1;

    } else if ( a.inFrustum != b.inFrustum ){

        // load tiles that are in the frustum at the current depth
        return a.inFrustum ? 1 : - 1;

    } else if ( a.used != b.used ){

        // load tiles that have been used
        return a.used ? 1 : - 1;

    } else if ( a.error != b.error ){

        // load the tile with the higher error
        return a.error >b.error ? 1 : - 1;

    } else if ( a.distanceFromCamera != b.distanceFromCamera ){

        // and finally visible tiles which have equal error (ex: if geometricError === 0)
        // should prioritize based on distance.
        return a.distanceFromCamera > b.distanceFromCamera ? - 1 : 1;

    }

    return 0;
}
