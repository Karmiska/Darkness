#include "platform/window/durango/DurangoWindow.h"
#include "tools/Debug.h"

namespace platform
{
	namespace implementation
	{
		WindowImpl::WindowImpl(WindowHandle handle, int width, int height)
			: m_window{ handle }
			, m_width(width)
			, m_height(height)
			, m_onResize{ nullptr }
		{
		}

		WindowImpl::WindowImpl(const WindowImpl& /*impl*/)
		{
		}

		WindowImpl::WindowImpl(const char* /*windowName*/, int width, int height)
			: m_width(width)
			, m_height(height)
		{
		}

		void WindowImpl::refreshSize()
		{
		}

		bool WindowImpl::processMessages() const
		{
			return true;
		}

		void WindowImpl::setResizeCallback(ResizeCallback onResize)
		{
			m_onResize = onResize;
		}

		void WindowImpl::setMouseCallbacks(
			MouseMoveCallback           onMouseMove,
			MouseDownCallback           onMouseDown,
			MouseUpCallback             onMouseUp,
			MouseDoubleClickCallback    onMouseDoubleClick,
			MouseWheelCallback			onMouseWheel)
		{
			m_onMouseMove = onMouseMove;
			m_onMouseDown = onMouseDown;
			m_onMouseUp = onMouseUp;
			m_onMouseDoubleClick = onMouseDoubleClick;
			m_onMouseWheel = onMouseWheel;
		}

		void WindowImpl::setKeyboardCallbacks(
			KeyDownCallback onKeyDown,
			KeyUpCallback   onKeyUp)
		{
			m_onKeyDown = onKeyDown;
			m_onKeyUp = onKeyUp;
		}

		int WindowImpl::width() const
		{
			return m_width;
		}

		int WindowImpl::height() const
		{
			return m_height;
		}

		void WindowImpl::resize(int width, int height)
		{
			ASSERT(false, "Cant");
			m_width = width;
			m_height = height;
		}

		WindowHandle WindowImpl::native() const
		{
			return m_window;
		}
	}
}
