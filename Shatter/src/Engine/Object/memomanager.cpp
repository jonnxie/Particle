//
// Created by AnWell on 2022/2/14.
//

#include "memomanager.h"

template<class Object,class State>
MemoManager<Object,State>::MemoManager(const State &_initial_state)
{
    pushState(_initial_state);
}

template<class Object,class State>
void MemoManager<Object,State>::pushState(const State &_state)
{
    undoStack.push(_state);
}

template<class Object,class State>
bool MemoManager<Object,State>::undo(Object &_obj)
{
    if(undoStack.size() < 2)
    {
        return false;
    }
    redoStack.push(undoStack.top());
    undoStack.pop();
    setState(undoStack.top(), _obj);
    return true;
}

template<class Object,class State>
bool MemoManager<Object,State>::redo(Object &_obj)
{
    if(redoStack.empty()) return false;
    undoStack.push(redoStack.top());
    redoStack.pop();
    setState(undoStack.top(),_obj);
}

template<class Object,class State>
void MemoManager<Object,State>::reset(Object &_obj)
{
    if(undoStack.empty()) return;
    while(undoStack.size() > 1) undoStack.pop();
    while(!redoStack.empty()) redoStack.pop();
    setState(undoStack.top(), _obj);
}






