#pragma once

#include "engine/graphics/Pipeline.h"
#include "engine/graphics/ResourceOwners.h"
#include "shaders/core/voxel/Voxelize.h"
#include "shaders/core/voxel/ClearVoxelGrids.h"
#include "shaders/core/voxel/CreateVoxelGridDebugDraws.h"
#include "shaders/core/voxel/CreateVoxelGridDebugDrawsFromGrid.h"
#include "shaders/core/voxel/CreateVoxelDebugDrawArgs.h"
#include "shaders/core/voxel/DrawVoxelDebug.h"
#include "shaders/core/voxel/VoxelListDispatchArgs.h"
#include "shaders/core/voxel/DownsampleVoxels.h"
#include "shaders/core/voxel/LightVoxels.h"
#include "shaders/core/voxel/DrawVoxelGridDebugCube.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/culling/FrustumCuller.h"
#include "engine/rendering/culling/IndexExpansion.h"

namespace engine
{
	class Device;
	class CommandList;
	class TextureRTV;
	class Camera;
	class LightData;

	class VoxelFrustumCull;

	class VoxelGrid
	{
	public:
		VoxelGrid(
			Device& device, 
			int resolution,
			const Vector3f& position,
			const Vector3f& size,
			int voxelMips = -1);

		void resolution(int gridResolution);
		int resolution() const;

		void position(const Vector3f& position);
		Vector3f position() const;

		void size(const Vector3f& size);
		Vector3f size() const;

		void clear(CommandList& cmd);

		void voxelize(
			Device& device, 
			CommandList& cmd,
			BufferSRV clusters,
			BufferIBV indexes,
			Buffer indirectArgs,
			Buffer count);

		void light(
			CommandList& cmd,
			TextureSRV shadowMap,
			BufferSRV shadowVP,
			BufferSRV lightIndexToShadowIndex,
			LightData& lights);

		engine::vector<TextureSRV>& voxels() { return m_voxels; }
		engine::vector<TextureSRVOwner>& voxelsOwner() { return m_voxelsSRV; }
		TextureSRV normals() { return m_normals; };
		BufferSRV colors() { return m_colors; };
	private:
		engine::Pipeline<shaders::Voxelize> m_voxelize;
		engine::Pipeline<shaders::ClearVoxelGrids> m_clearVoxelGrids;
		engine::Pipeline<shaders::DownsampleVoxels> m_downsampleVoxels;
		engine::Pipeline<shaders::LightVoxels> m_lightVoxels;
		engine::Pipeline<shaders::VoxelListDispatchArgs> m_createVoxelListDispatchArgs;
		int m_resolution;
		Vector3f m_position;
		Vector3f m_size;
		int m_mips;

		void refreshBuffers(Device& device, int resolution);
		void voxelize(
			Device& device, 
			CommandList& cmd, 
			BufferSRV clusters,
			BufferIBV indexes,
			Buffer indirectArgs,
			Buffer count,
			const Camera& camera,
			const Vector3f& cameraDirection,
			int mode);
		engine::vector<TextureUAVOwner> m_voxelsUAV;
		engine::vector<TextureSRVOwner> m_voxelsSRV;

		TextureUAVOwner m_normalsUAV;
		TextureSRVOwner m_normalsSRV;

		BufferUAVOwner m_colorsUAV;
		BufferSRVOwner m_colorsSRV;

		BufferUAVOwner m_voxelListUAV;
		BufferSRVOwner m_voxelListSRV;

		CountBuffers m_voxelListCount;
		BufferUAVOwner m_voxelListDispatchArgs;

		engine::vector<TextureSRV> m_voxels;
		TextureSRV m_normals;
		BufferSRV m_colors;
	};

	class VoxelFrustumCull
	{
	public:
		VoxelFrustumCull(Device& device);

		void cullScene(
			Device& device, 
			VoxelGrid& voxelGrid,
			CommandList& cmd);

		BufferSRV clusters();
		BufferIBV indexes();
		Buffer indirectArgs();
		Buffer counts();

	private:
		FrustumCuller m_frustumCuller;
		IndexExpansion m_indexExpansion;
		ClusterDataLine m_countBuffer;
		ClusterDataLine m_drawOutput;
		IndexDataLine m_indexDataLine;
		IndexDataLine m_indexDataLine2;
		DrawDataLine m_drawDataLine;
	};

	class SceneVoxelizer
	{
	public:
		SceneVoxelizer(Device& device);

		void voxelize(
			CommandList& cmd,
			const Camera& camera);

		void createDebugVoxelData(
			CommandList& cmd,
			const Camera& camera,
			int debugVoxelMip);

		void lightVoxels(
			CommandList& cmd,
			TextureSRV shadowMap,
			BufferSRV shadowVP,
			BufferSRV lightIndexToShadowIndex,
			LightData& lights
		);

		void renderDebug(
			CommandList& cmd,
			const Camera& camera,
			TextureRTV target,
			TextureDSV dsv,
			bool debugVoxelGrids);

		void clearDebug();

		engine::vector<VoxelGrid>& grids()
		{
			return m_grids;
		}
	private:
		Device& m_device;
		engine::Pipeline<shaders::CreateVoxelGridDebugDraws> m_createVoxelGridDebugDraws;
		engine::Pipeline<shaders::CreateVoxelGridDebugDrawsFromGrid> m_createVoxelGridDebugDrawsFromGrid;
		engine::Pipeline<shaders::CreateVoxelDebugDrawArgs> m_createVoxelDebugDrawArgs;
		engine::Pipeline<shaders::DrawVoxelDebug> m_drawVoxelDebug;
		engine::Pipeline<shaders::DrawVoxelGridDebugCube> m_drawVoxelGridDebug;
		engine::Pipeline<shaders::DrawVoxelGridDebugCube> m_drawVoxelGridDebugWireframe;

		VoxelFrustumCull m_voxelFrustumCull;
		engine::vector<VoxelGrid> m_grids;

		

		BufferUAVOwner m_debugVertexUAV;
		BufferSRVOwner m_debugVertexSRV;

		BufferUAVOwner m_debugColorUAV;
		BufferSRVOwner m_debugColorSRV;

		BufferUAVOwner m_debugIndexUAV;
		BufferIBVOwner m_debugIndexIBV;

		BufferUAVOwner m_debugAllocationsUAV;
		BufferSRVOwner m_debugAllocationsSRV;

		BufferOwner m_debugDrawArgs;
		BufferUAVOwner m_debugDrawArgsUAV;

		
	};
}
