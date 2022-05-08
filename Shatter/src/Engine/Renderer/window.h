//
// Created by jonnxie on 2022/5/6.
//

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <utility>

#include "Engine/Item/shatter_enum.h"
#include ItemCatalog
#include MacroCatalog
#include ConfigCatalog
#include <GLFW/glfw3.h>

class Window {
public:
    Window() {
        width = Config::getConfig("width");
        height = Config::getConfig("height");
    }
    Window(int _width, int _height, std::string  _title):width(_width), height(_height), title(std::move(_title)) {}

    virtual ~Window() {
    };
public:
    std::pair<int, int> operator()() {
        return std::make_pair(width, height);
    }
private:
    virtual void windowResized( int width, int height) = 0;
    virtual void keyCallback( int key, int action) = 0;
    virtual void mouseCallback(int button, int action, double xPos, double yPos) = 0;
    virtual void cursorPositionCallback( double xPos, double yPos) = 0;
    virtual void scrollCallback( double xOffset, double yOffset) = 0;
    virtual void keyTypeCallback( unsigned int code) = 0;

protected:
    int width;
    int height;
    std::string title{"Shatter"};
};

class GLFWWindow : public Window {
public:
    GLFWWindow() : Window(){
        init();
    }
    GLFWWindow(int _width, int _height, std::string  _title) : Window(_width, _height, std::move(_title)){
        init();
    }
    ~GLFWWindow() override {
        glfwDestroyWindow(window);
//        glfwTerminate();
    };
public:
    void keyCallback(int _key, int _action) override;
    void windowResized(int _width, int _height) override ;
    void mouseCallback(int _button, int _action, double _xPos, double _yPos) override ;
    void cursorPositionCallback( double _xPos, double _yPos) override ;
    void scrollCallback( double _xOffset, double _yOffset) override ;
    void keyTypeCallback( unsigned int _code) override ;
public:
    GLFWwindow* get(){
        return window;
    }
private:
    void init();
private:
    GLFWwindow *window{};
};




#endif //MAIN_WINDOW_H
