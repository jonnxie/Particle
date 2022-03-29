//
// Created by AnWell on 2022/3/29.
//

#include "precompiledhead.h"
#include "Tile.h"

TilesRendererBase::TilesRendererBase(const std::string &_url) {

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
