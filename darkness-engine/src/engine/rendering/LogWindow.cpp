#include "engine/rendering/LogWindow.h"
#include <sstream>
#include <iomanip>

namespace engine
{
    LogWindow::LogWindow()
    {
        //ImGuiStyle& style = ImGui::GetStyle();
        //style.WindowRounding = 0;
        //style.FrameRounding = 0;
        //style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
    }

    // disable warning about localtime. it's by no means deprecated. visual studio is an idiot.
    #pragma warning( push )
    #pragma warning( disable : 4996 )
    void LogWindow::pushMessages(const engine::vector<engine::string>& messages)
    {
        m_lastShaderChangeUpdate = std::chrono::high_resolution_clock::now();
        m_messages.reserve(m_messages.size() + std::distance(messages.begin(), messages.end()));
        m_messages.insert(m_messages.end(), messages.begin(), messages.end());

        auto now = std::chrono::system_clock::now();
        auto inTime = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&inTime), "%H:%M:%S");

        int oldSize = m_buf.size();
        for (auto&& message : m_messages)
        {
            m_buf.appendf((engine::string(ss.str().c_str()) + ": " + message + '\n').c_str());
            //m_buf.append((ss.str() + ": " + message + '\n').c_str());
        }
        for (int new_size = m_buf.size(); oldSize < new_size; oldSize++)
            if (m_buf[oldSize] == '\n')
                m_lineOffsets.push_back(oldSize);

        m_messages.clear();
    }
    #pragma warning( pop )

    void LogWindow::render(int windowWidth, int windowHeight)
    {
        bool open{ true };

        float w = static_cast<float>(windowWidth);
        float h = static_cast<float>(windowHeight);

        float logWidth = w;
        float logHeight = 170.0f;

        ImGui::SetNextWindowSizeConstraints(ImVec2(10.0f, 10.0f), ImVec2(logWidth, logHeight));
        if (ImGui::Begin("log", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::SetWindowSize(ImVec2(logWidth, logHeight), ImGuiCond_Always);
            ImGui::SetWindowPos(ImVec2(0.0f, h - logHeight));

            ImGui::BeginChild("scrolling");
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

            ImGui::TextUnformatted(m_buf.begin());

            ImGui::SetScrollHereY(1.0f);
            ImGui::PopStyleVar();
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
