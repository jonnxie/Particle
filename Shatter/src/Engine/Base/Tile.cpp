//
// Created by AnWell on 2022/3/29.
//

#include "precompiledhead.h"
#include "Tile.h"
#include "traverseFunction.h"
#include <regex>

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

TileBase<> *TilesRendererBase::root() {
    return tileSets[rootUrl].root;
}

void TilesRendererBase::traverse(const std::function<bool(const Tile& , const Tile&, int)>& beforeCb,
                                 const std::function<bool(const Tile& , const Tile&, int)>& afterCb) {
    auto rootTileSet = tileSets[rootUrl];
    if (!rootTileSet.root) return;

    traverseSet(*rootTileSet.root, beforeCb, afterCb);
}

void TilesRendererBase::update() {
    if (tileSets.count(rootUrl) == 0) {
        loadRootTileSet(rootUrl);
    } else if (!tileSets[rootUrl].root) {
        return;
    }
    auto root = rootTileSet().root;
    stats.inFrustum = 0;
    stats.used = 0;
    stats.active = 0;
    stats.visible = 0;
    frameCount++;

    determineFrustumSet(*rootTileSet().root, this);
    markUsedSetLeaves(*rootTileSet().root, this);
    skipTraversal(*rootTileSet().root, this);
    toggleTiles(*rootTileSet().root, this);

    lruCache.scheduleUnload();
}

void TilesRendererBase::parseTile(std::vector<unsigned char> _buffer,
                                  const Tile& _tile,
                                  const std::string& _extension) {


}

void TilesRendererBase::disposeTile(const Tile& _tile) {

}

std::string urlJoin(const std::vector<std::string>& _args, const std::string& _path) {
    std::regex protocolRegex("[a-zA-Z]+:\\/\\/");
    int lastRoot = -1;
    for (int i = 0, l = _args.size(); i < l; i++) {
        if (std::regex_match(_args[i], protocolRegex)) {
            lastRoot = i;
        }
    }
    if (lastRoot == -1) {
        std::string path = _path;
        for (auto& s : _args) {
            path += s;
        }
        std::regex regex("\\");
        return std::regex_replace(path, regex,"/");
    } else {
        std::vector<std::string> parts = _args;
        if (lastRoot > 0) {
            auto it = parts.begin();
            std::advance(it, lastRoot);
            parts.erase(it);
        }
        std::smatch match;
        std::regex_match(parts[0], match, protocolRegex);
        auto protocol = match[0].str();
        parts[0].erase(parts[0].find(protocol), protocol.length());
        std::string result = protocol + _path;
        for (auto& s : parts) {
            result += s;
        }
        std::regex regex("\\");
        return std::regex_replace(result, regex,"/");
    }
}

std::string getUrlExtension(const std::string& _url) {
    auto filename = _url.substr(0, _url.find('/'));
    int dotIndex = filename.find_last_of('.');
    if (dotIndex == -1 || dotIndex == filename.length() - 1) {
        return "";
    }
    return filename.substr(0, dotIndex+1);
}

void TilesRendererBase::preprocessNode(Tile &_tile, Tile* _parentTile, const std::string &_tileSetDir) {
    if (_tile.content.has_value()) {
        if (!_tile.content->uri.has_value() && _tile.content->url.has_value()) {
            _tile.content->uri->get() = _tile.content->url->get();
            _tile.content->url->get().clear();
        }

        if (_tile.content->uri.has_value()) {
            _tile.content->uri->get() = urlJoin(std::vector<std::string>{_tileSetDir, _tile.content->uri->get()}, "");
        }

        //
        {
//        if (
//                tile.content.boundingVolume &&
//                ! (
//                        'box' in tile.content.boundingVolume ||
//                                 'sphere' in tile.content.boundingVolume ||
//                                             'region' in tile.content.boundingVolume
//        )
//        ) {
//
//            delete tile.content.boundingVolume;
//
//        }
        }
    }

    _tile.parent = _parentTile;
    if (_tile.content->uri.has_value()) {
        auto extension = getUrlExtension(_tile.content->uri->get());
        bool isExternalTileSet = bool(extension == "json");
        _tile.externalTileSet = isExternalTileSet;
        _tile.contentEmpty = isExternalTileSet;
    } else {
        _tile.externalTileSet = false;
        _tile.contentEmpty = true;
    }

    _tile.distanceFromCamera = 0;
    _tile.error = 0;

    _tile.inFrustum = false;
    _tile.isLeaf = false;

    _tile.usedLastFrame = false;
    _tile.used = false;

    _tile.wasSetVisible = false;
    _tile.visible = false;
    _tile.childrenWereVisible = false;
    _tile.allChildrenLoaded = false;

    _tile.wasSetActive = false;
    _tile.active = false;

    _tile.loadingState = UNLOADED;
    _tile.loadIndex = 0;

    _tile.loadAbort = 0;

    _tile.depthFromRenderedParent = - 1;

    if (_parentTile == nullptr) {
        _tile.depth = 0;
        _tile.refine = _tile.refine + "REPLACE";
    } else {
        _tile.depth = _parentTile->depth+1;
        _tile.refine = _tile.refine + _parentTile->refine;
    }
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

void TilesRendererBase::loadRootTileSet(const std::string& _url) {

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


