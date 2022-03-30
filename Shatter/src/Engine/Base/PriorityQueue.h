//
// Created by jonnxie on 2022/3/30.
//

#ifndef MAIN_PRIORITYQUEUE_H
#define MAIN_PRIORITYQUEUE_H

#include <iostream>
#include <vector>
#include <functional>
#include "Engine/Item/shatter_macro.h"

template<class Item>
class PriorityQueue {
public:
    int maxJobs = 6;
    std::vector<Item> items{};
    std::unordered_map<Item, std::function<void(Item)>> callbacks;
    int currJobs = 0;
    bool scheduled = false;
    bool autoUpdate = true;
    std::function<void(const Item& a, const Item& b)> priorityCallback = [](const Item& a, const Item& b) ->bool {
        WARNING(PriorityQueue: PriorityCallback function not defined.);
    };
    void schedulingCallback(const std::function<void()>& func){

    }

    void _runjobs(){
        tryRunJobs();
        scheduled = false;
    }

    void sort(){
        std::sort(items.begin(), items.end(), priorityCallback);
    }

    void add(Item item, std::function<void(Item)> callback) {
        {
            items.push_back(item);
            callbacks[item] = callback;
            if(autoUpdate) {
                scheduleJobRun();
            }
        }
    }

    void remove(Item item) {
        auto iterator = std::find(items.begin(), items.end(), item);
        if(iterator != items.end()) {
            items.erase(item);
            callbacks.erase(item);
        }
    }

    void tryRunJobs(){
        sort();
        while (maxJobs > currJobs && items.size() > 0) {
            currJobs ++;
            auto item = items.front();
            callbacks[item](item);
            callbacks.erase(item);
            currJobs --;
            if(autoUpdate) {
                scheduleJobRun();
            }
        }

    }

    void scheduleJobRun() {
        if(!scheduled) {
            schedulingCallback(_runjobs());
            scheduled = true;
        }
    }
};


#endif //MAIN_PRIORITYQUEUE_H
