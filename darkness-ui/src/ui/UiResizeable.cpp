#include "ui/UiResizeable.h"
#include "ui/Frame.h"
#include "platform/window/Window.h"
#include "ui/UiState.h"

namespace ui
{
    void UiResizeable::onMouseMove(int x, int y)
    {
        //if (!canReceiveMouseMessages())
        //    return;

        auto frame = dynamic_cast<Frame*>(this);
        auto window = frame->windowShared().get();
        auto resizePart = inResizeArea(x, y);

        if (!m_canResize)
        {
            UiDragable::onMouseMove(x, y);
            window->mouseCursor(platform::MouseCursor::Arrow);
            return;
        }

        if (!m_mouseGrabbed && (resizePart == ResizePart::MiddleCenter || resizePart == ResizePart::None))
        {
            window->mouseCursor(platform::MouseCursor::Arrow);
            UiDragable::onMouseMove(x, y);
        }
        else
        {
            if (m_mouseGrabbed)
            {
                window->mouseCursor(m_grabCursor);
                auto gp = getGlobalPosition({ x, y }, this);
                auto delta = gp - UiPoint{ m_mousePositionX, m_mousePositionY };
                m_mousePositionX = gp.x;
                m_mousePositionY = gp.y;

                auto newLeftWidth = [&]()->int { return width() + (delta.x * -1); };
                auto newRightWidth = [&]()->int { return width() + delta.x; };
                auto newTopHeight = [&]()->int { return height() + (delta.y * -1); };
                auto newBottomHeight = [&]()->int { return height() + delta.y; };
                auto newLeftLeft = [&]()->int { return left() + delta.x; };
                auto newTopTop = [&]()->int { return top() + delta.y; };

                switch (m_grabPart)
                {
                    case ResizePart::TopLeft:
                    {
                        if (newLeftWidth() < m_minimumSize.x) delta.x = (right() - m_minimumSize.x) - left();
                        if (newTopHeight() < m_minimumSize.y) delta.y = (bottom() - m_minimumSize.y) - top();

                        left(newLeftLeft());
                        top(newTopTop());
                        width(newLeftWidth());
                        height(newTopHeight());
                        break;
                    }
                    case ResizePart::TopCenter:
                    {
                        if (newTopHeight() < m_minimumSize.y) delta.y = (bottom() - m_minimumSize.y) - top();

                        top(newTopTop());
                        height(newTopHeight());
                        break;
                    }
                    case ResizePart::TopRight:
                    {
                        if (newTopHeight() < m_minimumSize.y) delta.y = (bottom() - m_minimumSize.y) - top();
                        if (newRightWidth() < m_minimumSize.x) delta.x = (right() - m_minimumSize.x) - left();

                        top(newTopTop());
                        width(newRightWidth());
                        height(newTopHeight());
                        break;
                    }
                    case ResizePart::MiddleLeft:
                    {
                        if (newLeftWidth() < m_minimumSize.x) delta.x = (right() - m_minimumSize.x) - left();

                        left(newLeftLeft());
                        width(newLeftWidth());
                        break;
                    }
                    case ResizePart::MiddleCenter:
                    {
                        break;
                    }
                    case ResizePart::MiddleRight:
                    {
                        if (newRightWidth() < m_minimumSize.x) delta.x = (right() - m_minimumSize.x) - left();

                        width(newRightWidth());
                        break;
                    }
                    case ResizePart::BottomLeft:
                    {
                        if (newBottomHeight() < m_minimumSize.y) delta.y = (bottom() - m_minimumSize.y) - top();
                        if (newLeftWidth() < m_minimumSize.x) delta.x = (right() - m_minimumSize.x) - left();

                        left(newLeftLeft());
                        width(newLeftWidth());
                        height(newBottomHeight());
                        break;
                    }
                    case ResizePart::BottomCenter:
                    {
                        if (newBottomHeight() < m_minimumSize.y) delta.y = (bottom() - m_minimumSize.y) - top();

                        height(newBottomHeight());
                        break;
                    }
                    case ResizePart::BottomRight:
                    {
                        if (newBottomHeight() < m_minimumSize.y) delta.y = (bottom() - m_minimumSize.y) - top();
                        if (newRightWidth() < m_minimumSize.x) delta.x = (right() - m_minimumSize.x) - left();

                        height(newBottomHeight());
                        width(newRightWidth());
                        break;
                    }

                }
            }
            else
            {
                if(ui::dragFrame != nullptr && ui::dragFrame != this || isMoving())
                { }
                else
                {
                    window->mouseCursor(cursorFromResizePart(resizePart));
                }
            }
        }
    }

    void UiResizeable::onMouseDown(engine::MouseButton button, int x, int y)
    {
        if (!m_canResize)
        {
            UiDragable::onMouseDown(button, x, y);
            return;
        }

        auto part = inResizeArea(x, y);
        if (button == engine::MouseButton::Left && part != ResizePart::MiddleCenter && part != ResizePart::None)
        {
            m_mouseGrabbed = true;
            m_grabCursor = cursorFromResizePart(part);
            m_grabPart = part;
            auto gp = getGlobalPosition({ x, y }, this);
            m_mousePositionX = gp.x;
            m_mousePositionY = gp.y;
            ui::dragFrame = this;
        }
        else
            UiDragable::onMouseDown(button, x, y);
    }

    void UiResizeable::onMouseUp(engine::MouseButton button, int x, int y)
    {
        if (!m_canResize)
        {
            UiDragable::onMouseUp(button, x, y);
            return;
        }

        if (button == engine::MouseButton::Left && m_mouseGrabbed)
        {
            m_mouseGrabbed = false;
            ui::dragFrame = nullptr;
        }
        else
            UiDragable::onMouseUp(button, x, y);
    }

    UiResizeable::ResizePart UiResizeable::inResizeArea(int x, int y)
    {
        // top left corner
        if (x >= 0 && x < ResizeBorderSize && y >= 0 && y < ResizeBorderSize)
            return ResizePart::TopLeft;

        // top bar
        else if (x >= ResizeBorderSize && x < width() - ResizeBorderSize && y >= 0 && y < ResizeBorderSize)
            return ResizePart::TopCenter;

        // top right corner
        else if (x >= width() - ResizeBorderSize && x < width() && y >= 0 && y < ResizeBorderSize)
            return ResizePart::TopRight;

        // left bar
        else if (x >= 0 && x < ResizeBorderSize && y >= ResizeBorderSize && y < height() - ResizeBorderSize)
            return ResizePart::MiddleLeft;

        // center
        else if (x >= ResizeBorderSize && x < width() - ResizeBorderSize && y >= ResizeBorderSize && y < height() - ResizeBorderSize)
            return ResizePart::MiddleCenter;

        // right bar
        else if (x >= width() - ResizeBorderSize && x < width() && y >= ResizeBorderSize && y < height() - ResizeBorderSize)
            return ResizePart::MiddleRight;

        // bottom left corner
        else if (x >= 0 && x < ResizeBorderSize && y >= height() - ResizeBorderSize && y < height())
            return ResizePart::BottomLeft;

        // bottom bar
        else if (x >= ResizeBorderSize && x < width() - ResizeBorderSize && y >= height() - ResizeBorderSize && y < height())
            return ResizePart::BottomCenter;

        // bottom right corner
        else if (x >= width() - ResizeBorderSize && x < width() && y >= height() - ResizeBorderSize && y < height())
            return ResizePart::BottomRight;

        return ResizePart::None;
    }

    platform::MouseCursor UiResizeable::cursorFromResizePart(ResizePart part)
    {
        switch (part)
        {
            case ResizePart::TopLeft: return platform::MouseCursor::SizeNWSE;
            case ResizePart::TopCenter: return platform::MouseCursor::SizeNS;
            case ResizePart::TopRight: return platform::MouseCursor::SizeNESW;
            case ResizePart::MiddleLeft: return platform::MouseCursor::SizeWE;
            case ResizePart::MiddleCenter: return platform::MouseCursor::Arrow;
            case ResizePart::MiddleRight: return platform::MouseCursor::SizeWE;
            case ResizePart::BottomLeft: return platform::MouseCursor::SizeNESW;
            case ResizePart::BottomCenter: return platform::MouseCursor::SizeNS;
            case ResizePart::BottomRight: return platform::MouseCursor::SizeNWSE;
            case ResizePart::None: return platform::MouseCursor::No;
        }
        return platform::MouseCursor::No;
    }
}
