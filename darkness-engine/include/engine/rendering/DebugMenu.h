#pragma once

namespace engine
{
    class DebugMenu
    {
    public:
        bool visible() const;
        void visible(bool val);
        void render(int windowWidth, int windowHeight);
    private:
        bool m_visible = true;
        int m_currentRenderMode;
    };
}
