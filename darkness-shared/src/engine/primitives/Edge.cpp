#include "engine/primitives/Edge.h"

namespace engine
{

Edge::Edge(const engine_565::Color& col1, int xp1, int yp1,
           const engine_565::Color& col2, int xp2, int yp2)
{
    if(yp1 < yp2)
    {
        color1 = col1;
        x1 = xp1;
        y1 = yp1;
        color2 = col2;
        x2 = xp2;
        y2 = yp2;
    }
    else
    {
        color1 = col2;
        x1 = xp2;
        y1 = yp2;
        color2 = col1;
        x2 = xp1;
        y2 = yp1;
    }
}

}
