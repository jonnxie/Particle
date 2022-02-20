//
// Created by jonnxie on 2021/11/19.
//

#ifndef SHATTER_ENGINE_LISTENER_H
#define SHATTER_ENGINE_LISTENER_H

#include "Engine/Item/shatter_enum.h"
#include ItemCatalog
#include MacroCatalog
#include <unordered_map>

namespace Shatter{

    class Listener {
    public:
        Listener(){
            m_map = std::unordered_map<Event,bool>(int(Event::Count));
            m_action = std::unordered_map<Event,std::function<void()>>(int(Event::Count));
            for(int i = 0; i < int(Event::Count); i++)
            {
                m_action[Event(i)] = [i](){
                    return;
                };
            }
        }
        virtual void handle(Event _event){
            m_map[_event] = true;
            m_action[_event]();
        };
    protected:
        std::unordered_map<Event,bool> m_map;
        std::unordered_map<Event,std::function<void()>> m_action;
    };

    class DeleteObject : public Listener{
    public:
        DeleteObject();
    };


    class OutputPoint : public Listener{
    public:
        OutputPoint();
    };

    class OutputLine : public Listener{
    public:
        OutputLine();
    };


}

#endif //SHATTER_ENGINE_LISTENER_H
