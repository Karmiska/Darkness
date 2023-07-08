#pragma once

#include "ui/UiAnchors.h"
#include <limits>

namespace ui
{
    class UiDragable :  public UiAnchors
    {
    public:
        void onMouseMove(int /*x*/, int /*y*/) override;
        void onMouseDown(engine::MouseButton /*button*/, int /*x*/, int /*y*/) override;
        void onMouseUp(engine::MouseButton /*button*/, int /*x*/, int /*y*/) override;

        ui::UiPoint getGlobalPosition(ui::UiPoint point, UiBaseLayer* thisOne = nullptr);

        bool isMoving() const { return m_mouseDown; }
    protected:
        virtual void onDragMove(ui::UiPoint) {};
    private:
        
        bool m_mouseDown = false;
        int m_mousePositionX = 0;
        int m_mousePositionY = 0;
        UiPoint m_mouseDownPosition;
        void forceLegalPosition(ui::UiPoint& point);
    };
}
