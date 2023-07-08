#pragma once

namespace engine
{
    struct Rectangle
    {
        int left;
        int top;
        int right;
        int bottom;

        int width() const
        {
            if (right > left)
                return right - left;
            else
                return left - right;
        }

        int height() const
        {
            if (bottom > top)
                return bottom - top;
            else
                return top - bottom;
        }

        Rectangle intersect(const Rectangle& rect)
        {
            Rectangle result{ left, top, right, bottom };
            result.left = (result.left > rect.left) ? result.left : rect.left;
            result.top = (result.top > rect.top) ? result.top : rect.top;
            result.right = (result.right < rect.right) ? result.right : rect.right;
            result.bottom = (result.bottom < rect.bottom) ? result.bottom : rect.bottom;
            return result;
        }
    };
}
