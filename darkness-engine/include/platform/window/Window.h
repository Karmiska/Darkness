#pragma once

#include "tools/SmartPimpl.h"
#include "platform/Platform.h"
#include "engine/InputEvents.h"
#include <functional>
#include <mutex>

namespace platform
{
    using ResizeCallback = std::function<void(int, int)>;
    
    using MouseMoveCallback = std::function<void(int, int)>;
    using MouseDownCallback = std::function<void(engine::MouseButton, int, int)>;
    using MouseUpCallback = std::function<void(engine::MouseButton, int, int)>;
    using MouseDoubleClickCallback = std::function<void(engine::MouseButton, int, int)>;
	using MouseWheelCallback = std::function<void(int, int, int)>;

    using KeyDownCallback = std::function<void(engine::Key, engine::ModifierState)>;
    using KeyUpCallback = std::function<void(engine::Key, engine::ModifierState)>;

    enum class MouseCursor
    {
        Arrow,
        Ibeam,
        Wait,
        Cross,
        UpArrow,
        SizeNWSE,
        SizeNESW,
        SizeWE,
        SizeNS,
        SizeAll,
        No
    };

    PIMPL_FWD(Window, WindowImpl)
    class Window
    {
    public:
        //Window(const implementation::WindowImpl& implementation);
        Window(WindowHandle handle, int width, int height, bool createOwnWindow);
        Window(const char* windowName, int width, int height);
        bool processMessages();
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

        int width() const;
        int height() const;
        void resize(int width, int height);
		void position(int x, int y);
        void refreshSize();
		WindowHandle native() const;

        MouseCursor mouseCursor() const;
        void mouseCursor(MouseCursor cursor);

        bool quitSignaled() const;

        std::mutex& resizeMutex() { return m_resizeMutex; }
        std::mutex& resizeMutex() const { return m_resizeMutex; }

        PIMPL_FRIEND_ACCESS(WindowImpl)

        
    private:
        mutable std::mutex m_resizeMutex;
    };
}
