//
// Created by AnWell on 2022/2/19.
//

#include "delayevent.h"
#include "taskpool.h"

DelayEvent::DelayEvent() {
    m_pool = std::make_unique<ThreadPool>();
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
