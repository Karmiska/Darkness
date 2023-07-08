#include "engine/rendering/ViewportRenderer.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "engine/primitives/Vector2.h"
#include "engine/graphics/Common.h"

#include "components/Camera.h"

namespace engine
{
    ViewportRenderer::ViewportRenderer(
        Device& device,
        int width,
        int height)
        : m_device{ device }
        , m_rtv{ device.createTextureRTV(TextureDescription()
            .width(width)
            .height(height)
            .format(engine::Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("main render target")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f })
        ) }
        , m_depthPyramid{ device, width, height }
        , m_srv{ device.createTextureSRV(m_rtv) }

        , m_modelRenderer(device, Vector2<int>{ width, height })
        , m_shadowRenderer(device)
        , m_renderCubemap(device)
        , m_postProcess(device)
		, m_ssrDebugPhase{ 0 }
		, m_debugBoundingBoxes{ device }
    {
    }

	Vector2<int> ViewportRenderer::virtualResolution() const
	{
		return m_modelRenderer.virtualResolution();
	}

	bool ViewportRenderer::virtualResolutionChange(bool reset)
	{
		return m_modelRenderer.virtualResolutionChange(reset);
	}

	void ViewportRenderer::render(
		CommandList& cmd,
		FlatScene& scene,
		TextureRTV destination,
		unsigned int mouseX,
		unsigned int mouseY)
	{
		if (!scene.validScene())
			return;

		auto & camera = *scene.cameras[scene.selectedCamera];
        camera.jitteringEnabled(m_modelRenderer.taaEnabled());

		{
			CPU_MARKER(cmd.api(), "Clear viewport rtv");
			GPU_MARKER(cmd, "Clear viewport rtv");
			cmd.clearRenderTargetView(m_rtv, { 0.0f, 0.0f, 0.0f, 1.0f });
		}

		{
			CPU_MARKER(cmd.api(), "Render shadows");
			GPU_MARKER(cmd, "Render shadows");
			if (scene.lightData->changeHappened())
				m_shadowRenderer.refresh();
			m_shadowRenderer.render(cmd, scene);
		}

		if (!(*m_modelRenderer.forwardRendering()))
		{
			m_modelRenderer.render(
                m_device, 
                m_depthPyramid, 
                cmd, 
                scene, 
                m_shadowRenderer.shadowMap(),
                m_shadowRenderer.shadowVP(), 
                m_shadowRenderer.lightIndexToShadowIndex(),
                *scene.lightData);
			m_modelRenderer.renderLightBins(m_device, cmd, m_depthPyramid, scene, *scene.lightData);
#ifndef _DURANGO
			m_modelRenderer.renderPicker(cmd, mouseX, mouseY);
#endif
			m_modelRenderer.renderSSAO(m_device, cmd, m_depthPyramid, scene);
			m_modelRenderer.renderSSR(m_device, cmd, m_depthPyramid, scene);
			m_modelRenderer.renderLighting(
				cmd, scene, m_depthPyramid, m_shadowRenderer.shadowMap(), 
                m_shadowRenderer.shadowVP(), m_shadowRenderer.lightIndexToShadowIndex());

		}
		else
		{
			//m_modelRenderer.renderSSAOForward(m_device, cmd, m_depthPyramid, camera);
			m_modelRenderer.renderForward(m_device, m_depthPyramid, cmd, scene,
				m_shadowRenderer.shadowMap(), m_shadowRenderer.shadowVP(), m_shadowRenderer.lightIndexToShadowIndex(), *scene.lightData);

#ifndef _DURANGO
			m_modelRenderer.renderPicker(cmd, mouseX, mouseY);
#endif
		}

		{
			CPU_MARKER(cmd.api(), "Render environment cubemap");
			GPU_MARKER(cmd, "Render environment cubemap");
            if(scene.cameraDebugging())
			    m_renderCubemap.render(m_modelRenderer.lightingTargetRTV(), m_modelRenderer.debugDepth(), scene.drawCamera(), cmd);
            else
                m_renderCubemap.render(m_modelRenderer.lightingTargetRTV(), m_depthPyramid.dsv(), scene.drawCamera(), cmd);
		}

#ifdef PARTICLE_TEST_ENABLED
        m_modelRenderer.renderParticles(
            m_device,
            cmd,
            m_modelRenderer.lightingTargetRTV(),
            m_depthPyramid.srv(),
            scene.drawCamera(),
            *scene.lightData,
            m_shadowRenderer.shadowMap(), m_shadowRenderer.shadowVP());
#endif

        m_modelRenderer.renderFrameDownsample(cmd);
        m_modelRenderer.renderTransparent(
            m_device, m_depthPyramid, 
            cmd, scene, 
            m_shadowRenderer.shadowMap(), m_shadowRenderer.shadowVP());
        
        m_modelRenderer.renderTerrain(
            cmd,
            scene,
            m_modelRenderer.lightingTargetRTV(),
            m_depthPyramid.dsv(),
            m_shadowRenderer.shadowMap(), 
            m_shadowRenderer.shadowVP(), 
            m_shadowRenderer.lightIndexToShadowIndex(), 
            *scene.lightData);

        m_modelRenderer.renderDebugVoxels(
            cmd,
            scene,
            m_modelRenderer.lightingTargetRTV(),
            m_depthPyramid.dsv()
        );

		if(m_modelRenderer.debugBoundingBoxes())
			m_debugBoundingBoxes.render(cmd, m_modelRenderer.lightingTargetRTV(),
				m_depthPyramid.dsv(), scene.drawCamera(), virtualResolution());

        m_modelRenderer.renderOutline(m_device, cmd, scene, m_depthPyramid, scene.drawCamera());
        m_modelRenderer.renderTemporalResolve(cmd, m_depthPyramid, scene.drawCamera());
		
		if (*m_modelRenderer.forwardRendering())
			m_modelRenderer.renderSSRForward(m_device, cmd, m_depthPyramid, scene.drawCamera());
		
        m_modelRenderer.flip();

        if(scene.postprocess)
        {
            CPU_MARKER(cmd.api(), "Render postprocess");
            GPU_MARKER(cmd, "Render postprocess");
            m_postProcess.render(destination, m_modelRenderer.finalFrame(), cmd, *scene.postprocess, m_modelRenderer.histogramDebug());
        }

		m_modelRenderer.renderSSRDebug(cmd, destination, m_depthPyramid, m_ssrDebugPhase);
    }

	TextureSRV ViewportRenderer::ssrResult()
	{
		return m_modelRenderer.ssrResult();
	}

    FrameStatistics ViewportRenderer::getStatistics()
    {
        return m_modelRenderer.getStatistics();
    }

    void ViewportRenderer::refresh(engine::Vector2<int> virtualResolution)
    {
        m_rtv = m_device.createTextureRTV(TextureDescription()
            .width(virtualResolution.x)
            .height(virtualResolution.y)
            .format(engine::Format::R16G16B16A16_FLOAT)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("main render target")
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 1.0f })
        );
        m_srv = m_device.createTextureSRV(m_rtv);
        m_depthPyramid.refresh(virtualResolution);
        m_modelRenderer.resize(static_cast<uint32_t>(virtualResolution.x), static_cast<uint32_t>(virtualResolution.y));
    }

    unsigned int ViewportRenderer::pickedObject(Device& device)
    {
        return m_modelRenderer.pickedObject(device);
    }

    void ViewportRenderer::setSelectedObject(int64_t object)
    {
        m_modelRenderer.setSelectedObject(object);
    }

	void ViewportRenderer::setSSRDebugMousePosition(int x, int y)
	{
		m_modelRenderer.setSSRDebugMousePosition(x, y);
	}

}