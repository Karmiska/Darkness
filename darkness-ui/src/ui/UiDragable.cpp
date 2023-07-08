#include "ui/UiDragable.h"
#include "ui/UiState.h"

namespace ui
{
    ui::UiPoint UiDragable::getGlobalPosition(ui::UiPoint point, UiBaseLayer* thisOne)
    {
        if (!thisOne)
            thisOne = this;
        point += dynamic_cast<UiAnchors*>(thisOne)->position();
        if (dynamic_cast<UiDragable*>(thisOne)->parent())
            point = getGlobalPosition(point, dynamic_cast<UiBaseLayer*>(dynamic_cast<UiDragable*>(thisOne)->parent()));
        return point;
    }

    void UiDragable::forceLegalPosition(ui::UiPoint& point)
    {
        if (m_maximumPosition.x != InvalidPosition &&
            point.x > m_maximumPosition.x) point.x = m_maximumPosition.x;
        if (m_minimumPosition.x != InvalidPosition &&
            point.x < m_minimumPosition.x) point.x = m_minimumPosition.x;
        if (m_maximumPosition.y != InvalidPosition &&
            point.y > m_maximumPosition.y) point.y = m_maximumPosition.y;
        if (m_minimumPosition.y != InvalidPosition &&
            point.y < m_minimumPosition.y) point.y = m_minimumPosition.y;
    }

    void UiDragable::onMouseMove(int x, int y)
    {
        if (m_canMove == ui::AllowedMovement::None)
            return;
        auto gp = getGlobalPosition({ x, y }, this);
        if (m_mouseDown)
        {
            if (m_canMove == ui::AllowedMovement::All)
            {
                auto newPos = m_mouseDownPosition + UiPoint(
                    gp.x - m_mousePositionX,
                    gp.y - m_mousePositionY);
                forceLegalPosition(newPos);
                position(newPos);
                onDragMove(newPos);
            }
            else if (m_canMove == ui::AllowedMovement::Vertical)
            {
                auto newPos = m_mouseDownPosition + UiPoint(0, gp.y - m_mousePositionY);
                forceLegalPosition(newPos);
                position(newPos);
                onDragMove(newPos);
            }
            else if (m_canMove == ui::AllowedMovement::Horizontal)
            {
                auto newPos = m_mouseDownPosition + UiPoint(gp.x - m_mousePositionX, 0);
                forceLegalPosition(newPos);
                position(newPos);
                onDragMove(newPos);
            }
        }
        //if(gp.x >= m_minimumPosition.x && gp.x <= m_maximumPosition.x)
        //    m_mousePositionX = gp.x;
        //if (gp.y >= m_minimumPosition.y && gp.y <= m_maximumPosition.y)
        //    m_mousePositionY = gp.y;
    }

    void UiDragable::onMouseDown(engine::MouseButton button, int x, int y)
    {
        if (m_canMove == ui::AllowedMovement::None)
            return;
        auto gp = getGlobalPosition({ x, y }, this);
        if (button == engine::MouseButton::Left)
        {
            m_mouseDown = true;
            ui::dragFrame = this;
        }
        m_mousePositionX = gp.x;
        m_mousePositionY = gp.y;
        m_mouseDownPosition = position();
    }

    void UiDragable::onMouseUp(engine::MouseButton button, int x, int y)
    {
        if (m_canMove == ui::AllowedMovement::None)
            return;
        auto gp = getGlobalPosition({ x, y }, this);
        if (button == engine::MouseButton::Left)
        {
            m_mouseDown = false;
            ui::dragFrame = nullptr;
        }
        m_mousePositionX = gp.x;
        m_mousePositionY = gp.y;
    }
}
