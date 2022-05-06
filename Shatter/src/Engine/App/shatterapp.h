//
// Created by maybe on 2020/11/13.
//

#ifndef VULKAN_TOTURIOL_SHATTER_APP_H
#define VULKAN_TOTURIOL_SHATTER_APP_H

#include "Engine/Renderer/shatter_render_include.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include <iostream>
#include <memory>
#include <map>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <unordered_map>
#include "Engine/Object/CaptureObject.h"

class Camera;

class WorkPlane;

namespace Shatter{
    class Listener;


    namespace app{

        class ShatterApp {
        public:
            static ShatterApp& getApp();
            ~ShatterApp();
            DefineUnCopy(ShatterApp);
        public:
            int getScreenWidth() const {return m_width;};
            int getScreenHeight() const {return m_height;};
        public:
            void updateTimer();
            void update();
            void key_event_callback(int key, int action);
            void mouse_event_callback(int button, int action, double xpos, double ypos);
            void listener(Listener* _listener);
            void appendListener(const std::string& _name,Listener* _listener);
            void deleteListener(const std::string& _name);
            void removeListener(const std::string& _name);
            TargetPlane& getWorkTargetPlane();
            glm::vec3&   getWorkTargetCenter();
            TargetPlane& getCameraTargetPlane();
            WorkPlane* generateWorkPlane(TargetPlane& _coordinate, const glm::vec3& _center);
            ClassPointerElement(m_swapChainImageCount, int, SwapChainCount);
            ClassPointerElement(m_workImageIndex, int, WorkImageIndex);

        public:
            void capturedPush(const std::shared_ptr<CaptureObject>& _id);
            void capturedRelease(const std::shared_ptr<CaptureObject>& _id);
            std::shared_ptr<CaptureObject> getCaptureById(uint32_t _id);
            std::vector<std::shared_ptr<CaptureObject>> m_captured;
        private:
            std::mutex m_captured_lock;
            ShatterApp();
            ClassElementInitial(m_work, bool, Work, false);
            WorkPlane* m_work_plane{nullptr};
        private:
            std::vector<Event> m_events;
        public:
            bool cameraChanged = true;
        private:
            float lastTime;
            time_point m_start_time;
            time_point m_pre_time;
            Listener* m_listener;
            std::unordered_map<std::string,Listener*> m_otherListener;
            int m_width;
            int m_height;
        };

    }
}

#define SingleAPP Shatter::app::ShatterApp::getApp()

#endif //VULKAN_TOTURIOL_SHATTER_APP_H
