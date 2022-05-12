//
// Created by jonnxie on 2022/5/6.
//

#include "window.h"
#include "Engine/Object/inputaction.h"
#include "imgui.h"
#include AppCatalog
#include CameraCatalog

void GLFWWindow::windowResized(int _width, int _height) {
    width = _width;
    height = _height;
}

void GLFWWindow::keyCallback(int _key, int _action) {
    ImGuiIO& io = ImGui::GetIO();

    if(_action == GLFW_PRESS)
    {
        pressKey(_key);
        io.KeysDown[_key] =  true;
    } else if(_action == GLFW_RELEASE)
    {
        io.KeysDown[_key] =  false;
        releaseKey(_key);
    }
    SingleAPP.key_event_callback(_key, _action);
}

void GLFWWindow::mouseCallback(int _button, int _action, double _xPos, double _yPos) {
    if (_action == GLFW_PRESS)
    {
        glm::uvec2& coordinate = input::getMousePressCoordiante();
        coordinate.x = _xPos;
        coordinate.y = _yPos;
        pressMouse(_button);
        glm::vec2& tmp = getCursorPressPos();
        tmp.x = float(coordinate.x) / getWindowViewPort().view.width;
        tmp.y = float(coordinate.y) / getWindowViewPort().view.height;
        tmp *= 2.0f;
        tmp -= 1.0f;
        glm::vec3& press_cursor = input::getCursorPress();
        press_cursor = input::getCursor();
    } else if(_action == GLFW_RELEASE)
    {
        releaseMouse(_button);
    }
    SingleAPP.mouse_event_callback(_button, _action, _xPos, _yPos);
}

void GLFWWindow::cursorPositionCallback(double _xPos, double _yPos) {
    static glm::vec2& cursor = input::getCursorWindow();
    cursor.x = _xPos;
    cursor.y = _yPos;
    UnionViewPort& viewport = getWindowViewPort();
    glm::vec2& tmp = getCursorCoor();
    tmp.x = cursor.x * viewport.inverseWidth;
    tmp.y = cursor.y * viewport.inverseHeight;
    tmp *= 2.0f;
    tmp -= 1.0f;

    UnionViewPort& presentViewPort = SingleAPP.getPresentViewPort();
    glm::vec2& viewCursor = input::getCursorView();
    glm::vec2 worldCoor;
    worldCoor.x = viewCursor.x * presentViewPort.inverseWidth;
    worldCoor.y = viewCursor.y * presentViewPort.inverseHeight;
    worldCoor *= 2.0f;
    worldCoor -= 1.0f;

    glm::vec4 view = SingleCamera.inverseProj * glm::vec4(worldCoor, input::getTargetDepth(), 1.0f);
    view /= view.w;
    glm::vec3& world = input::getCursor();
    world = SingleCamera.inverseView * view;
    SingleCamera.updateCursorRay();
}

void GLFWWindow::scrollCallback(double _xOffset, double _yOffset) {
    updateScrollPos(glm::vec2(_xOffset, _yOffset));
    SingleAPP.cameraChanged = true;
}

void GLFWWindow::keyTypeCallback(unsigned int _code) {
    ImGuiIO& io = ImGui::GetIO();
    if(_code > 0 && _code < 0x10000)
    {
        io.AddInputCharacter((unsigned short)_code);
    }
}

static void windowResizedStatic(GLFWwindow *_window, int _width, int _height) {
    printf("Window resized!\n");
    auto *w = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(_window));
    w->windowResized(_width, _height);
}

static void keyCallbackStatic(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
    auto *w = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(_window));
    w->keyCallback(_key, _action);
}

static void mouseCallbackStatic(GLFWwindow* _window, int _button, int _action, int _mods) {
    auto *w = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(_window));
    double xpos, ypos;
    glfwGetCursorPos(_window, &xpos, &ypos);
    w->mouseCallback(_button, _action, xpos, ypos);
}

static void cursorPositionCallbackStatic(GLFWwindow* _window, double _xPos, double _yPos) {
    auto *w = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(_window));
    w->cursorPositionCallback(_xPos, _yPos);
}

static void scrollCallbackStatic(GLFWwindow* _window, double _xOffset, double _yOffset) {
    auto *w = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(_window));
    w->scrollCallback(_xOffset, _yOffset);
}

static void keyTypeCallbackStatic(GLFWwindow* _window, unsigned int _code) {
    auto *w = reinterpret_cast<GLFWWindow*>(glfwGetWindowUserPointer(_window));
    w->keyTypeCallback(_code);
}

void GLFWWindow::init() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);

    glfwSetWindowSizeCallback(window, windowResizedStatic);

    glfwSetKeyCallback(window, keyCallbackStatic);

    glfwSetMouseButtonCallback(window, mouseCallbackStatic);

    glfwSetCursorPosCallback(window, cursorPositionCallbackStatic);

    glfwSetScrollCallback(window, scrollCallbackStatic);

    glfwSetCharCallback(window, keyTypeCallbackStatic);
}

