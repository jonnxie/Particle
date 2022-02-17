//
// Created by jonnxie on 2021/9/24.
//
#include "precompiledhead.h"

#include "ReactorTask.h"
#include "threadpool.h"

std::unordered_map<int,std::vector<std::function<void(bool)>>> ReactorTask::key_reaction = std::unordered_map<int,std::vector<std::function<void(bool)>>>();
std::unordered_map<int,std::vector<std::function<void(bool)>>> ReactorTask::mouse_reaction = std::unordered_map<int,std::vector<std::function<void(bool)>>>();

static std::mutex key_lock;
static std::mutex mouse_lock;


int ReactorTask::pushTask(ReactionType _type,int _id,std::function<void(bool)> _task)
{
    switch (_type) {
        case ReactionType::Key:{
            std::lock_guard<std::mutex> guard_mutex(key_lock);
            if(key_reaction.count(_id) == 0){
                key_reaction[_id] = std::vector<std::function<void(bool)>>{_task};
                return 0;
            }
            else{
                key_reaction[_id].emplace_back(_task);
                return key_reaction.count(_id) - 1;
            }
        }
        case ReactionType::Mouse:{
            std::lock_guard<std::mutex> guard_mutex(mouse_lock);
            if(mouse_reaction.count(_id) == 0){
                mouse_reaction[_id] = std::vector<std::function<void(bool)>>{_task};
                return 0;
            }
            else{
                mouse_reaction[_id].emplace_back(_task);
                return mouse_reaction.count(_id) - 1;
            }
        }
        default:{
            WARNING(no such type);
            break;
        }
    }
    return -1;
}

void ReactorTask::execute(ReactionType _type, int _id, bool _value) {
    switch (_type) {
        case ReactionType::Key:{
            if(key_reaction.count(_id) == 0){
                return ;
            }
            else{
                for(auto &task : key_reaction[_id])
                {
                    task(_value);
                }
                return;
            }
        }
        case ReactionType::Mouse:{
            if(mouse_reaction.count(_id) == 0){
                return ;
            }
            else{
                for(auto &task : mouse_reaction[_id])
                {
                    task(_value);
                }
                return;
            }
        }
        default:{
            WARNING(no such type);
            break;
        }
    }
}

void ReactorTask::executeMultiple(ReactionType _type, int _id, bool _value) {
    switch (_type) {
        case ReactionType::Key: {
            if (key_reaction.count(_id) == 0) {
                return;
            } else {
                for (auto &task: key_reaction[_id]) {
                    std::function<void(int64_t)> func = task;
                    ThreadPool::pool()->addTask([=] { func(_value); });
                }
                ThreadPool::pool()->wait();
                return;
            }
        }
        case ReactionType::Mouse: {
            if (mouse_reaction.count(_id) == 0) {
                return;
            } else {
                for (auto &task: mouse_reaction[_id]) {
                    std::function<void(int64_t)> func = task;
                    ThreadPool::pool()->addTask([=] { func(_value); });
                }
                ThreadPool::pool()->wait();
                return;
            }
        }
        default: {
            WARNING(no such type);
            break;
        }
    }
}

void ReactorTask::removeTask(ReactionType _type,int _id,int _index) {
    switch (_type) {
        case ReactionType::Key:{
            std::lock_guard<std::mutex> guard_mutex(key_lock);
            if(key_reaction.count(_id) <= _index){
                WARNING(no such task);
            }
            else{
                ADVANCE_UNORDERED(key_reaction,_id,_index);
            }
        }
        case ReactionType::Mouse:{
            std::lock_guard<std::mutex> guard_mutex(mouse_lock);
            if(mouse_reaction.count(_id) <= _index){
                WARNING(no such task);
            }
            else{
                ADVANCE_UNORDERED(mouse_reaction,_id,_index);
            }
        }
        default:{
            WARNING(no such type);
            break;
        }
    }
}
