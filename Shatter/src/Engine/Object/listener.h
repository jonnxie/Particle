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
            m_map = std::unordered_map<Event,bool>();
            m_action = std::unordered_map<Event,std::function<void()>>();
        }
        DefineUnCopy(Listener);
        virtual ~Listener()= default;
        virtual void handle(Event _event){
            if(m_action.count(_event) != 0)
            {
                m_map[_event] = true;
                m_action[_event]();
            }
        };
    protected:
        std::unordered_map<Event,bool> m_map;
        std::unordered_map<Event,std::function<void()>> m_action;
    };

    class DeleteObject : public Listener{
    public:
        DeleteObject();
    };

    class CaptureObject : public Listener{
    public:
        CaptureObject();
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
