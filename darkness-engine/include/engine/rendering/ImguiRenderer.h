#pragma once

#include "shaders/core/imgui/ImguiRender.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"

#include <map>

struct ImGuiContext;

namespace engine
{
    class CommandList;
    struct FlatScene;
    enum class MouseButton;
    struct BoundingSphere;

    void renderBoxes(const FlatScene& scene, const engine::vector<BoundingBox>& boxes, uint32_t color);

    class ImguiRenderer
    {
    public:
        ImguiRenderer(Device& device, ImGuiContext* context);
        void startFrame(
            Device& device,
            float delta);

        void endFrame(
            Device& device,
            TextureRTV currentRenderTarget,
            TextureDSV currentDepthTarget,
            CommandList& cmd);

        void render(const FlatScene& scene);

        
        void renderSpheres(const FlatScene& scene, const engine::vector<BoundingSphere>& spheres, uint32_t color);

        void onMouseMove(int x, int y);
        void onMouseDown(MouseButton button, int x, int y);
        void onMouseUp(MouseButton button, int x, int y);
        void onMouseDoubleClick(MouseButton button, int x, int y);
		void onMouseWheel(int x, int y, int delta);

        bool usesMouse() const;

    private:
        ImGuiContext* m_context;
        engine::Pipeline<shaders::ImguiRender> m_pipeline;
        TextureSRVOwner m_fontAtlas;
        BufferVBVOwner m_vbv;
        BufferIBVOwner m_ibv;

        void recreateBuffersIfNeeded(Device& device, CommandList& cmd, ImVector<ImDrawVert>& vbv, ImVector<ImDrawIdx>& ibv);

        engine::vector<BufferVBVOwner> m_vbvs;
        engine::vector<BufferIBVOwner> m_ibvs;

        std::map<MouseButton, bool> m_mouseButtonStatus;
        std::map<MouseButton, bool> m_mouseButtonWasDownAtleastOnce;
        std::pair<int, int> m_lastKnownMousePosition;
        std::pair<int, int> m_lastKnownMousePositionPrev;
        std::map<MouseButton, std::pair<int, int>> m_lastKnownClickPosition;
		int m_mouseWheelDelta;
    };
}
