#pragma once

#include "engine/graphics/CommandList.h"
#include "components/Transform.h"
#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/LightComponent.h"
#include "components/Camera.h"
#include "shaders/core/tools/Cubemap.h"
#include "shaders/core/tools/EquirectToCubemap.h"
#include "shaders/core/ibl/Irradiance.h"
#include "shaders/core/ibl/PrefilterConvolution.h"
#include "shaders/core/ibl/BrdfConvolution.h"
#include "engine/Scene.h"
#include "engine/rendering/LightData.h"
#include "containers/memory.h"

namespace engine
{
    class RenderCubemap
    {
    public:
        RenderCubemap(Device& device);

        void createCubemapFromEquirect(
            Device& device,
            TextureSRV equirectEnvironmentMap);

        void createIrradianceCubemap(
            Device& device,
            TextureSRV diffuseEnvironmentMap,
            CommandList& cmd);

        void prefilterConvolution(
            Device& device,
            TextureSRV diffuseEnvironmentMap,
            CommandList& cmd);

        void brdfConvolution(Device& device, CommandList& cmd);

        void render(
            TextureRTV rtv,
			TextureDSV dsv,
            Camera& camera,
            CommandList& cmd);

        TextureSRV cubemap() { return m_overrideCubemap ? m_overrideCubemap : m_cubemap; }
		void cubemap(TextureSRV cubemap);
        TextureSRV irradiance() { return m_irradiance; }
        TextureSRV prefilteredEnvironmentMap() { return m_prefilteredEnvironmentMap; }
        TextureSRV brdfConvolution() { return m_brdfConvolution; }

    private:
        engine::Pipeline<shaders::Cubemap> m_cubemapPipeline;
        engine::Pipeline<shaders::EquirectToCubemap> m_equirectToCubemapPipeline;
        engine::Pipeline<shaders::Irradiance> m_irradiancePipeline;
        engine::Pipeline<shaders::PrefilterConvolution> m_prefilterConvolutionPipeline;
        engine::Pipeline<shaders::BrdfConvolution> m_brdfConvolutionPipeline;
        engine::Camera m_cubemapCamera;
        engine::Camera m_cubemapRenderCamera;

		TextureSRV m_overrideCubemap;
        TextureSRVOwner m_cubemap;
        TextureSRVOwner m_irradiance;
        TextureSRVOwner m_prefilteredEnvironmentMap;
        TextureRTVOwner m_prefilteredEnvironmentMapRTV;

        TextureSRVOwner m_brdfConvolution;
        TextureRTVOwner m_brdfConvolutionRTV;

        uint32_t m_lastCubemapSize;
        engine::vector<engine::vector<TextureRTVOwner>> m_cubemapRenderTargets;

        uint32_t m_lastIrradianceSize;
        engine::vector<TextureRTVOwner> m_irradianceRenderTargets;

        uint32_t m_lastPrefilterConvolutionSize;
        engine::vector<engine::vector<TextureRTVOwner>> m_prefilterConvolutionTargets;
    };
}
