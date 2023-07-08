#include "engine/rendering/DebugMenu.h"
#include "imgui.h"

namespace engine
{
    bool DebugMenu::visible() const
    {
        return m_visible;
    }

    void DebugMenu::visible(bool val)
    {
        m_visible = val;
    }

    void DebugMenu::render(int /*windowWidth*/, int /*windowHeight*/)
    {
#if 0
        float w = static_cast<float>(windowWidth);
        float h = static_cast<float>(windowHeight);

        float menuWidth = w;
        float menuHeight = 300;

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(10.0f, 10.0f), 
            ImVec2(menuWidth, menuHeight));

        if (ImGui::Begin("DebugMenu", &m_visible, io.DisplaySize, 0, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoInputs | 
            ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoFocusOnAppearing | 
            ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            ImGui::SetWindowSize(ImVec2(menuWidth, menuHeight), ImGuiSetCond_Always);
            ImGui::SetWindowPos(ImVec2(0.0f, h - menuHeight));
            //ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));

            ImGui::BeginChild("scrolling", ImVec2(menuWidth, menuHeight));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

            ImGui::TextUnformatted("");

            ImGui::SetScrollHere(1.0f);
            ImGui::PopStyleVar();
            ImGui::EndChild();

        }
        ImGui::End();
        /*ImGui::BeginMenu("DebugMenu", m_visible);
        if (ImGui::TreeNode("Rendering"))
        {
            ImGui::Combo("Mode", &m_currentRenderMode, "Full0DebugClusters0Albedo0Roughness0Metalness0Occlusion0Uv0DebugNormal");
            ImGui::TreePop();
        }
        ImGui::EndMenu();*/
        
#endif

#if 0
        bool open{ true };

        float w = static_cast<float>(windowWidth);
        float h = static_cast<float>(windowHeight);

        float logWidth = 150.0f;
        float logHeight = 70.0f;

        ImGui::SetNextWindowSizeConstraints(ImVec2(10.0f, 10.0f), ImVec2(logWidth, logHeight));
        if (ImGui::Begin("DebugMenu", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::SetWindowSize(ImVec2(logWidth, logHeight), ImGuiSetCond_Always);
            ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));

            if (ImGui::TreeNode("Rendering"))
            {
                ImGui::Combo("Mode", &m_currentRenderMode, "Full\0DebugClusters\0Albedo\0Roughness\0Metalness\0Occlusion\0Uv\0DebugNormal\0");
                ImGui::TreePop();
            }
        }
        ImGui::End();
        
#endif
    }
}
