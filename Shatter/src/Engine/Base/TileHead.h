//
// Created by jonnxie on 2022/4/16.
//

#ifndef MAIN_TILEHEAD_H
#define MAIN_TILEHEAD_H

#include <vector>
#include <functional>
#include <optional>
#include "json.hpp"
#include "Engine/Item/shatter_macro.h"

using namespace nlohmann;

struct Tile;

template<class Child = Tile>
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

    int geometricError{};

    std::vector<Child*> children;

    struct Content{
        json extensions;
        json extras;
        std::optional<std::reference_wrapper<std::string>> uri;
        std::optional<std::reference_wrapper<std::string>> url;
    };
    std::optional<Content> content;

    json extensions;
    json extras;
    std::string refine;
    std::vector<int> transform;
};

struct Tile : public TileBase<Tile>{
    Tile* parent{};
    float depth{};
    int   error{};
    float distanceFromCamera{};
    bool  active{};
    bool  used{};
    bool  inFrustum{};
    float depthFromRenderedParent{};
    int   lastFrameVisited;
    bool  isLeaf;
    bool  visible;
    bool  wasSetVisible;
    bool  wasSetActive;
    bool  childrenWereVisible;
    bool  allChildrenLoaded;
    bool  contentEmpty;
    bool  externalTileSet;
    int   loadingState;
    int   loadAbort;
    int   loadIndex;
    bool  usedLastFrame;
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

    template<>
    struct less<Tile>{
    public:
        bool operator()(const Tile& t1, const Tile& t2) const
        {
            if (t1.depthFromRenderedParent < t2.depthFromRenderedParent)
            {
                return true;
            }
        }
    };

}

#endif //MAIN_TILEHEAD_H
