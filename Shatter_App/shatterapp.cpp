//
// Created by maybe on 2020/11/13.
//

#include "shatterapp.h"
#include "../Shatter_Object/camera.h"
#include "../Shatter_Object/mpool.h"
#include "../Shatter_Object/dobject.h"
#include "../Shatter_Object/object.h"
#include "../Shatter_Object/taskpool.h"
#include "../Shatter_Object/inputaction.h"
#include ListenerCatalog
#include ConfigCatalog
#include "../Shatter_Mesh/line.h"
#include "../Shatter_Mesh/plane.h"

namespace shatter::app{
    bool app_created = false;

    static int particle_count = 1;

    ShatterApp& ShatterApp::getApp(){
        static ShatterApp app;
        if(!app_created) {
            app_created = true;
            if (!app.initApp()) {
                throw std::runtime_error("create app error!");
            }
        }
        return app;
    }

    bool ShatterApp::initApp(){
        bool val = false;
        do{
            val = true;
        }while(false);
        return val;
    }

    ShatterApp::ShatterApp():
    lastTime(0.0),
    showFPS(false)
    {
        m_listener = new Listener;
        m_width = Config::getConfig("width");
        m_height = Config::getConfig("height");
    }

    ShatterApp::~ShatterApp(){
        delete m_listener;
        for(auto& [name,listener] : m_otherListener)
        {
            delete listener;
        }
    }

    void ShatterApp::update(){
        auto dpool = MPool<DObject>::getPool();
        m_start_time = std::chrono::system_clock::now();
        while (!glfwWindowShouldClose(render::ShatterRender::getRender().getWindow())) {
            auto now = std::chrono::system_clock::now();
            auto abs_time = now - m_start_time;
            int64_t abs_time_f = std::chrono::duration_cast<std::chrono::milliseconds>(abs_time).count();
            float abs_time_s = abs_time_f / 1000.0f;
            timer::setDetaTime(abs_time_s -timer::getTime());
            timer::setTime( abs_time_s);
            glfwPollEvents();

            TaskPool::updateMultiple(abs_time_s);
            TaskPool::executeMultiple();

            for(auto& drawobj : m_dobjects){
                (*dpool)[drawobj]->update();
            }

            for(auto& tObj : transparency_vec){
                (*dpool)[tObj]->update();
            }

            for(auto& nObj : normal_vec){
                (*dpool)[nObj]->update();
            }

            for(auto& drawobj : m_offscreenobjects){
                (*dpool)[drawobj]->update();
            }

            Camera::getCamera().update(cameraChanged);
            render::ShatterRender::getRender().loop();
        }
        vkDeviceWaitIdle(*render::ShatterRender::getRender().getDevice());
    }

    void ShatterApp::key_event_callback(int key, int action){
        if(action == GLFW_PRESS)
        {
            if(key == GLFW_KEY_LEFT ||
                    key == GLFW_KEY_RIGHT ||
                    key == GLFW_KEY_DOWN ||
                    key == GLFW_KEY_UP)
            {
                cameraChanged = true;
            }
            m_listener->handle(Event::SinglePress);
            if(key == GLFW_KEY_SPACE)
            {
                SingleCamera.reset();
                cameraChanged = true;
                {
//                  auto particle = Particle::createParticle(particle_count++);
//                  m_dobjects.insert(m_dobjects.end(),particle->m_dobjs.begin(),particle->m_dobjs.end());
//                  m_objs.push_back(particle);
//                  render::ShatterRender::getRender().getDObjects()->insert(render::ShatterRender::getRender().getDObjects()->begin(),
//                                                                           m_dobjects.begin(),
//                                                                           m_dobjects.end());
                }
            }

            if(key == GLFW_KEY_F1)
            {
                Config::setConfig("enableScreenGui", 1 - Config::getConfig("enableScreenGui"));
            }

            if(key == GLFW_KEY_DELETE)
            {
                m_listener->handle(Event::DeletePress);
            }

            if(key == GLFW_KEY_F2)
            {
                static bool drawLine = true;
                if(drawLine)
                {
                    drawLine = false;
                    appendListener("DrawLine",new DrawLine);
                }else{
                    deleteListener("DrawLine");
                    drawLine = true;
                }
            }

            if(key == GLFW_KEY_F3)
            {
                static bool drawPlane = true;
                if(drawPlane)
                {
                    drawPlane = false;
                    appendListener("DrawPlane",new DrawPlane);
                }else{
                    deleteListener("DrawPlane");
                    drawPlane = true;
                }
            }
        }

        if(action == GLFW_REPEAT)
        {
            m_listener->handle(Event::DoublePress);
        }
    }

    void ShatterApp::mouse_event_callback(int button, int action, double xpos, double ypos){
        if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            m_listener->handle(Event::SingleClick);
            std::for_each(m_otherListener.begin(),m_otherListener.end(),[]
            (std::pair<const std::basic_string<char>, Listener *>& m)
            {
                m.second->handle(Event::SingleClick);
            });
        }
        if(button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            cameraChanged = true;
        }
        if((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE) &&
        action == GLFW_RELEASE)
        {
            cameraChanged = false;
        }
    }

    void ShatterApp::listener(Listener* _listener){
        if(nullptr == _listener) {
            PING(no suitable listener!);
        }
        delete m_listener;
        m_listener = _listener;
    }

    void ShatterApp::appendListener(const std::string& _name,Listener* _listener){
        m_otherListener[_name] = _listener;
    }

    std::vector<int> *ShatterApp::getCObjects() {
        return &m_cobjects;
    }

    std::vector<Object *> *ShatterApp::getObjects() {
        return &m_objs;
    }

    std::vector<int> *ShatterApp::getDObjects() {
        return &m_dobjects;
    }

    std::vector<int> *ShatterApp::getOffDObjects() {
        return &m_offscreenobjects;
    }

    std::vector<int>* ShatterApp::getNObjects(){
        return &normal_vec;
    }

    std::vector<int> *ShatterApp::getTObjects() {
        return &transparency_vec;
    }

    void ShatterApp::deleteListener(const std::string &_name) {
        delete m_otherListener[_name];
        m_otherListener.erase(_name);
    }

}





