//
// Created by jonnxie on 2021/9/24.
//

#ifndef SHATTER_ENGINE_REACTORTASK_H
#define SHATTER_ENGINE_REACTORTASK_H

#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <map>
#include <unordered_map>
#include "../Shatter_Item/shatter_item.h"
#include "../Shatter_Item/shatter_macro.h"
#include "../Shatter_Item/shatter_enum.h"


class ReactorTask {
public:
    static int pushTask(ReactionType _type,int _id,std::function<void(bool)> _task);
    static void removeTask(ReactionType _type,int _id,int _index);
    static void execute(ReactionType _type,int _id,bool _value);
    static void executeMultiple(ReactionType _type,int _id,bool _value);
public:
    static std::unordered_map<int,std::vector<std::function<void(bool)>>> key_reaction;
    static std::unordered_map<int,std::vector<std::function<void(bool)>>> mouse_reaction;
};


#endif //SHATTER_ENGINE_REACTORTASK_H
