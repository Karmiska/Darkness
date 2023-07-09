#pragma once

#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "shaders/core/debug/DebugBoundingBoxInstanceCollect.h"
#include "shaders/core/debug/DebugBoundingBoxClusterExpandArgs.h"
#include "shaders/core/debug/DebugBoundingBoxInstanceToClusterExpand.h"
#include "shaders/core/debug/DebugBoundingBox.h"
#include "shaders/core/debug/DebugBoundingBoxCreateDrawArgs.h"

namespace engine
{
	class Device;
	class CommandList;
	class TextureRTV;
	class Camera;
	class DepthPyramid;

	class DebugBoundingBoxes
	{
	public:
		DebugBoundingBoxes(Device& device);

		void render(
			CommandList& cmd,
			TextureRTV rtv,
			TextureDSV dsv,
			const Camera& camera,
			Vector2<int> virtualResolution);

	private:
		Device& m_device;
		engine::Pipeline<shaders::DebugBoundingBoxInstanceCollect> m_instanceCollect;
		engine::Pipeline<shaders::DebugBoundingBoxClusterExpandArgs> m_instanceClusterExpandArgs;
		engine::Pipeline<shaders::DebugBoundingBoxInstanceToClusterExpand> m_instanceToClusterExpand;
		engine::Pipeline<shaders::DebugBoundingBox> m_debugBoundingBox;
		engine::Pipeline<shaders::DebugBoundingBoxCreateDrawArgs> m_createDebugDrawArgs;

		BufferUAVOwner  m_instanceCountUAV;
		BufferSRVOwner  m_instanceCountSRV;
		BufferUAVOwner  m_allocationUAV;
		BufferUAVOwner  m_allocationSingleUAV;
		BufferUAVOwner  m_frustumCullingOutputUAV;
		BufferSRVOwner  m_frustumCullingOutputSRV;
		CountBuffers    m_clusterCount;

		ClusterDataLine	m_clusters;
		DrawDataLine m_clusterDraw;

		BufferOwner      m_clusterExpandDispatchArgs;
		BufferUAVOwner   m_clusterExpandDispatchArgsUAV;
	};
}
