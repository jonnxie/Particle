//
// Created by AnWell on 2022/2/14.
//

#ifndef PARTICLE_MEMOMANAGER_H
#define PARTICLE_MEMOMANAGER_H

#include <stack>

template<class Object, class State>
class MemoManager {
public:
    explicit MemoManager(){}
    explicit MemoManager(const State & _initial_state);

    ~MemoManager(){}

    void pushState(const State& _state);
    virtual void setState(const State& _state,Object& _obj) const = 0;

    bool undo(Object& _obj);
    bool redo(Object& _obj);
    void reset(Object& _obj);

private:
    std::stack<State> undoStack;
    std::stack<State> redoStack;
};


#endif //PARTICLE_MEMOMANAGER_H
