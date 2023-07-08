#pragma once

#include "platform/window/Window.h"
#include "platform/Platform.h"

namespace platform
{
	namespace implementation
	{
		class WindowImpl
		{
		public:
			WindowImpl(WindowHandle window, int width, int height);
			WindowImpl(const char* windowName, int width, int height);
			WindowImpl(const WindowImpl& impl);

			void setResizeCallback(ResizeCallback onResize);
			void setMouseCallbacks(
				MouseMoveCallback           onMouseMove,
				MouseDownCallback           onMouseDown,
				MouseUpCallback             onMouseUp,
				MouseDoubleClickCallback    onMouseDoubleClick,
				MouseWheelCallback			onMouseWheel
			);
			void setKeyboardCallbacks(
				KeyDownCallback onKeyDown,
				KeyUpCallback   onKeyUp
			);

			bool processMessages() const;
			int width() const;
			int height() const;
			void resize(int width, int height);

			void refreshSize();

			WindowHandle native() const;

		private:
			WindowHandle m_window;
			int m_width, m_height;
			ResizeCallback m_onResize;

			MouseMoveCallback           m_onMouseMove;
			MouseDownCallback           m_onMouseDown;
			MouseUpCallback             m_onMouseUp;
			MouseDoubleClickCallback    m_onMouseDoubleClick;
			MouseWheelCallback			m_onMouseWheel;

			KeyDownCallback m_onKeyDown;
			KeyUpCallback   m_onKeyUp;
		};
	}
}
