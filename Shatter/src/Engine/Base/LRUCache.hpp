//
// Created by AnWell on 2022/3/29.
//

#ifndef MAIN_LRUCACHE_HPP
#define MAIN_LRUCACHE_HPP

#include "Engine/Item/shatter_macro.h"
#include "Engine/Item/shatter_item.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <set>
#include <functional>

template<class Item>
class LRUCache {
public:
    LRUCache() = default;
    DefineUnCopy(LRUCache);

private:
    int maxSize = 800;
    int minSize = 600;
    float unloadPercent = 0.05;
    std::unordered_map<Item, float> itemSet;
    std::vector<Item> itemList;
    std::set<Item> usedSet;
    std::unordered_map<Item, std::function<void(Item)>> callbacks;
    std::function<float(Item)> unloadPriorityCallback;
    bool scheduled = false;
    std::function<float(Item)> defaultPriorityCallback = [&] (Item item) -> float{
        return itemSet[item];
    };
public:
    bool isFull(){
        return itemSet.size() >= maxSize;
    };

    bool add(Item item, const std::function<void(Item)>& removeCb) {
        if (itemSet.count(item) != 0){
            return false;
        }

        if (isFull()) {
            return false;
        }

        itemList.push_back(item);
        usedSet.insert(item);
        itemSet[item] = timer::getTime();
        callbacks[item] = removeCb;

        return true;
    }

    bool remove(Item item) {
        if(itemSet.count(item) != 0) {
            callbacks[item](item);
            auto iterator = std::find(itemList.begin(), itemList.end(), item);
            itemList.erase(iterator);
            usedSet.erase(item);
            itemSet.erase(item);
            callbacks.erase(item);
            return true;
        }
        return false;
    }

    void markUsed(Item item) {
        if(itemSet.count(item) != 0 && usedSet.count(item) == 0) {
            itemSet[item] = timer::getTime();
            usedSet.insert(item);
        }
    }

    void markAllUnused() {
        usedSet.clear();
    }

    void unloadUnusedContent() {
        int unused = itemSet.size() - usedSet.size();
        int excess = itemList.size() - minSize;

        if (excess > 0 && unused > 0) {
            std::sort(itemList.begin(), itemList.end(), [&](const Item& a,const Item& b) -> bool {
                int a_count = usedSet.count(a);
                int b_count = usedSet.count(b);
                if (a_count != 0 && b_count != 0) {
                    return true;
                }else if (a_count == 0 && b_count ==0) {
                    // Use the sort function otherwise
                    // higher priority should be further to the left
                    return defaultPriorityCallback( a ) < defaultPriorityCallback( b );
                }else {
                    return a_count == 0;
                }
            });

            int unusedExcess = std::min(excess, unused);
            int maxUnload = std::max(minSize * unloadPercent,
                                     float(unusedExcess) * unloadPercent);
            int nodesToUnload = std::min(maxUnload, unused);

            for(size_t i = 0; i < nodesToUnload; i++) {
                callbacks[itemList[i]](itemList[i]);
                itemSet.erase(itemList[i]);
                callbacks.erase(itemList[i]);
            }
        }
    }

    void scheduleUnload(bool _markAllUnused = true) {
        if( !scheduled) {
            scheduled = true;

            {
                scheduled = false;
                unloadUnusedContent();
                if(_markAllUnused) {
                    markAllUnused();
                }
            }
        }
    }


};


#endif //MAIN_LRUCACHE_HPP
