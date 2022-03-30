//
// Created by jonnxie on 2022/3/30.
//

#ifndef MAIN_PRIORITYQUEUE_H
#define MAIN_PRIORITYQUEUE_H

#include <iostream>
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include "Engine/Item/shatter_macro.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Event/delayevent.h"

template<class Item>
class PriorityQueue {
public:
    std::mutex jobLock;
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
        TaskPool::pushTask(func);
    }

    void _runjobs(){
        tryRunJobs();
        scheduled = false;
    }

    void sort(){
        std::sort(items.begin(), items.end(), priorityCallback);
    }

    std::future<void> add(Item item, std::function<void(Item)> callback) {
        return std::future<void>([&](){
            items.push_back(item);
            callbacks[item] = callback;
            if(autoUpdate) {
                scheduleJobRun();
            }
        });
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
        while (maxJobs > currJobs && !items.empty()) {
            {
                std::lock_guard<std::mutex> lockGuard(jobLock);
                currJobs ++;
            }
            auto item = items.front();
            auto callback = callbacks[item];
            callbacks.erase(item);
            DelayEvent::getDelayEvent().push([&, item](){
                callback(item);
            },[&](){
                {
                    std::lock_guard<std::mutex> lockGuard(jobLock);
                    currJobs --;
                }
                if ( autoUpdate ) {
                    scheduleJobRun();
                }
            });
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
