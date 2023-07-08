#pragma once

namespace ui
{
    enum class AllowedMovement
    {
        None,
        Vertical,
        Horizontal,
        All
    };
    const int InvalidPosition = 2147483647;

    class UiRestrictions
    {
    public:
        void canMove(AllowedMovement val) { m_canMove = val; };
        AllowedMovement canMove() const { return m_canMove; }
        void setMinimumPosition(const ui::UiPoint& minPosition) { m_minimumPosition = minPosition; }
        void setMaximumPosition(const ui::UiPoint& maxPosition) { m_maximumPosition = maxPosition; }
        ui::UiPoint minimumSize() const { return m_minimumSize; }
        void minimumSize(const ui::UiPoint& size) { m_minimumSize = size; }
    protected:
        AllowedMovement m_canMove = AllowedMovement::All;
        ui::UiPoint m_minimumPosition{ InvalidPosition, InvalidPosition };
        ui::UiPoint m_maximumPosition{ InvalidPosition, InvalidPosition };
        ui::UiPoint m_minimumSize{ 0, 0 };
    };
}
