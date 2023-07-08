#pragma once

#include "ui/UiRect.h"
#include "engine/graphics/Rect.h"

namespace ui
{
    class UiArea
    {
    public:
        const UiRect area() const { return m_area; }
        const engine::Rectangle clientArea() const { return m_clientArea; }

        UiRect& area() { return m_area; }
        engine::Rectangle& clientArea() { return m_clientArea; }

    private:
        UiRect m_area;
        engine::Rectangle m_clientArea;
    };
}
