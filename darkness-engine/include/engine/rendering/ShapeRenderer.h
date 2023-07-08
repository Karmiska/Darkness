#pragma once

#include "engine/graphics/Pipeline.h"
#include "shaders/core/shape/ClearLightBins.h"
#include "shaders/core/shape/ShapeRender.h"
#include "shaders/core/shape/RenderCones.h"
#include "shaders/core/shape/RenderSpheres.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/primitives/Matrix4.h"
#include "engine/rendering/ShapeMeshFactory.h"
#include "tools/ToolsCommon.h"

namespace engine
{
	class Device;
	class CommandList;
	class TextureRTV;
	class Camera;
	class DepthPyramid;
	struct GpuShape;
	class LightData;

	template<typename T>
	T binSize(T size)
	{
		return std::max(roundUpToMultiple(size, static_cast<T>(8)) / static_cast<T>(8), static_cast<T>(1));
	}

	constexpr int MaxLightsPerBin = 256;
	constexpr int SpotSectors = 30;

	class ShapeRenderer
	{
	public:
		ShapeRenderer(Device& device);

		void clearBins(CommandList& cmd, const Camera& camera);
		void render(
			CommandList& cmd, 
			DepthPyramid& depthPyramid,
			const Camera& camera,
			const Matrix4f& jitterViewProjection,
			LightData& lights);

		BufferSRV lightBins();

	private:
		Device& m_device;
		engine::Pipeline<shaders::ClearLightBins> m_clearLightBins;
		engine::Pipeline<shaders::ShapeRender> m_shapeRender;
		engine::Pipeline<shaders::RenderCones> m_renderCones;
        engine::Pipeline<shaders::RenderSpheres> m_renderSpheres;

		bool createLightBins(int width, int height);
		BufferUAVOwner m_lightBins;
		BufferSRVOwner m_lightBinsSRV;
		BufferSRV m_lightBinsSRVView;

		TextureDSVOwner m_shapeDSV;
		TextureDSV m_shapeDSVView;

		TextureRTVOwner m_shapeRTV;
		TextureRTV m_shapeRTVView;

		Shape m_cpusphere;
		Shape m_cpucone;

		GpuShape m_sphere;
		GpuShape m_cone;

		BufferSRVOwner m_lightParameters;
		BufferSRVOwner m_spotLightRanges;
		BufferSRVOwner m_spotLightIds;
		BufferSRVOwner m_spotLightTransforms;

		TextureRTVOwner m_pointlightBinTexture;
		void createPointlightBinTexture(size_t width, size_t height);
	};
}
