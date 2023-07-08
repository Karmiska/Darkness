#pragma once

#include "shaders/core/model/ClusterRender.h"
#include "shaders/core/model/ClusterShadowRenderer.h"
#include "engine/graphics/Pipeline.h"
#include "engine/rendering/culling/ModelDataLine.h"

namespace engine
{
	class Device;
	class CommandList;
	class ModelResources;
	class GBuffer;
	class TextureDSV;

	struct ClusterRendererArguments
	{
		CommandList& cmd;
		Camera* camera;

		GBuffer* gBuffer;
		TextureDSV* depthBuffer;
		uint64_t frameNumber;
		Vector2<int> virtualResolution;

		// temporal stuff
		Matrix4f previousViewMatrix;
		Matrix4f previousProjectionMatrix;

		int currentRenderMode;
		bool zPrepass = false;
		bool debugCamera = false;
		bool alphaclipped = false;
		bool gbufferEqual = false;
	};

	class ClusterRenderer
	{
	public:
		ClusterRenderer(Device& device);

		void render(
			ClusterRendererArguments args,
			ClusterDataLine& clusters,
			IndexDataLine& indexes,
			DrawDataLine& draws);

	private:
		Device & m_device;
		engine::Pipeline<shaders::ClusterRender> m_fullRender;
		engine::Pipeline<shaders::ClusterRender> m_fullEqualRender;
		engine::Pipeline<shaders::ClusterRender> m_gbufferRender;
		engine::Pipeline<shaders::ClusterRender> m_debugCamerafullRender;
		engine::Pipeline<shaders::ClusterRender> m_debugCameragbufferRender;
		engine::Pipeline<shaders::ClusterShadowRenderer> m_depthRender;
		engine::Pipeline<shaders::ClusterRender> m_clusterDebugRender;
		engine::Pipeline<shaders::ClusterRender> m_zPrepass;
		engine::Pipeline<shaders::ClusterRender> m_alphaclipped;
		engine::Pipeline<shaders::ClusterRender> m_alphaclippedShadow;
	};
}
