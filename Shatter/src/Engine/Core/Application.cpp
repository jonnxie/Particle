#include "precompiledhead.h"
#include "Application.h"
#include "Engine/Event/threadpool.h"

namespace Shatter{

    Application& Application::getApplication() {
        static Application app;

        return app;
    }

    Application::Application() {

    }

    Application::~Application() {

    }

    void Application::update() {
//        auto dpool = MPool<DObject>::getPool();
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
                        l.second.handle(e);
                    }
                });
            }
            ThreadPool::pool()->wait();
            m_events.clear();
//
//            TaskPool::executeMultiple();
//            TaskPool::updateMultiple(abs_time_s);
//
//            static auto dUpdateTask = [&](){
//                for(auto& drawobj : m_dobjects){
//                    (*dpool)[drawobj]->update();
//                }
//            };
//            ThreadPool::pool()->addTask(dUpdateTask);
//
//            static auto transUpdateTask = [&](){
//                for(auto& tObj : transparency_vec){
//                    (*dpool)[tObj]->update();
//                }
//            };
//            ThreadPool::pool()->addTask(transUpdateTask);
//
//            static auto nUpdateTask = [&](){
//                for(auto& nObj : normal_vec){
//                    (*dpool)[nObj]->update();
//                }
//            };
//            ThreadPool::pool()->addTask(nUpdateTask);
//
//            static auto offUpdateTask = [&](){
//                for(auto& drawobj : m_offscreenobjects){
//                    (*dpool)[drawobj]->update();
//                }
//            };
//            ThreadPool::pool()->addTask(offUpdateTask);
//            ThreadPool::pool()->wait();
//
//            Camera::getCamera().update(cameraChanged);
//            render::ShatterRender::getRender().loop();
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

}
