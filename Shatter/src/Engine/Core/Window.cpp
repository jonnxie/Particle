#include "precompiledhead.h"
#include "Window.h"
#include "Platform/Windows/WindowsWindow.h"

namespace Shatter {
    void Window::setEventCallback(eventCallback _callback) {
        m_callbacks.insert(_callback);
    }

    std::unique_ptr<Window> Window::createWindow(const WindowProps &props) {
        #ifdef SHATTER_PLATPORM_WINDOWS
        ////		return std::make_unique<WindowsWindow>(props);
        #endif
    }


}
