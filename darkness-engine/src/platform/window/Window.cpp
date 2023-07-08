#include "platform/window/Window.h"
#ifdef _WIN32
#ifndef _DURANGO
#include "platform/window/windows/WindowsWindow.h"
#else
#include "platform/window/durango/DurangoWindow.h"
#endif
#endif
#ifdef __APPLE__
#include "OsxWindow.h"
#endif

using namespace platform::implementation;

namespace platform
{
    PIMPL_ACCESS_IMPLEMENTATION(Window, WindowImpl)
    
    /*Window::Window(const implementation::WindowImpl& implementation)
        : m_impl{ tools::make_unique_impl<WindowImpl>(implementation) }
    {
    }*/

    Window::Window(WindowHandle handle, int width, int height, bool createOwnWindow)
        : m_impl{ tools::make_unique_impl<WindowImpl>(handle, width, height, createOwnWindow) }
    {}

    Window::Window(const char* windowName, int width, int height)
        : m_impl{ tools::make_unique_impl<WindowImpl>(windowName, width, height) }
    {}

    bool Window::processMessages()
    {
        std::lock_guard<std::mutex> lock(m_resizeMutex);
        return m_impl->processMessages();
    }

    void Window::setResizeCallback(ResizeCallback onResize)
    {
        m_impl->setResizeCallback(onResize);
    }

    void Window::setMouseCallbacks(
        MouseMoveCallback           onMouseMove,
        MouseDownCallback           onMouseDown,
        MouseUpCallback             onMouseUp,
        MouseDoubleClickCallback    onMouseDoubleClick,
		MouseWheelCallback			onMouseWheel)
    {
        m_impl->setMouseCallbacks(onMouseMove, onMouseDown, onMouseUp, onMouseDoubleClick, onMouseWheel);
    }

    void Window::setKeyboardCallbacks(
        KeyDownCallback onKeyDown,
        KeyUpCallback   onKeyUp)
    {
        m_impl->setKeyboardCallbacks(onKeyDown, onKeyUp);
    }

    int Window::width() const
    {
        return m_impl->width();
    }

    int Window::height() const
    {
        return m_impl->height();
    }

    void Window::resize(int width, int height)
    {
        m_impl->resize(width, height);
    }

	void Window::position(int x, int y)
	{
		m_impl->position(x, y);
	}

    void Window::refreshSize()
    {
        m_impl->refreshSize();
    }

	WindowHandle Window::native() const
	{
		return m_impl->native();
	}

    MouseCursor Window::mouseCursor() const
    {
        return m_impl->mouseCursor();
    }

    void Window::mouseCursor(MouseCursor cursor)
    {
        m_impl->mouseCursor(cursor);
    }

    bool Window::quitSignaled() const
    {
        return m_impl->quitSignaled();
    }
}
