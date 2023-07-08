#pragma once

#include "engine/graphics/CommandList.h"
#include "components/Transform.h"
#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/LightComponent.h"
#include "components/Camera.h"
#include "shaders/core/tools/DebugViewer.h"
#include "engine/Scene.h"
#include "engine/rendering/LightData.h"
#include "containers/memory.h"

namespace engine
{
    class DebugView
    {
    public:
        DebugView(Device& device);
        void render(
			CommandList& cmd,
			TextureRTV currentRenderTarget,
            TextureSRV debugSRV);

    private:
        Device& m_device;
        engine::Pipeline<shaders::DebugViewer> m_pipeline;
    };
}
