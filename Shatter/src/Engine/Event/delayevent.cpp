//
// Created by AnWell on 2022/2/19.
//
#include "precompiledhead.h"
#include "delayevent.h"
#include "taskpool.h"

DelayEvent::DelayEvent() {
    m_pool = new ThreadPool(2);
}

DelayEvent& DelayEvent::getDelayEvent() {
    static DelayEvent event;
    return event;
}

void DelayEvent::push(const std::function<void()>& _delayAction, const std::function<void()>& _doneAction) {
    m_pool->addTask([=](){
        _delayAction();
        TaskPool::pushTask(_doneAction);
    });
}
