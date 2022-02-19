//
// Created by AnWell on 2022/2/19.
//

#ifndef MAIN_DELAYEVENT_H
#define MAIN_DELAYEVENT_H

#include "precompiledhead.h"
#include "threadpool.h"
#include "Engine/Item/shatter_macro.h"

class DelayEvent{
    static DelayEvent& getDelayEvent();
    void push(const std::function<void()>& _delayAction,const std::function<void()>& _doneAction);
private:
    DelayEvent();
    DefineUnCopy(DelayEvent);
private:
    std::unique_ptr<ThreadPool> m_pool;
    std::unordered_map<uint32_t,std::function<void()>> m_reactions;
};

#define SingleDelaySystem DelayEvent::getDelayEvent()

#define PushDelayAction(delay,action) SingleDelaySystem.push(delay,action)

#endif //MAIN_DELAYEVENT_H
