#pragma once

#include "Color.h"

namespace engine
{

    class Edge
    {
    public:
        engine_565::Color color1, color2;
        int x1, y1, x2, y2;

        Edge(const engine_565::Color& col1, int xp1, int yp1,
             const engine_565::Color& col2, int xp2, int yp2);
    };

}
