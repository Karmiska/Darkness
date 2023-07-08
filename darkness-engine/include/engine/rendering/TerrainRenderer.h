#pragma once

#include "engine/graphics/Resources.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/rendering/LightData.h"
#include "shaders/core/terrain/Terrain.h"
#include "shaders/core/terrain/RefreshTerrainClusters.h"
#include "engine/rendering/TerrainSettings.h"
#include "containers/vector.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/renderers/RenderersCommon.h"
#include "shaders/core/terrain/TerrainClustersForward.h"
#include "shaders/core/terrain/TerrainClustersForwardJitterEqual.h"
#include "shaders/core/terrain/TerrainClustersDepthDefault.h"
#include "shaders/core/terrain/TerrainClustersDepthJitter.h"

namespace engine
{
    namespace image
    {
        class ImageIf;
    }

    /*struct TerrainDefinition
    {
        float3 position;
        float3 sizeMeters;
    };

    struct TerrainCell
    {
    };

    struct TerrainSector
    {
        uint width;
        uint height;

    };

    class Terrain
    {
    public:
        Terrain(const TerrainDefinition& definition);
    private:
        TerrainDefinition m_terrainDefinition;

        uint3 cellCount(const TerrainDefinition& terrainDefinition);
        uint calculateTerrainMipLevels(const TerrainDefinition& terrainDefinition);
    };*/

    class TerrainCell
    {
    public:
        TerrainCell(Device& device, uint size);

        BufferIBV indexes()
        {
            return m_indexes;
        }

        BufferSRV vertexes()
        {
            return m_vertexes;
        }

		ModelResourceAllocation& index()
		{
			return m_index;
		}

		ModelResourceAllocation vertex()
		{
			return m_vertex;
		}

		VertexScale vertexScale() const
		{
			return m_vertexScale;
		}
    private:
        void buildIndexDataStrip(Device& device, uint size);
		void buildIndexDataList(Device& device, uint size);
        void buildVertexData(Device& device, uint size);
        uint m_size;
        BufferIBVOwner m_indexes;
        BufferSRVOwner m_vertexes;
        
		ModelResourceAllocation m_index;
		ModelResourceAllocation m_vertex;

		VertexScale m_vertexScale;
    };

    class Device;
    class Camera;
    class CommandList;

	enum class TerrainPipelineMode
	{
		Default,
		Debug
	};

    class TerrainRenderer
    {
    public:
        TerrainRenderer(Device& device);
        
        void updateTerrainPatches(
			CommandList& cmd,
			const Camera& camera,
			bool terrainAllDebugBoundingBoxes);

        void render(
            CommandList& cmd,
            Camera& camera,
            TextureRTV target,
            TextureDSV dsv,
            TextureSRV shadowMap,
            BufferSRV shadowVP,
            BufferSRV lightIndexToShadowIndex,
            TextureSRV ssao,
            Vector3f probePosition,
            float probeRange,
            uint32_t debugMode,
            const Vector2<int>& virtualResolution,
            LightData& lights,
            BufferSRV lightBins,
            bool drawTerrain,
            bool drawWireframe,
			const Vector3f& location);

		void renderForward(
			CommandList& cmd,
			ClusterDataLine& clusters,
			IndexDataLine& indexes,
			DrawDataLine& draws,
			TextureDSV depth,
			TextureRTV rtv,
			TextureRTV motion,
			TextureSRV shadowMap,
			BufferSRV shadowVP,
			BufferSRV lightIndexToShadowIndex,
			TextureSRV ssao,
			Camera& camera,
			Vector3f probePosition,
			float probeRange,
			JitterOption jitter,
			DepthTestOption depthTest,
			uint32_t debugMode,
			const Vector2<int>& virtualResolution,
			LightData& lights,
			BufferSRV lightBins,
			const Matrix4f& previousCameraViewMatrix,
			const Matrix4f& previousCameraProjectionMatrix,
			int terrainDebugMode);

		void renderDepth(
			CommandList& cmd,
			ClusterDataLine& clusters,
			IndexDataLine& indexes,
			DrawDataLine& draws,
			TextureDSV depth,
			Camera& camera,
			JitterOption jitter,
			BiasOption bias,
			const Vector2<int>& virtualResolution);

		void settings(const TerrainSettings& settings)
		{
			m_settings = settings;
		}

		TerrainSettings settings() const
		{
			return m_settings;
		}

		void updateTransform(const Matrix4f& mat);

		uint32_t clusterCount() const;
    private:
        Device& m_device;
        TerrainCell m_cell;
		size_t m_cellWidth;
		size_t m_cellHeight;
		size_t m_cellCount;
        engine::Pipeline<shaders::Terrain> m_terrainPipeline;
        engine::Pipeline<shaders::Terrain> m_terrainWireframePipeline;
		engine::Pipeline<shaders::RefreshTerrainClusters> m_refreshTerrainPipeline;
		engine::Pipeline<shaders::RefreshTerrainClusters> m_refreshTerrainPipelineDebug;
        
		// depth
		engine::Pipeline<shaders::TerrainClustersDepthDefault> m_default;
		engine::Pipeline<shaders::TerrainClustersDepthJitter> m_jitter;
		engine::Pipeline<shaders::TerrainClustersDepthDefault> m_defaultBias;
		engine::Pipeline<shaders::TerrainClustersDepthJitter> m_jitterBias;

		// forward
		engine::Pipeline<shaders::TerrainClustersForward> m_defaultGreaterEqual;
		engine::Pipeline<shaders::TerrainClustersForward> m_defaultGreaterEqualDebug;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterGreaterEqual;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterGreaterEqualDebug;
		engine::Pipeline<shaders::TerrainClustersForward> m_defaultEqual;
		engine::Pipeline<shaders::TerrainClustersForward> m_defaultEqualDebug;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterEqual;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterEqualDebug;

		engine::Pipeline<shaders::TerrainClustersForward> m_defaultGreaterEqualWireframe;
		engine::Pipeline<shaders::TerrainClustersForward> m_defaultGreaterEqualDebugWireframe;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterGreaterEqualWireframe;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterGreaterEqualDebugWireframe;
		engine::Pipeline<shaders::TerrainClustersForward> m_defaultEqualWireframe;
		engine::Pipeline<shaders::TerrainClustersForward> m_defaultEqualDebugWireframe;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterEqualWireframe;
		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterEqualDebugWireframe;

		engine::Pipeline<shaders::TerrainClustersForwardJitterEqual> m_jitterEqualFlipped;

		// gbuffer

		engine::Sampler m_heightSampler;

        engine::shared_ptr<image::ImageIf> m_heightMapImage;
        TextureSRVOwner m_heightMapSRV;

        engine::shared_ptr<image::ImageIf> m_colorMapImage;
        TextureSRVOwner m_colorMapSRV;
        
		TerrainSettings m_settings;

		ModelResource m_clusters;
		ModelResource m_submesh;
		ModelResource m_lods;
		engine::shared_ptr<SubMeshInstance> m_instance;
		size_t m_terrainInstanceCount;
		size_t m_terrainClusterCount;
		engine::shared_ptr<ModelResources::ModelAllocation> m_subMeshAllocation;
		Matrix4f m_lastMatrix;
		bool m_matrixUpdate;
		InstanceMaterial m_terrainMaterial;
    };
}
