//
// Created by maybe on 2020/11/13.
//

#ifndef VULKAN_TOTURIOL_SHATTER_APP_H
#define VULKAN_TOTURIOL_SHATTER_APP_H

#include "Engine/Render/shatter_render_include.h"
//#include "../Shatter_Asset/shatterasset.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include <iostream>
#include <memory>
#include <map>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <unordered_map>

//namespace shatter::camera{
//    class ShatterCamera;
//}

class Camera;

class Object;

class Listener;

namespace shatter{
    namespace app{

        class ShatterApp : public std::enable_shared_from_this<ShatterApp>{
        public:
            static ShatterApp& getApp();
            ShatterApp(const ShatterApp&) = delete;
            ShatterApp& operator=(const ShatterApp&)=delete;
            ~ShatterApp();
            bool initApp();
        public:
            int getScreenWidth(){return m_width;};
            int getScreenHeight(){return m_height;};
        public:
            void update();
            std::vector<int>* getDObjects();
            std::vector<int>* getCObjects();
            std::vector<int>* getOffDObjects();
            std::vector<int>* getNObjects();
            std::vector<int>* getTObjects();
            std::vector<Object*>* getObjects();
            void key_event_callback(int key, int action);
            void mouse_event_callback(int button, int action, double xpos, double ypos);
            void listener(Listener* _listener);
            void appendListener(const std::string& _name,Listener* _listener);
            void deleteListener(const std::string& _name);
        private:
            ShatterApp();

        private:
            /*
             * dobject
             */
            std::vector<int> m_dobjects;
            std::vector<int> m_offscreenobjects;
            std::vector<int> transparency_vec;
            std::vector<int> normal_vec;
            std::vector<Object*> m_objs;

            std::vector<int> m_cobjects;
            std::vector<Event> m_events;
        public:
            bool cameraChanged = true;

        private:
            float lastTime;
            bool showFPS;
            time_point m_start_time;
            Listener* m_listener;
            std::unordered_map<std::string,Listener*> m_otherListener;
            int m_width;
            int m_height;
        };

    }
}

#define SingleAPP app::ShatterApp::getApp()

#endif //VULKAN_TOTURIOL_SHATTER_APP_H
