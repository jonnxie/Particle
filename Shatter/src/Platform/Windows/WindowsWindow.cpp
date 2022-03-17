#include "precompiledhead.h"
#include "WindowsWindow.h"

#include <utility>
#include "Engine/Item/shatter_item.h"
#include "Engine/Base/GUI.h"
#include "Engine/Object/inputaction.h"
#include "Engine/Core/Application.h"

namespace Shatter {

	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
        std::cout << error << description << std::endl;
	}

	WindowsWindow::WindowsWindow(WindowProps  props)
    :m_info(std::move(props))
	{
        init();
	}

	WindowsWindow::~WindowsWindow()
	{
        release();
	}

    void WindowsWindow::init() {
        if (s_GLFWWindowCount == 0)
        {
            int success = glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwSetErrorCallback(GLFWErrorCallback);
            setViewPort(UnionViewPort{
                    VkViewport{0, 0,
                               float(m_info.m_width),
                               float(m_info.m_height),
                               0,
                               1.0f}
            });

            setScissor(VkRect2D{
                    VkOffset2D{0,0},
                    VkExtent2D{uint32_t(m_info.m_width), uint32_t(m_info.m_height)}
            });
        }

        {
            m_Window = glfwCreateWindow((int)m_info.m_width, (int)m_info.m_height, m_info.m_title.c_str(), nullptr, nullptr);
            ++s_GLFWWindowCount;
        }

        glfwSetWindowUserPointer(m_Window, this);

        //full
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            if(width == 0 || height == 0) return;

            setViewPort(UnionViewPort{
                    VkViewport{0, 0, float(width), float(height), 0, 1}
            });

            setScissor(VkRect2D{VkOffset2D{0,0},VkExtent2D{uint32_t(width),uint32_t(height)}});

            auto *app = reinterpret_cast<WindowsWindow *>(glfwGetWindowUserPointer(window));

            app->m_callbacks[WindowEvent::Resize]();
        });

        //undone
        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            ImGuiIO& io = ImGui::GetIO();

            if(action == GLFW_PRESS)
            {
                pressKey(key);
                io.KeysDown[key] =  true;
            }else if(action == GLFW_RELEASE)
            {
                io.KeysDown[key] =  false;
                releaseKey(key);
            }

            auto *app = reinterpret_cast<WindowsWindow *>(glfwGetWindowUserPointer(window));
            app->m_application->keyCallback(key,action);
        });

        //done
        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
        {
            ImGuiIO& io = ImGui::GetIO();
            if(keycode > 0 && keycode < 0x10000)
            {
                io.AddInputCharacter((unsigned short)keycode);
            }
            auto *app = reinterpret_cast<WindowsWindow *>(glfwGetWindowUserPointer(window));
            app->m_callbacks[WindowEvent::Char]();
        });


        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
        {
            if(action == GLFW_PRESS)
            {
                double xpos, ypos;
                glfwGetCursorPos(window,&xpos,&ypos);
                pressMouse(button);
                glm::vec2 tmp(xpos/getViewPort().view.width, ypos/getViewPort().view.height);
                tmp *= 2.0f;
                tmp -= 1.0f;
                updateCursorPressPos(tmp);
            }else if(action == GLFW_RELEASE)
            {
                releaseMouse(button);
            }
            auto *app = reinterpret_cast<WindowsWindow *>(glfwGetWindowUserPointer(window));
            app->m_callbacks[WindowEvent::MouseButton]();
        });

        //done
        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            updateScrollPos(glm::vec2(xOffset,yOffset));
            auto *app = reinterpret_cast<WindowsWindow *>(glfwGetWindowUserPointer(window));
            app->m_callbacks[WindowEvent::Scroll]();
        });

        //done
        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            glm::vec2& cursor = input::getCursorWindow();
            cursor.x = xPos;
            cursor.y = yPos;
            VkViewport viewport = getViewPort().view;
            glm::vec2 tmp(xPos/viewport.width,yPos/viewport.height);
            tmp *= 2.0f;
            tmp -= 1.0f;
            updateCursor(tmp);
//            input::cursorWindow(cursor,STATE_IN);
            {
                ImGuiIO& io = ImGui::GetIO();
                bool handled = io.WantCaptureMouse;
            }
            auto *app = reinterpret_cast<WindowsWindow *>(glfwGetWindowUserPointer(window));
            app->m_callbacks[WindowEvent::CursorPos]();
        });
    }

	void WindowsWindow::release()
	{
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}

	void WindowsWindow::update()
	{
		glfwPollEvents();
	}

    bool WindowsWindow::closed() {
        return glfwWindowShouldClose(m_Window);
    }

}
