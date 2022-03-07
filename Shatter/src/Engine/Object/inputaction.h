//
// Created by AnWell on 2021/6/26.
//

#ifndef SHATTER_ENGINE_INPUTACTION_H
#define SHATTER_ENGINE_INPUTACTION_H

#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include <map>
#include <string>
#include <mutex>
#include <vector>
#include <algorithm>
#include <vulkan/vulkan.h>

static std::mutex m_mutex;
static std::map<Input_Action,State> m_action_map;

State getActionState(const Input_Action&);
void setActionState(const Input_Action&,State);

static glm::vec2 m_cursor_pos;
void updateCursor(const glm::vec2&);
void getCursor(glm::vec2&);
glm::vec2& getCursorPos();

static glm::vec2 m_cursor_press_pos;
void updateCursorPressPos(const glm::vec2&);
glm::vec2& getCursorPressPos();

static glm::vec2 m_scroll_pos;
void updateScrollPos(const glm::vec2&);
glm::vec2 getScrollPos();

static std::map<int,bool> m_press_key;
bool checkKey(int);
void pressKey(int);
void releaseKey(int);

static std::map<int,bool> m_press_mouse;
bool checkMouse(int);
void pressMouse(int);
void releaseMouse(int);

namespace input{
    delInOut(cursor, glm::vec3);
    delInOut(mouseRay, glm::vec3);
    delInOut(captureObject,uint32_t);
    delInOut(MousePressCoordiante,glm::u32vec2);
    delInOut(inputMode, InputMode);
    delInOut(targetDepth, float);
    delInOut(cursorWindow, glm::vec2);
    delInOut(formatProperties, VkFormatProperties);
}

#endif //SHATTER_ENGINE_INPUTACTION_H
