#pragma once

#include "containers/vector.h"
#include "containers/string.h"
#include <chrono>
#include "imgui.h"

namespace engine
{
    class LogWindow
    {
    public:
        LogWindow();
        void pushMessages(const engine::vector<engine::string>& messages);
        void render(int windowWidth, int windowHeight);

    private:
        engine::vector<engine::string> m_messages;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_lastShaderChangeUpdate;
        ImGuiTextBuffer m_buf;
        ImGuiTextFilter m_filter;
        ImVector<int> m_lineOffsets;
    };
}
