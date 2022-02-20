#pragma once

#include <utility>
#include "Core.h"
#include "precompiledhead.h"
#include "Engine/Item/shatter_enum.h"
#include "Engine/Item/configs.h"

namespace Shatter {

	struct WindowProps
	{
		std::string m_title;
		uint32_t m_width;
		uint32_t m_height;

		WindowProps(uint32_t _width,
			        uint32_t _height,
                    std::string  _title = "Shatter Engine")
			: m_title(std::move(_title)), m_width(_width), m_height(_height)
		{
		}
        WindowProps(std::string _title = "Shatter Engine")
        : m_title(std::move(_title))
        {
            m_width = Config::getConfig("width");
            m_height = Config::getConfig("height");
        }
	};

	// Interface representing a desktop system based Window
    class Window
    {
    public:
        virtual ~Window() = default;

        void setEventCallback(eventCallback _callback);

        virtual void update() = 0;

        virtual bool closed() = 0;

        virtual void* getNativeWindow() const = 0;
    public:
        std::unique_ptr<Window> createWindow(const WindowProps& props = WindowProps());
    protected:
        std::unordered_map<WindowEvent,std::function<void()>> m_callbacks;
    };

}
