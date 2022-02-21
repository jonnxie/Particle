#include "precompiledhead.h"
#include "Application.h"
#include "Engine/Event/threadpool.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Vulkan/VulkanRender.h"
#include ListenerCatalog
#include "Engine/Mesh/line.h"
#include "Engine/Mesh/plane.h"
#include "Engine/Particle/particle.h"
#include "Engine/Event/taskpool.h"
#include "Engine/Object/camera.h"

namespace Shatter{
    Application& Application::getApplication() {
        static Application app;

        return app;
    }

    Application::Application() {
#ifdef SHATTER_PLATFORM_WINDOWS
      m_window = std::make_unique<WindowsWindow>(WindowProps());
#endif
      m_window->setApplication(this);
#ifdef SHATTER_GRAPHICS_VULKAN
      m_render =  std::unique_ptr<Render>();
#endif
    }

    Application::~Application() {

    }

    void Application::update() {
        auto dpool = MPool<DObject>::getPool();
        m_start_time = std::chrono::system_clock::now();
        while (m_running && !m_window->closed()) {
            auto now = std::chrono::system_clock::now();
            auto abs_time = now - m_start_time;
            int64_t abs_time_f = std::chrono::duration_cast<std::chrono::milliseconds>(abs_time).count();
            float abs_time_s = abs_time_f / 1000.0f;
            timer::setDetaTime(abs_time_s -timer::getTime());
            timer::setTime( abs_time_s);
            m_window->update();

            for(auto& e : m_events)
            {
                ThreadPool::pool()->addTask([&,e](){
                    for(auto& l : m_listeners)
                    {
                        l.second->handle(e);
                    }
                });
            }
            ThreadPool::pool()->wait();
            m_events.clear();

            TaskPool::executeMultiple();
            TaskPool::updateMultiple(abs_time_s);

            static auto dUpdateTask = [&](){
                for(auto& drawobj : m_defaultObjects){
                    (*dpool)[drawobj]->update();
                }
            };
            ThreadPool::pool()->addTask(dUpdateTask);

            static auto transUpdateTask = [&](){
                for(auto& tObj : m_transObjects){
                    (*dpool)[tObj]->update();
                }
            };
            ThreadPool::pool()->addTask(transUpdateTask);

            static auto nUpdateTask = [&](){
                for(auto& nObj : m_normalObjects){
                    (*dpool)[nObj]->update();
                }
            };
            ThreadPool::pool()->addTask(nUpdateTask);

            static auto offUpdateTask = [&](){
                for(auto& drawobj : m_offObjects){
                    (*dpool)[drawobj]->update();
                }
            };
            ThreadPool::pool()->addTask(offUpdateTask);
            ThreadPool::pool()->wait();

            Camera::getCamera().update(m_cameraChanged);
            render::ShatterRender::getRender().loop();
        }
//        vkDeviceWaitIdle(*render::ShatterRender::getRender().getDevice());
    }

    void Application::pushObject(DrawObjectType _type, uint32_t _id) {
        switch (_type) {
            case DrawObjectType::Default:{
                m_defaultObjects.push_back(_id);
            }
            case DrawObjectType::OffScreen:{
                m_offObjects.push_back(_id);
            }
            case DrawObjectType::Transparency:{
                m_transObjects.push_back(_id);
            }
            case DrawObjectType::Normal:{
                m_normalObjects.push_back(_id);
            }
            default:
            {
                WARNING("No such DrawObjectType");
            }
        }
    }

    void Application::pushObjects(DrawObjectType _type,const std::vector<uint32_t>& _ids) {
        switch (_type) {
            case DrawObjectType::Default:{
                m_defaultObjects.insert(m_defaultObjects.end(), _ids.begin(), _ids.end());
            }
            case DrawObjectType::OffScreen:{
                m_offObjects.insert(m_offObjects.end(), _ids.begin(), _ids.end());
            }
            case DrawObjectType::Transparency:{
                m_transObjects.insert(m_transObjects.end(), _ids.begin(), _ids.end());
            }
            case DrawObjectType::Normal:{
                m_normalObjects.insert(m_normalObjects.end(), _ids.begin(), _ids.end());
            }
            default:
            {
                WARNING("No such DrawObjectType");
            }
        }
    }

    void Application::pushCObject(uint32_t _id) {
        m_computeObjects.push_back(_id);
    }

    void Application::pushCObjects(const std::vector<uint32_t> &_ids) {
        m_computeObjects.insert(m_computeObjects.end(), _ids.begin(), _ids.end());
    }

    void Application::resizeCallback(int _width, int _height) {

    }

    void Application::keyCallback(int _key, int _action) {
        if(_action == GLFW_PRESS)
        {
            if(_key == GLFW_KEY_LEFT ||
               _key == GLFW_KEY_RIGHT ||
               _key == GLFW_KEY_DOWN ||
               _key == GLFW_KEY_UP)
            {
                m_cameraChanged = true;
            }
            m_events.push_back(Event::SinglePress);
            if(_key == GLFW_KEY_SPACE)
            {
//                SingleCamera.reset();
                m_cameraChanged = true;
            }

            if(_key == GLFW_KEY_F1)
            {
                Config::setConfig("enableScreenGui", 1 - Config::getConfig("enableScreenGui"));
            }

            if(_key == GLFW_KEY_DELETE)
            {
                m_events.push_back(Event::DeletePress);
            }

            if(_key == GLFW_KEY_F2)
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

            if(_key == GLFW_KEY_F3)
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

            if(_key == GLFW_KEY_F4)
            {
                static bool drawPoint = true;
                if(drawPoint)
                {
                    drawPoint = false;
                    appendListener("DrawPoint",new DrawPoint);
                }else{
                    deleteListener("DrawPoint");
                    drawPoint = true;
                }
            }
        }

        if(_action == GLFW_REPEAT)
        {
            m_events.push_back(Event::DoublePress);
        }
    }

    void Application::charCallback(int _keycode) {

    }

    void Application::mouseCallback(int _button, int _action, double _xPos, double _yPos) {
        if(_button == GLFW_MOUSE_BUTTON_LEFT && _action == GLFW_PRESS)
        {
            m_events.push_back(Event::SingleClick);
        }
        if(_button == GLFW_MOUSE_BUTTON_LEFT || _button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            m_cameraChanged = true;
        }
        if((_button == GLFW_MOUSE_BUTTON_LEFT || _button == GLFW_MOUSE_BUTTON_MIDDLE) &&
                _action == GLFW_RELEASE)
        {
            m_cameraChanged = false;
        }
    }

    void Application::scrollCallback(double _xOffset, double yOffset) {
        m_cameraChanged = true;
    }

    void Application::cursorCallback(double _xPos, double _yPos) {

    }

    void Application::deleteListener(const std::string &_name) {
        delete m_listeners[_name];
        m_listeners.erase(_name);
    }

    void Application::appendListener(const std::string &_name, Listener* _listener) {
        m_listeners[_name] = _listener;
    }

}
