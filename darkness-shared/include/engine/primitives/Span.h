#pragma once

#include "Color.h"

namespace engine
{

class Span
{
public:
    Color color1, color2;
    int x1, x2;

    Span(const Color& col1, int xp1, const Color& col2, int xp2)
    {
        if(xp1 < xp2)
        {
            color1 = col1;
            x1 = xp1;
            color2 = col2;
            x2 = xp2;
        }
        else
        {
            color1 = col2;
            x1 = xp2;
            color2 = col1;
            x2 = xp1;
        }
    }
};

}
