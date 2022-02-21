#pragma once

#include "Core.h"
#include <iostream>
#include "Window.h"
#include "Render.h"
#include "Engine/Item/shatter_item.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/shatter_macro.h"
#include "Engine/Object/listener.h"

namespace Shatter {

	class Application
	{
	public:
        static Application& getApplication();

		virtual ~Application();

        DefineUnCopy(Application);
    public:
        void update();

        void pushObject(DrawObjectType _type,uint32_t _id);

        void pushObjects(DrawObjectType _type,const std::vector<uint32_t>& _ids);

        void pushCObject(uint32_t _id);

        void pushCObjects(const std::vector<uint32_t>& _ids);

        void appendListener(const std::string& _name,Listener* _listener);

        void deleteListener(const std::string& _name);
    public:
        void resizeCallback(int _width,int _height);

        void keyCallback(int _key,int _action);

        void charCallback(int _keycode);

        void mouseCallback(int _button,int _action,double _xPos,double _yPos);

        void scrollCallback(double _xOffset,double yOffset);

        void cursorCallback(double _xPos,double _yPos);
    private:
        Application();
    private:
        std::vector<uint32_t> m_defaultObjects{};
        std::vector<uint32_t> m_offObjects{};
        std::vector<uint32_t> m_transObjects{};
        std::vector<uint32_t> m_normalObjects{};
        std::vector<uint32_t> m_computeObjects{};
        std::vector<Event> m_events{};
        std::unordered_map<std::string,Listener*> m_listeners;
    private:
		std::unique_ptr<Window> m_window{};
        std::unique_ptr<Render> m_render{};
		bool m_running = true;
        time_point m_start_time{};
        bool m_cameraChanged = true;
	};

	//To be defined in CLIENT
	Application* createApplication();

}

