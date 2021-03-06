#pragma once

#include "Engine/Core/Window.h"
#include "Engine/Item/shatter_macro.h"
#include <GLFW/glfw3.h>

namespace Shatter {

	class WindowsWindow : public Window
	{
	public:
		explicit WindowsWindow(WindowProps  props);
		~WindowsWindow() override;
        DefineUnCopy(WindowsWindow);

        void init();
        void release();
        void update() final;
        bool closed() final;
        void* getNativeWindow() const override { return m_Window; }
	private:
        WindowProps m_info;
		GLFWwindow* m_Window;
	};
}