//
// Created by maybe on 2020/11/13.
//
#include "precompiledhead.h"
#include "shatterapp.h"
#include "Engine/Object/camera.h"
#include "Engine/Pool/mpool.h"
#include "Engine/Object/dobject.h"
#include "Engine/Object/object.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Object/inputaction.h"
#include ListenerCatalog
#include ConfigCatalog
#include "Engine/Mesh/line.h"
#include "Engine/Mesh/plane.h"
#include "Engine/Particle/particle.h"
#include "Engine/Event/threadpool.h"
#include "Engine/Base/WorkPlane.h"
#include "Engine/Base/tris.h"
#include "Engine/Base/WorkPlane.h"
#include "Engine/Base/lines.h"
#include "Engine/Base/points.h"

namespace Shatter::app{
    bool app_created = false;

    static int particle_count = 1;

    ShatterApp& ShatterApp::getApp(){
        static ShatterApp app;
        return app;
    }

    ShatterApp::ShatterApp():
    lastTime(0.0),
    showFPS(false)
    {
        m_listener = new Shatter::Listener;
        m_width = Config::getConfig("width");
        m_height = Config::getConfig("height");
    }

    ShatterApp::~ShatterApp(){
//        delete m_listener;
//        for(auto& [name,listener] : m_otherListener)
//        {
//            delete listener;
//        }
    }

    void ShatterApp::updateTimer()
    {
        auto now = std::chrono::system_clock::now();
        auto abs_time = now - m_start_time;
        float deta_time = float(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_pre_time).count()) / 1000.0f;
        m_pre_time = now;
        float abs_time_s = float(std::chrono::duration_cast<std::chrono::milliseconds>(abs_time).count()) / 1000.0f;
        timer::setDetaTime(deta_time);
        timer::setTime(abs_time_s);
    }

    void ShatterApp::update(){
        m_start_time = std::chrono::system_clock::now();
        m_pre_time = m_start_time;
        while (!glfwWindowShouldClose(render::ShatterRender::getRender().getWindow())) {
            updateTimer();
            glfwPollEvents();

            for(auto& e : m_events)
            {
                ThreadPool::pool()->addTask([&,e](){
                    m_listener->handle(e);
                    for(auto& l : m_otherListener)
                    {
                        l.second->handle(e);
                    }
                });
            }
            ThreadPool::pool()->wait();
            m_events.clear();

            TaskPool::executeMultiple();
            TaskPool::updateMultiple(timer::getTime());

            if(!GUI::getGUI()->getItemState())
            {
                Camera::getCamera().update(cameraChanged);
            }
            render::ShatterRender::getRender().loop();
        }
        vkDeviceWaitIdle(*render::ShatterRender::getRender().getDevice());
        delete m_listener;
        for(auto& [name,listener] : m_otherListener)
        {
            delete listener;
        }

        delete m_work_plane;
    }

    void ShatterApp::key_event_callback(int key, int action){
        if(action == GLFW_PRESS)
        {
            if( key == GLFW_KEY_LEFT ||
                    key == GLFW_KEY_RIGHT ||
                    key == GLFW_KEY_DOWN ||
                    key == GLFW_KEY_UP)
            {
                cameraChanged = true;
            }
            m_events.push_back(Event::SinglePress);
            if( key == GLFW_KEY_SPACE)
            {
                SingleCamera.reset();
                cameraChanged = true;
            }

            if (key == GLFW_KEY_F1)
            {
                Config::setConfig("enableScreenGui", 1 - Config::getConfig("enableScreenGui"));
                SingleRender.guiChanged = true;
            }

            if (key == GLFW_KEY_DELETE)
            {
                m_events.push_back(Event::DeletePress);
            }

            if (key == GLFW_KEY_F2)
            {
                static bool drawLine = true;
                if(drawLine)
                {
                    drawLine = false;
                    appendListener("DrawLine",new DrawLine);
                } else {
                    deleteListener("DrawLine");
                    drawLine = true;
                }
            }

            if (key == GLFW_KEY_F3)
            {
                static bool drawPlane = true;
                if(drawPlane)
                {
                    drawPlane = false;
                    appendListener("DrawPlane",new DrawPlane);
                } else {
                    deleteListener("DrawPlane");
                    drawPlane = true;
                }
            }

            if (key == GLFW_KEY_F4)
            {
                static bool drawPoint = true;
                if(drawPoint)
                {
                    drawPoint = false;
                    if (Config::getConfig("enableScreenGui"))
                    {
                        auto pointHandle = new PointsHandle();
                        SingleRender.normalChanged = true;
                    } else {
                        appendListener("DrawPoint",new DrawPoint);
                    }
                } else {
                    deleteListener("DrawPoint");
                    drawPoint = true;
                }
            }

            if (key == GLFW_KEY_F5)
            {
                static bool captureObject = true;
                if(captureObject)
                {
                    captureObject = false;
                    appendListener("CaptureObject",new CaptureObjectListener);
                } else {
                    deleteListener("CaptureObject");
                    captureObject = true;
                }
            }

            if(key == GLFW_KEY_F6)
            {
                static bool drawLinePool = true;
                if(drawLinePool)
                {
                    drawLinePool = false;
                    if (Config::getConfig("enableScreenGui"))
                    {
                        auto lineHandle = new LineHandle();
                        SingleRender.normalChanged = true;
                    } else {
                        appendListener("drawLinePool", new DrawLinePool);
                    }
                } else {
                    deleteListener("drawLinePool");
                    drawLinePool = true;
                }
            }

            if(key == GLFW_KEY_F7)
            {
                static bool drawNPlane = true;
                if(drawNPlane)
                {
                    drawNPlane = false;
                    if (Config::getConfig("enableScreenGui"))
                    {
                        auto planeHandle = new DPlaneHandle();
                        SingleRender.normalChanged = true;
                    } else {
                        appendListener("drawNPlane", new DrawNPlane);
                    }
                } else {
                    deleteListener("drawNPlane");
                    drawNPlane = true;
                }
            }

            if(key == GLFW_KEY_F8)
            {
                static bool drawCube = true;
                if(drawCube)
                {
                    drawCube = false;
                    if (Config::getConfig("enableScreenGui"))
                    {
                        auto cubeHandle = new DCubeHandle();
                        SingleRender.normalChanged = true;
                    } else {
                        appendListener("drawCube",new DrawCube);
                    }
                } else {
                    deleteListener("drawCube");
                    drawCube = true;
                }
            }

            if(key == GLFW_KEY_F9)
            {
                static bool chooseWorkPlane = true;
                if(chooseWorkPlane)
                {
                    chooseWorkPlane = false;
                    appendListener("chooseWorkPlane",new ChooseWorkPlane);
                } else {
                    deleteListener("chooseWorkPlane");
                    chooseWorkPlane = true;
                }
            }

        }

        if(action == GLFW_REPEAT)
        {
            m_events.push_back(Event::DoublePress);
        }
    }

    void ShatterApp::mouse_event_callback(int button, int action, double xpos, double ypos){
        if(action == GLFW_PRESS)
        {
            cameraChanged = true;
            if(button == GLFW_MOUSE_BUTTON_LEFT)
            {
                m_events.push_back(Event::SingleClick);
//                cameraChanged = true;
            }else if(button == GLFW_MOUSE_BUTTON_MIDDLE)
            {
//                cameraChanged = true;
            }else if(button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                m_events.push_back(Event::MouseClick);
            }
        }else if(action == GLFW_RELEASE){
            if(button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE)
            {
                cameraChanged = false;
            }
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

    void ShatterApp::removeListener(const std::string& _name) {
        m_otherListener.erase(_name);
    }

    TargetPlane& ShatterApp::getWorkTargetPlane(){
        if(m_work)
        {
            return m_work_plane->getCoordinate();
        }else{
            return SingleCamera.m_targetPlane;
        }
    }

    glm::vec3& ShatterApp::getWorkTargetCenter(){
        if(m_work)
        {
            return m_work_plane->getCenter();
        }else{
            return SingleCamera.center;
        }
    }

    TargetPlane& ShatterApp::getCameraTargetPlane(){
        return SingleCamera.m_targetPlane;
    }

    WorkPlane* ShatterApp::generateWorkPlane(TargetPlane& _coordinate, const glm::vec3& _center){
        if(nullptr == m_work_plane)
        {
            m_work_plane = new WorkPlane(_coordinate, _center);
        }else{
            m_work_plane->regenerate(_coordinate, _center);
        }
        m_work = true;
        return m_work_plane;
    }

}





