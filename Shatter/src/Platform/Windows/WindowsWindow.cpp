#include "precompiledhead.h"
#include "Platform/Windows/WindowsWindow.h"


//namespace Shatter {
//
//	static uint8_t s_GLFWWindowCount = 0;
//
//	static void GLFWErrorCallback(int error, const char* description)
//	{
//	}
//
//	WindowsWindow::WindowsWindow(const WindowProps& props)
//	{
//
//		Init(props);
//	}
//
//	WindowsWindow::~WindowsWindow()
//	{
//
//		Shutdown();
//	}
//
//	void WindowsWindow::Init(const WindowProps& props)
//	{
//
//		m_Data.Title = props.Title;
//		m_Data.Width = props.Width;
//		m_Data.Height = props.Height;
//
//
//		if (s_GLFWWindowCount == 0)
//		{
//			int success = glfwInit();
//			glfwSetErrorCallback(GLFWErrorCallback);
//		}
//
//		{
//
//			m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
//			++s_GLFWWindowCount;
//		}
//
//
//		glfwSetWindowUserPointer(m_Window, &m_Data);
//		SetVSync(true);
//
//		// Set GLFW callbacks
//		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
//		{
//			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
//			data.Width = width;
//			data.Height = height;
//
//
//		});
//
//		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
//		{
//			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
//
//		});
//
//		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
//		{
//			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
//
//			switch (action)
//			{
//				case GLFW_PRESS:
//				{
//
//					break;
//				}
//				case GLFW_RELEASE:
//				{
//
//					break;
//				}
//				case GLFW_REPEAT:
//				{
//
//					break;
//				}
//			}
//		});
//
//		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
//		{
//			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
//
//
//		});
//
//		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
//		{
//			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
//
//			switch (action)
//			{
//				case GLFW_PRESS:
//				{
//
//					break;
//				}
//				case GLFW_RELEASE:
//				{
//
//					break;
//				}
//			}
//		});
//
//		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
//		{
//			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
//
//
//		});
//
//		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
//		{
//			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
//
//
//		});
//	}
//
//	void WindowsWindow::Shutdown()
//	{
//
//		glfwDestroyWindow(m_Window);
//		--s_GLFWWindowCount;
//
//		if (s_GLFWWindowCount == 0)
//		{
//			glfwTerminate();
//		}
//	}
//
//	void WindowsWindow::OnUpdate()
//	{
//
//		glfwPollEvents();
//	}
//
//	void WindowsWindow::SetVSync(bool enabled)
//	{
//
//		if (enabled)
//			glfwSwapInterval(1);
//		else
//			glfwSwapInterval(0);
//
//		m_Data.VSync = enabled;
//	}
//
//	bool WindowsWindow::IsVSync() const
//	{
//		return m_Data.VSync;
//	}
//
//}
