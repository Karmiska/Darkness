#pragma once

#include "ui/UiDragable.h"
#include "ui/UiRect.h"

namespace platform
{
    enum class MouseCursor;
}

namespace ui
{
    class Frame;

    constexpr int ResizeBorderSize = 5;
    class UiResizeable : public UiDragable
    {
    public:
        void onMouseMove(int, int) override;
        void onMouseDown(engine::MouseButton /*button*/, int /*x*/, int /*y*/) override;
        void onMouseUp(engine::MouseButton /*button*/, int /*x*/, int /*y*/) override;

        void canResize(bool val) { m_canResize = val; };
        bool canResize() const { return m_canResize; }

        
    private:
        enum class ResizePart
        {
            TopLeft, TopCenter, TopRight,
            MiddleLeft, MiddleCenter, MiddleRight,
            BottomLeft, BottomCenter, BottomRight,
            None
        };
        
        bool m_canResize = false;
        bool m_mouseGrabbed = false;
        int m_mousePositionX = 0;
        int m_mousePositionY = 0;
        
        ResizePart m_grabPart;
        platform::MouseCursor m_grabCursor;
        
        ResizePart inResizeArea(int x, int y);
        platform::MouseCursor cursorFromResizePart(ResizePart part);
    };
}
