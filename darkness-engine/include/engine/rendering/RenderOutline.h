#pragma once

#include "engine/graphics/CommandList.h"
#include "components/Transform.h"
#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/LightComponent.h"
#include "components/Camera.h"
#include "shaders/core/outline/Outline.h"
#include "shaders/core/outline/CreateOutlineIndirectIndexedDrawArgs.h"
#include "engine/graphics/Sampler.h"
#include "engine/Scene.h"
#include "engine/rendering/LightData.h"
#include "containers/memory.h"
#include <map>

namespace engine
{
    class ModelResources;
    class RenderOutline
    {
    public:
        RenderOutline(
            Device& device, 
            Vector2<int> virtualResolution);

        void render(
            Device& device,
            TextureRTV currentRenderTarget,
            TextureSRV depthBuffer,
            Camera& camera,
            ModelResources& modelResources,
            CommandList& cmd,
            FlatSceneNode& model
        );

    private:
        engine::Pipeline<shaders::Outline> m_outlinePipeline;
        engine::Pipeline<shaders::CreateOutlineIndirectIndexedDrawArgs> m_createOutlineIndirectIndexedDrawArgs;
        Vector2<int> m_virtualResolution;
        Sampler m_depthSampler;
        BufferOwner m_indexedDrawArguments;
        BufferUAVOwner m_indexedDrawArgumentsUAV;
    };
}
