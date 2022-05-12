//
// Created by AnWell on 2021/6/26.
//
#include <Engine/Item/shatter_item.h>
#include "precompiledhead.h"

#include "inputaction.h"

State getActionState(const Input_Action&_id){
    return m_action_map[_id];
}

void setActionState(const Input_Action&_id,State _state){
    std::lock_guard<std::mutex> guard_mutex(m_mutex);
    m_action_map[_id] = _state;
}

void updateCursor(const glm::vec2& _pos){
    m_cursor_pos = _pos;
}

void getCursor(glm::vec2& _pos){
    _pos = m_cursor_pos;
}

glm::vec2& getCursorCoor(){
    return m_cursor_pos;
}

void updateCursorPressPos(const glm::vec2& _pos){
    m_cursor_press_pos = _pos;
}

glm::vec2& getCursorPressPos(){
    return m_cursor_press_pos;
}

void updateScrollPos(const glm::vec2&_pos){
    m_scroll_pos = _pos;
}

glm::vec2 getScrollPos(){
    glm::vec2 tmpPos = m_scroll_pos;
    m_scroll_pos = glm::vec2(0.0f,0.0f);
    return tmpPos;
}


bool checkKey(int _key){
    return m_press_key[_key];
}

void pressKey(int _key){
    m_press_key[_key] = true;
}

void releaseKey(int _key){
    m_press_key[_key] = false;
}

bool checkMouse(int _mouse){
    return m_press_mouse[_mouse];
}

void pressMouse(int _mouse){
    m_press_mouse[_mouse] = true;
}

void releaseMouse(int _mouse){
    m_press_mouse[_mouse] = false;
}

namespace input{
    genReferenceItem(Cursor, glm::vec3);
    genReferenceItem(CursorRay, glm::vec3);
    genReferenceItem(CursorPress, glm::vec3);
    genReferenceItem(CursorWindow, glm::vec2);
    genReferenceItem(CursorView, glm::vec2);
    genReferenceItem(MousePressCoordiante, glm::uvec2);
    genReferenceItem(CenterViewPort, UnionViewPort);
    genReferenceItem(TargetDepth, float);
    genInOut(mouseRay, glm::vec3);
    genInOut(captureObject, uint32_t);
    genInOut(LineColor, glm::vec3);
    genInOut(inputMode, InputMode);
    genInOut(targetDepth, float);
    genInOut(cursorWindow, glm::vec2);
    genInOut(formatProperties, VkFormatProperties);
}
