#pragma once

#ifndef _DURANGO

#include "containers/string.h"
#include "platform/window/Window.h"
#include "platform/Platform.h"

namespace platform
{
    namespace implementation
    {
        class WindowImpl
        {
        public:
            WindowImpl(HWND handle, int width, int height, bool createOwnWindow);
            WindowImpl(const char* windowName, int width, int height);
            
			
			WindowImpl(const WindowImpl& impl) = delete;
			WindowImpl& operator=(const WindowImpl&) = delete;
			WindowImpl(WindowImpl&&) = delete;
			WindowImpl& operator=(WindowImpl&&) = delete;
            
			
			~WindowImpl();

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
			void position(int x, int y);
            const HWND handle() const;

            void refreshSize();

            WindowHandle native() const;

            MouseCursor mouseCursor() const;
            void mouseCursor(MouseCursor cursor);

            bool quitSignaled() const { return m_quitSignaled; };
        public:
            void setHandle(HWND handle);
            LRESULT handleMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
            HINSTANCE m_hInstance;
        private:
			mutable bool m_quitSignaled;
            int m_width, m_height;
            HWND m_windowHandle;
			HWND m_parentWindowHandle;
            MouseCursor m_mouseCursor;

            ResizeCallback m_onResize;
			bool m_mainWindow;
			engine::string m_windowName;

            MouseMoveCallback           m_onMouseMove;
            MouseDownCallback           m_onMouseDown;
            MouseUpCallback             m_onMouseUp;
            MouseDoubleClickCallback    m_onMouseDoubleClick;
			MouseWheelCallback			m_onMouseWheel;

            KeyDownCallback m_onKeyDown;
            KeyUpCallback   m_onKeyUp;

            bool m_createdFromHandle;
            void createWindow(HWND parentWindow, const char* windowName, int width, int height);

            bool m_isMaximized;
            bool m_isMinimized;
            bool m_isResizing;
			bool m_disableNotification;
            void conditionalRefresSize(int width, int height);

            bool handleSizeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
            bool handleKeyboardMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
            bool handleMouseMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
            bool handleCursorMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

            engine::Key engineKeyFromWindows(WPARAM wParam, LPARAM lParam) const;
            engine::ModifierState modifierState() const;

            void createCursors();
            HCURSOR m_cursors[11];
            int m_lastMouseX;
            int m_lastMouseY;
        };
    }
}
#endif
