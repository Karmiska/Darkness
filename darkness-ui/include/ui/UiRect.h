#pragma once

#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector4.h"

namespace ui
{
    using UiPoint = engine::Vector2<int>;

    class UiRect
    {
    public:
        UiRect(int x = 0, int y = 0, int width = 0, int height = 0)
            : m_data{ x, y, width, height }
        {}

        UiPoint position() const { return { m_data.x, m_data.y }; }
        UiPoint size() const { return { m_data.z, m_data.w }; }

        int x() const { return m_data.x; }
        int y() const { return m_data.y; }
        
        int width() const { return m_data.z; }
        int height() const { return m_data.w; }
        
        int left() const { return m_data.x; }
        int top() const { return m_data.y; }
        int right() const { return m_data.x + m_data.z; }
        int bottom() const { return m_data.y + m_data.w; }

        void position(const UiPoint& point)
        {
            m_data.x = point.x;
            m_data.y = point.y;
        }

        void position(int x, int y)
        {
            m_data.x = x;
            m_data.y = y;
        }

        void size(const UiPoint& size)
        {
            m_data.z = size.x;
            m_data.w = size.y;
        }

        void x(int val)
        {
            m_data.x = val;
        }

        void y(int val)
        {
            m_data.y = val;
        }

        void width(int val)
        {
            m_data.z = val;
        }

        void height(int val)
        {
            m_data.w = val;
        }

        void left(int val)
        {
            m_data.x = val;
        }

        void top(int val)
        {
            m_data.y = val;
        }

        void right(int val)
        {
            m_data.z = val - m_data.x;
        }

        void bottom(int val)
        {
            m_data.w = val - m_data.y;
        }

    private:
        engine::Vector4<int> m_data;
    };
}
