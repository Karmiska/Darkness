#include "engine/rendering/TerrainRenderer.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/rendering/ShapeRenderer.h"
#include "platform/Environment.h"
#include "engine/rendering/ModelCpu.h"
#include <algorithm>

namespace engine
{
	/*Terrain::Terrain(const TerrainDefinition& definition)
		: m_terrainDefinition{ definition }
	{
	}

	uint Terrain::calculateTerrainMipLevels(const TerrainDefinition& terrainDefinition)
	{
		return mipCount(
			terrainDefinition.cellCount.x, 
			terrainDefinition.cellCount.y, 
			terrainDefinition.cellCount.z);
	}*/

	TerrainCell::TerrainCell(Device& device, uint size)
		: m_size{ size }
	{
		buildVertexData(device, size+1);
		//buildIndexDataStrip(device, size+1);
		buildIndexDataList(device, size + 1);
	}

	void TerrainCell::buildIndexDataStrip(Device& device, uint size)
	{
		int strips = size - 1;
		int degenerates = 2 * (strips - 1);
		int vertexPerString = 2 * size;
		int indexCount = (vertexPerString * strips) + degenerates;

		engine::vector<uint16_t> indexes(indexCount);

		int index = 0;
		for (uint y = 0; y < size - 1; ++y)
		{
			if (y > 0)
				indexes[index++] = static_cast<uint16_t>(y * size);

			for (uint x = 0; x < size; ++x)
			{
				indexes[index++] = static_cast<uint16_t>((y * size) + x);
				indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + x);
			}

			if (y < size - 2)
			{
				indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + (size - 1));
			}
		}

		m_index = device.modelResources().gpuBuffers().indexAllocator().allocate(indexCount);
		auto upload = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().index().buffer(), m_index.gpuIndex * sizeof(uint16_t), &indexes[0], indexes.size() * sizeof(uint16_t));
		upload.blockUntilUploaded();
#if 0
		// allocate GPU index data
		m_index = device.modelResources().gpuBuffers().indexAllocator().allocate(indexCount);

		// create upload allocation
		auto uploadAllocation = device.modelResources().residency().createUpdateAllocation(indexes.size() * sizeof(uint16_t));
		uploadAllocation.gpuIndex = m_index.gpuIndex;
		uploadAllocation.buffer = device.modelResources().gpuBuffers().index();
		memcpy(uploadAllocation.ptr, &indexes[0], indexes.size() * sizeof(uint16_t));

		// upload data
		device.modelResources().residency().makeResident(uploadAllocation);
		device.modelResources().residency().freeUpdateAllocation(uploadAllocation);
#endif
		m_indexes = device.createBufferIBV(BufferDescription()
			.usage(ResourceUsage::GpuReadWrite)
			.format(Format::R16_UINT)
			.name("Terrain cell index buffer")
			.elements(indexes.size())
			.elementSize(formatBytes(Format::R16_UINT))
			.setInitialData(BufferDescription::InitialData(indexes)));
	}

	void TerrainCell::buildIndexDataList(Device& device, uint size)
	{
		int strips = size - 1;
		int indexCount = strips * strips * 6;

		engine::vector<uint16_t> indexes(indexCount);
		int index = 0;
		for (int y = 0; y < strips; ++y)
		{
			for (int x = 0; x < strips; ++x)
			{
				if (x % 2 == y % 2)
				{
					indexes[index++] = static_cast<uint16_t>((y * size) + x);
					indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + x);
					indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + (x + 1));

					indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + (x + 1));
					indexes[index++] = static_cast<uint16_t>((y * size) + (x + 1));
					indexes[index++] = static_cast<uint16_t>((y * size) + x);
				}
				else
				{
					indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + x);
					indexes[index++] = static_cast<uint16_t>((y * size) + (x + 1));
					indexes[index++] = static_cast<uint16_t>((y * size) + x);

					indexes[index++] = static_cast<uint16_t>((y * size) + (x + 1));
					indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + x);
					indexes[index++] = static_cast<uint16_t>(((y + 1) * size) + (x + 1));
				}
			}
		}

		// allocate GPU index data
		m_index = device.modelResources().gpuBuffers().indexAllocator().allocate(indexCount);
		auto upload = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().index().buffer(), m_index.gpuIndex * sizeof(uint16_t), &indexes[0], indexes.size() * sizeof(uint16_t));
		upload.blockUntilUploaded();
#if 0
		m_index = device.modelResources().gpuBuffers().indexAllocator().allocate(indexCount);

		// create upload allocation
		auto uploadAllocation = device.modelResources().residency().createUpdateAllocation(indexes.size() * sizeof(uint16_t));
		uploadAllocation.gpuIndex = m_index.gpuIndex;
		uploadAllocation.buffer = device.modelResources().gpuBuffers().index();
		memcpy(uploadAllocation.ptr, &indexes[0], indexes.size() * sizeof(uint16_t));

		// upload data
		device.modelResources().residency().makeResident(uploadAllocation);
		device.modelResources().residency().freeUpdateAllocation(uploadAllocation);
#endif

		m_indexes = device.createBufferIBV(BufferDescription()
			.usage(ResourceUsage::GpuReadWrite)
			.format(Format::R16_UINT)
			.name("Terrain cell index buffer")
			.elements(indexes.size())
			.elementSize(formatBytes(Format::R16_UINT))
			.setInitialData(BufferDescription::InitialData(indexes)));
	}

	void TerrainCell::buildVertexData(Device& device, uint size)
	{
		engine::vector<float3> vertexes(size * size);

		int offset = 0;
		for (uint y = 0; y < size; y++)
		{
			for (uint x = 0; x < size; x++)
			{
				vertexes[offset++] = { static_cast<float>(x), 0.0f, static_cast<float>(y) };
			}
		}

		m_vertexScale.origo = { 0.0f, 0.0f, 0.0f };
		m_vertexScale.range = { static_cast<float>(size), 1.0f, static_cast<float>(size) };

		auto packedVertexData = packVertexBuffer(vertexes, m_vertexScale);

		// allocate GPU index data
		m_vertex = device.modelResources().gpuBuffers().vertexDataAllocator().allocate(packedVertexData.size());
		auto upload = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().vertex().buffer(), m_vertex.gpuIndex * sizeof(Vector2<uint32_t>), &packedVertexData[0], packedVertexData.size() * sizeof(Vector2<uint32_t>));
		upload.blockUntilUploaded();
#if 0
		m_vertex = device.modelResources().gpuBuffers().vertexDataAllocator().allocate(packedVertexData.size());

		// create upload allocation
		auto uploadAllocation = device.modelResources().residency().createUpdateAllocation(packedVertexData.size() * sizeof(Vector2<uint32_t>));
		uploadAllocation.gpuIndex = m_vertex.gpuIndex;
		uploadAllocation.buffer = device.modelResources().gpuBuffers().vertex();
		memcpy(uploadAllocation.ptr, &packedVertexData[0], packedVertexData.size() * sizeof(Vector2<uint32_t>));

		// upload data
		device.modelResources().residency().makeResident(uploadAllocation);
		device.modelResources().residency().freeUpdateAllocation(uploadAllocation);
#endif

		m_vertexes = device.createBufferSRV(BufferDescription()
			.usage(ResourceUsage::GpuReadWrite)
			.structured(true)
			.elements(vertexes.size())
			.elementSize(sizeof(float3))
			.name("Terrain cell vertex buffer")
			.setInitialData(BufferDescription::InitialData(vertexes)));
	}

	template<typename T>
	void setupTerrainPipeline(
		Device& device,
		T& pipeline,
		DepthTestOption depthTest,
		DepthStencilOpDescription front,
		DepthStencilOpDescription back,
		bool cullBack,
		TextureSRV heightMap,
		TextureSRV colorMap,
		Sampler heightSampler,
		bool wireframe,
		TerrainPipelineMode mode = TerrainPipelineMode::Default)
	{
		pipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		pipeline.setRasterizerState(RasterizerDescription().cullMode(cullBack ? CullMode::Back : CullMode::None).fillMode(wireframe ? FillMode::Wireframe : FillMode::Solid));
		pipeline.vs.debug = mode == TerrainPipelineMode::Debug;
		pipeline.vs.heightMapSampler = heightSampler;
		pipeline.vs.heightmap = heightMap;
		pipeline.ps.debug = mode == TerrainPipelineMode::Debug;
		pipeline.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true).depthWriteMask(depthTest == DepthTestOption::GreaterEqual ? DepthWriteMask::All : DepthWriteMask::Zero)
			.depthFunc(depthTest == DepthTestOption::GreaterEqual ? ComparisonFunction::GreaterEqual : ComparisonFunction::Equal)
			.frontFace(front).backFace(back));
		pipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
		pipeline.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
		pipeline.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
		pipeline.ps.shadow_sampler = device.createSampler(SamplerDescription()
			.addressU(TextureAddressMode::Mirror)
			.addressV(TextureAddressMode::Mirror)
			.filter(Filter::Comparison));
		pipeline.ps.heightmap = heightMap;
		pipeline.ps.colormap = colorMap;
	};

	TerrainRenderer::TerrainRenderer(Device& device)
		: m_device{ device }
		, m_cell{ device, 10 }
		, m_cellWidth{ 100 }
		, m_cellHeight{ 100 }
		, m_cellCount{ m_cellWidth * m_cellHeight }
		, m_terrainPipeline{ device.createPipeline<shaders::Terrain>() }
		, m_terrainWireframePipeline{ device.createPipeline<shaders::Terrain>() }
		, m_refreshTerrainPipeline{ device.createPipeline<shaders::RefreshTerrainClusters>() }
		, m_refreshTerrainPipelineDebug{ device.createPipeline<shaders::RefreshTerrainClusters>() }

		, m_default{ device.createPipeline<shaders::TerrainClustersDepthDefault>() }
		, m_jitter{ device.createPipeline<shaders::TerrainClustersDepthJitter>() }
		, m_defaultBias{ device.createPipeline<shaders::TerrainClustersDepthDefault>() }
		, m_jitterBias{ device.createPipeline<shaders::TerrainClustersDepthJitter>() }

		, m_defaultGreaterEqual{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_defaultGreaterEqualDebug{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_jitterGreaterEqual{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }
		, m_jitterGreaterEqualDebug{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }
		, m_defaultEqual{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_defaultEqualDebug{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_jitterEqual{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }
		, m_jitterEqualDebug{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }

		, m_defaultGreaterEqualWireframe{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_defaultGreaterEqualDebugWireframe{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_jitterGreaterEqualWireframe{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }
		, m_jitterGreaterEqualDebugWireframe{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }
		, m_defaultEqualWireframe{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_defaultEqualDebugWireframe{ device.createPipeline<shaders::TerrainClustersForward>() }
		, m_jitterEqualWireframe{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }
		, m_jitterEqualDebugWireframe{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }

		, m_jitterEqualFlipped{ device.createPipeline<shaders::TerrainClustersForwardJitterEqual>() }

		, m_heightSampler{ device.createSampler(SamplerDescription().textureAddressMode(TextureAddressMode::Border).borderColor(0.0f, 0.0f, 0.0f, 0.0f).filter(Filter::Point)) }

		//, m_heightMapImage{ device.createImage("C:\\Users\\Aleksi Jokinen\\Darkness\\processed\\heightmaps\\test_terrain_01_32bit.tif") }
		, m_heightMapImage{ image::Image::createImage(
			pathClean(pathJoin(getExecutableDirectory(), engine::string("..\\..\\..\\..\\..\\darkness-engine\\data\\test_terrain_02_16bit.tif"))), 
			image::ImageType::EXTERNAL) }
		, m_heightMapSRV{ device.createTextureSRV(TextureDescription()
				.name("Terrain heightmap")
				.width(static_cast<uint32_t>(m_heightMapImage->width()))
				.height(static_cast<uint32_t>(m_heightMapImage->height()))
				.format(m_heightMapImage->format())
				.arraySlices(static_cast<uint32_t>(m_heightMapImage->arraySlices()))
				.mipLevels(m_heightMapImage->mipCount() == 0 ? 1 : static_cast<uint32_t>(m_heightMapImage->mipCount()))
				.setInitialData(TextureDescription::InitialData(
					tools::ByteRange(
						m_heightMapImage->data(),
						m_heightMapImage->data() + m_heightMapImage->bytes()),
					static_cast<uint32_t>(m_heightMapImage->width()), static_cast<uint32_t>(m_heightMapImage->width()* m_heightMapImage->height())))) }
		, m_colorMapImage{ device.createImage("C:\\Users\\aleks\\Documents\\TestDarknessProject\\processed\\heightmaps\\test_terrain_02_D2.png") }
		, m_colorMapSRV{ device.createTextureSRV(TextureDescription()
				.name("Terrain color")
				.width(static_cast<uint32_t>(m_colorMapImage->width()))
				.height(static_cast<uint32_t>(m_colorMapImage->height()))
				.format(m_colorMapImage->format())
				.arraySlices(static_cast<uint32_t>(m_colorMapImage->arraySlices()))
				.mipLevels(m_colorMapImage->mipCount() == 0 ? 1 : static_cast<uint32_t>(m_colorMapImage->mipCount()))
				.setInitialData(TextureDescription::InitialData(
					tools::ByteRange(
						m_colorMapImage->data(),
						m_colorMapImage->data() + m_colorMapImage->bytes()),
					static_cast<uint32_t>(m_colorMapImage->width()), static_cast<uint32_t>(m_colorMapImage->width()* m_colorMapImage->height())))) }
		, m_terrainInstanceCount{ 1 }
		, m_terrainClusterCount{ m_cellCount }
		, m_matrixUpdate{ true }
	{
		DepthStencilOpDescription front;
		front.StencilFailOp = StencilOp::Keep;
		front.StencilDepthFailOp = StencilOp::Incr;
		front.StencilPassOp = StencilOp::Keep;
		front.StencilFunc = ComparisonFunction::Always;

		DepthStencilOpDescription back;
		back.StencilFailOp = StencilOp::Keep;
		back.StencilDepthFailOp = StencilOp::Decr;
		back.StencilPassOp = StencilOp::Keep;
		back.StencilFunc = ComparisonFunction::Always;

		auto setupPipeline = [&](engine::Pipeline<shaders::Terrain>& pipeline)
		{
			//pipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
			pipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
			pipeline.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
			pipeline.setDepthStencilState(DepthStencilDescription()
				.depthEnable(true)
				.depthWriteMask(DepthWriteMask::All)
				.depthFunc(ComparisonFunction::GreaterEqual)
				.frontFace(front)
				.backFace(back));
			pipeline.vs.vertices = m_cell.vertexes();
			pipeline.vs.heightMapSampler = m_heightSampler;
			pipeline.vs.colorMapSampler = device.createSampler(SamplerDescription().textureAddressMode(TextureAddressMode::Clamp).filter(Filter::Point));
			pipeline.vs.heightmap = m_heightMapSRV;
			pipeline.vs.colormap = m_colorMapSRV;

			pipeline.ps.heightmap = m_heightMapSRV;
			pipeline.ps.colormap = m_colorMapSRV;
			pipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear).textureAddressMode(TextureAddressMode::Wrap));
			pipeline.ps.tri_sampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
			pipeline.ps.point_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));
			pipeline.ps.shadow_sampler = device.createSampler(SamplerDescription()
				.addressU(TextureAddressMode::Mirror)
				.addressV(TextureAddressMode::Mirror)
				.filter(Filter::Comparison));
		};
		setupPipeline(m_terrainPipeline);
		m_terrainPipeline.vs.heightBias = 0.0f;

		setupPipeline(m_terrainWireframePipeline);
		m_terrainWireframePipeline.vs.heightBias = 0.1f;

		// setup depth
		m_default.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_default.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_default.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_default.vs.heightMapSampler = m_heightSampler;
		m_default.vs.heightmap = m_heightMapSRV;


		m_jitter.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_jitter.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_jitter.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_jitter.vs.heightMapSampler = m_heightSampler;
		m_jitter.vs.heightmap = m_heightMapSRV;

		m_defaultBias.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_defaultBias.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_defaultBias.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_defaultBias.setRasterizerState(RasterizerDescription()
			.depthBias(-16384)
			.depthBiasClamp(1000000.0f)
			//.depthClipEnable(false)
			.slopeScaledDepthBias(-1.75f)
			.cullMode(CullMode::None));
		m_defaultBias.vs.heightMapSampler = m_heightSampler;
		m_defaultBias.vs.heightmap = m_heightMapSRV;

		m_jitterBias.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_jitterBias.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_jitterBias.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::Greater));
		m_jitterBias.setRasterizerState(RasterizerDescription()
			.depthBias(-16384)
			.depthBiasClamp(1000000.0f)
			//.depthClipEnable(false)
			.slopeScaledDepthBias(-1.75f)
			.cullMode(CullMode::None));
		m_jitterBias.vs.heightMapSampler = m_heightSampler;
		m_jitterBias.vs.heightmap = m_heightMapSRV;


		// setup forward
		setupTerrainPipeline(m_device, m_defaultGreaterEqual, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false);
		setupTerrainPipeline(m_device, m_defaultGreaterEqualDebug, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false, TerrainPipelineMode::Debug);
		setupTerrainPipeline(m_device, m_jitterGreaterEqual, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false);
		setupTerrainPipeline(m_device, m_jitterGreaterEqualDebug, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false, TerrainPipelineMode::Debug);
		setupTerrainPipeline(m_device, m_defaultEqual, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false);
		setupTerrainPipeline(m_device, m_defaultEqualDebug, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false, TerrainPipelineMode::Debug);
		setupTerrainPipeline(m_device, m_jitterEqual, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false);
		setupTerrainPipeline(m_device, m_jitterEqualDebug, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false, TerrainPipelineMode::Debug);
		
		setupTerrainPipeline(m_device, m_jitterEqualFlipped, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, false);
		m_jitterEqualFlipped.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back).fillMode(FillMode::Solid).frontCounterClockwise(false));

		setupTerrainPipeline(m_device, m_defaultGreaterEqualWireframe, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true);
		setupTerrainPipeline(m_device, m_defaultGreaterEqualDebugWireframe, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true, TerrainPipelineMode::Debug);
		setupTerrainPipeline(m_device, m_jitterGreaterEqualWireframe, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true);
		setupTerrainPipeline(m_device, m_jitterGreaterEqualDebugWireframe, DepthTestOption::GreaterEqual, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true, TerrainPipelineMode::Debug);
		setupTerrainPipeline(m_device, m_defaultEqualWireframe, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true);
		setupTerrainPipeline(m_device, m_defaultEqualDebugWireframe, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true, TerrainPipelineMode::Debug);
		setupTerrainPipeline(m_device, m_jitterEqualWireframe, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true);
		setupTerrainPipeline(m_device, m_jitterEqualDebugWireframe, DepthTestOption::Equal, front, back, true, m_heightMapSRV, m_colorMapSRV, m_heightSampler, true, TerrainPipelineMode::Debug);

		// setup cluster data
		auto& modelResources = device.modelResources();
		{
			m_clusters.modelResource = modelResources.gpuBuffers().clusterDataAllocator().allocate(m_cellCount);
			engine::vector<ClusterData> cData(m_cellCount);
			engine::vector<BoundingBox> bbData(m_cellCount);
			engine::vector<Vector4f> bsData(m_cellCount);
			uint indexStart = static_cast<uint>(m_cell.index().gpuIndex);
			for (int a = 0; a < m_cellCount; ++a)
			{
				cData[a].indexCount = static_cast<uint>(m_cell.index().elements);
				cData[a].indexPointer = indexStart;
				cData[a].vertexPointer = static_cast<uint>(m_cell.vertex().gpuIndex); // TODO: check this
				bbData[a] = {};//outputData[i].clusterBounds[a];
				bsData[a] = {};//outputData[i].clusterCones[a]; // TODO: there actually isn't any bounding sphere data currently

				//indexStart += m_cell.index().elements;
			}

			auto uploadFuture0 = device.residencyV2().uploadTemp(modelResources.gpuBuffers().clusterBinding().buffer(), m_clusters.modelResource.gpuIndex * sizeof(ClusterData), &cData[0], sizeof(ClusterData) * cData.size());
			auto uploadFuture1 = device.residencyV2().uploadTemp(modelResources.gpuBuffers().clusterBoundingBox().buffer(), m_clusters.modelResource.gpuIndex * sizeof(BoundingBox), &bbData[0], sizeof(BoundingBox) * bbData.size());
			auto uploadFuture2 = device.residencyV2().uploadTemp(modelResources.gpuBuffers().clusterCone().buffer(), m_clusters.modelResource.gpuIndex * sizeof(Vector4f), &bsData[0], sizeof(Vector4f) * bsData.size());
			uploadFuture0.blockUntilUploaded();
			uploadFuture1.blockUntilUploaded();
			uploadFuture2.blockUntilUploaded();
		}
#if 0
		m_clusters.modelResource = device.modelResources().gpuBuffers().clusterDataAllocator().allocate(m_cellCount);

		m_clusters.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(ClusterData) * m_cellCount)));
		m_clusters.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(BoundingBox) * m_cellCount)));
		m_clusters.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(Vector4f) * m_cellCount)));

		m_clusters.uploads[0].gpuIndex = m_clusters.modelResource.gpuIndex;
		m_clusters.uploads[1].gpuIndex = m_clusters.modelResource.gpuIndex;
		m_clusters.uploads[2].gpuIndex = m_clusters.modelResource.gpuIndex;

		m_clusters.uploads[0].buffer = modelResources.gpuBuffers().clusterBinding();
		m_clusters.uploads[1].buffer = modelResources.gpuBuffers().clusterBoundingBox();
		m_clusters.uploads[2].buffer = modelResources.gpuBuffers().clusterCone();

		ClusterData* cData = reinterpret_cast<ClusterData*>(m_clusters.uploads[0].ptr);
		BoundingBox* bbData = reinterpret_cast<BoundingBox*>(m_clusters.uploads[1].ptr);
		Vector4f* bsData = reinterpret_cast<Vector4f*>(m_clusters.uploads[2].ptr);
		uint32_t indexStart = m_cell.index().gpuIndex;// gpuData[i].triangleData.modelResource.gpuIndex;
		for (int a = 0; a < m_cellCount; ++a)
		{
			cData->indexCount = m_cell.index().elements;// outputData[i].clusterIndexCount[a];
			cData->indexPointer = indexStart;
			cData->vertexPointer = m_cell.vertex().gpuIndex;// gpuData[i].vertexData.modelResource.gpuIndex;
			//*bbData = outputData[i].clusterBounds[a];
			//*bsData = outputData[i].clusterCones[a]; // TODO: there actually isn't any bounding sphere data currently
			//indexStart += outputData[i].clusterIndexCount[a];
			cData++;
			bbData++;
			bsData++;
		}

		modelResources.residency().makeResident(m_clusters.uploads[0]);
		modelResources.residency().makeResident(m_clusters.uploads[1]);
		modelResources.residency().makeResident(m_clusters.uploads[2]);
		modelResources.residency().freeUpdateAllocation(m_clusters.uploads[0]);
		modelResources.residency().freeUpdateAllocation(m_clusters.uploads[1]);
		modelResources.residency().freeUpdateAllocation(m_clusters.uploads[2]);
		m_clusters.uploads.clear();
#endif

		// submesh data
		{
			m_submesh.modelResource = modelResources.gpuBuffers().subMeshDataAllocator().allocate(1);
			SubMeshAdjacency   sAdjacency;
			SubMeshData        sMeshData;
			BoundingBox        sbbData;
			BoundingSphere     sbsData;

			sAdjacency.adjacencyPointer = 0;
			sAdjacency.adjacencyCount = 0;
			sAdjacency.baseVertexPointer = static_cast<uint>(m_cell.vertex().gpuIndex);
			sMeshData.clusterCount = static_cast<uint>(m_cellCount);
			sMeshData.clusterPointer = static_cast<uint>(m_clusters.modelResource.gpuIndex);

			sbbData = {};
			sbsData = {};

			auto uploadFuture0 = device.residencyV2().uploadTemp(modelResources.gpuBuffers().subMeshAdjacency().buffer(), m_submesh.modelResource.gpuIndex * sizeof(SubMeshAdjacency), &sAdjacency, sizeof(SubMeshAdjacency));
			auto uploadFuture1 = device.residencyV2().uploadTemp(modelResources.gpuBuffers().subMeshData().buffer(), m_submesh.modelResource.gpuIndex * sizeof(SubMeshData), &sMeshData, sizeof(SubMeshData));
			auto uploadFuture2 = device.residencyV2().uploadTemp(modelResources.gpuBuffers().subMeshBoundingBox().buffer(), m_submesh.modelResource.gpuIndex * sizeof(BoundingBox), &sbbData, sizeof(BoundingBox));
			auto uploadFuture3 = device.residencyV2().uploadTemp(modelResources.gpuBuffers().subMeshBoundingSphere().buffer(), m_submesh.modelResource.gpuIndex * sizeof(BoundingSphere), &sbsData, sizeof(BoundingSphere));
			uploadFuture0.blockUntilUploaded();
			uploadFuture1.blockUntilUploaded();
			uploadFuture2.blockUntilUploaded();
			uploadFuture3.blockUntilUploaded();
		}
#if 0
		m_submesh.modelResource = modelResources.gpuBuffers().subMeshDataAllocator().allocate(1);
		m_submesh.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(SubMeshAdjacency)));
		m_submesh.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(SubMeshData)));
		m_submesh.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(BoundingBox)));
		m_submesh.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(BoundingSphere)));

		m_submesh.uploads[0].gpuIndex = m_submesh.modelResource.gpuIndex;
		m_submesh.uploads[1].gpuIndex = m_submesh.modelResource.gpuIndex;
		m_submesh.uploads[2].gpuIndex = m_submesh.modelResource.gpuIndex;
		m_submesh.uploads[3].gpuIndex = m_submesh.modelResource.gpuIndex;

		m_submesh.uploads[0].buffer = modelResources.gpuBuffers().subMeshAdjacency();
		m_submesh.uploads[1].buffer = modelResources.gpuBuffers().subMeshData();
		m_submesh.uploads[2].buffer = modelResources.gpuBuffers().subMeshBoundingBox();
		m_submesh.uploads[3].buffer = modelResources.gpuBuffers().subMeshBoundingSphere();

		SubMeshAdjacency*   sAdjacency = reinterpret_cast<SubMeshAdjacency*>(m_submesh.uploads[0].ptr);
		SubMeshData*        sMeshData = reinterpret_cast<SubMeshData*>(m_submesh.uploads[1].ptr);
		BoundingBox*        sbbData = reinterpret_cast<BoundingBox*>(m_submesh.uploads[2].ptr);
		BoundingSphere*     sbsData = reinterpret_cast<BoundingSphere*>(m_submesh.uploads[3].ptr);

		sAdjacency->adjacencyPointer = 0;// gpuData[i].adjacencyData.modelResource.gpuIndex;
		sAdjacency->adjacencyCount = 0;// gpuData[i].adjacencyData.modelResource.elements;
		sAdjacency->baseVertexPointer = m_cell.vertex().gpuIndex;// gpuData[i].vertexData.modelResource.gpuIndex;
		sMeshData->clusterCount = m_cellCount;
		sMeshData->clusterPointer = m_clusters.modelResource.gpuIndex;
		//*sbbData = boundingBox;
		//*sbsData = boundingSphere;

		modelResources.residency().makeResident(m_submesh.uploads[0]);
		modelResources.residency().makeResident(m_submesh.uploads[1]);
		modelResources.residency().makeResident(m_submesh.uploads[2]);
		modelResources.residency().makeResident(m_submesh.uploads[3]);
		modelResources.residency().freeUpdateAllocation(m_submesh.uploads[0]);
		modelResources.residency().freeUpdateAllocation(m_submesh.uploads[1]);
		modelResources.residency().freeUpdateAllocation(m_submesh.uploads[2]);
		modelResources.residency().freeUpdateAllocation(m_submesh.uploads[3]);
		m_submesh.uploads.clear();
#endif

		// lod binding data
		{
			engine::vector<SubMeshUVLod> lods(1);
			m_lods.modelResource = modelResources.gpuBuffers().lodAllocator().allocate(lods.size());
			for (auto&& lod : lods)
			{
				lod.submeshPointer = static_cast<uint>(m_submesh.modelResource.gpuIndex);
				lod.uvPointer = 0;
			}
			auto uploadLod = device.residencyV2().uploadTemp(modelResources.gpuBuffers().lod().buffer(), m_lods.modelResource.gpuIndex * sizeof(SubMeshUVLod), &lods[0], sizeof(SubMeshUVLod) * lods.size());
			uploadLod.blockUntilUploaded();
		}
#if 0
		// lod binding data
		uint32_t lodCount = 1;
		m_lods.modelResource = modelResources.gpuBuffers().lodAllocator().allocate(lodCount);
		m_lods.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(SubMeshUVLod) * lodCount)));
		m_lods.uploads[0].gpuIndex = m_lods.modelResource.gpuIndex;
		m_lods.uploads[0].buffer = modelResources.gpuBuffers().lod();
		SubMeshUVLod* lodBindingPtr = reinterpret_cast<SubMeshUVLod*>(m_lods.uploads[0].ptr);
		for (int i = 0; i < lodCount; ++i)
		{
			lodBindingPtr->submeshPointer = m_submesh.modelResource.gpuIndex;
			lodBindingPtr->uvPointer = 0;
			++lodBindingPtr;
		}
		modelResources.residency().makeResident(m_lods.uploads[0]);
		modelResources.residency().freeUpdateAllocation(m_lods.uploads[0]);
#endif

		// instance data
		m_instance = SubMesh::createInstance(
			m_device,
			engine::vector<ModelResource>{}, 
			m_terrainInstanceCount, 
			m_terrainClusterCount,
			m_lods, 1ull, m_cell.vertexScale());

		m_subMeshAllocation = device.modelResources().addSubmesh(m_instance);
		
		m_terrainMaterial.materialSet = 0;
		m_terrainMaterial.materialSet |= 0x80;
		device.modelResources().updateSubmeshMaterial(*m_subMeshAllocation, m_terrainMaterial);
	}

	void TerrainRenderer::updateTerrainPatches(
		CommandList& cmd,
		const Camera& camera,
		bool terrainAllDebugBoundingBoxes)
	{
		CPU_MARKER(cmd.api(), "Update terrain clusters");
		GPU_MARKER(cmd, "Update terrain clusters");

		if (m_matrixUpdate)
		{
			m_matrixUpdate = false;
			// *m_meshBuffers.modelAllocations
			m_device.modelResources().updateSubmeshTransform(*m_subMeshAllocation, m_lastMatrix);
		}

		auto vertexes = m_cell.vertex().elements;
		auto setupRefreshPipeline = [&](engine::Pipeline<shaders::RefreshTerrainClusters>& pipeline)
		{
			pipeline.cs.cameraWorldPosition = camera.position();
			pipeline.cs.wholeVertexCount.x = static_cast<uint32_t>(vertexes * m_cellCount);
			pipeline.cs.heightMapSize = {
				static_cast<uint32_t>(m_heightMapImage->width()),
				static_cast<uint32_t>(m_heightMapImage->height()) };
			pipeline.cs.heightMapTexelSize = {
				1.0f / static_cast<float>(m_heightMapImage->width()),
				1.0f / static_cast<float>(m_heightMapImage->height()) };

			pipeline.cs.WorldSize = m_settings.worldSize;
			pipeline.cs.vertexPerCluster.x = static_cast<uint32_t>(m_cell.index().elements);

			pipeline.cs.SectorCount = m_settings.sectorCount;
			pipeline.cs.subMeshBoundingBoxIndex.x = static_cast<uint32_t>(m_submesh.modelResource.gpuIndex);

			pipeline.cs.CellCount = m_settings.cellCount;
			pipeline.cs.clusterStartPtr.x = static_cast<uint32_t>(m_clusters.modelResource.gpuIndex);

			pipeline.cs.NodeCount = m_settings.nodeCount;
			pipeline.cs.terrainInstancePtr.x = static_cast<uint32_t>(m_subMeshAllocation->subMeshInstance->instanceData.modelResource.gpuIndex);

			pipeline.cs.VertexCount = Vector3f{
				pipeline.cs.NodeCount.x,
				pipeline.cs.NodeCount.y,
				pipeline.cs.NodeCount.z } +Vector3f{ 1.0f, 0.0f, 1.0f };
			pipeline.cs.SectorSize = m_settings.sectorSize;
			pipeline.cs.CellSize = m_settings.sectorSize / m_settings.cellCount;

			pipeline.cs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
			pipeline.cs.heightmap = m_heightMapSRV;
			pipeline.cs.heightMapSampler = m_heightSampler;
			pipeline.cs.subMeshBoundingBoxes = m_device.modelResources().gpuBuffers().subMeshBoundingBoxUAV();
			pipeline.cs.clusterBoundingBoxes = m_device.modelResources().gpuBuffers().clusterBoundingBoxUAV();
		};

		// TODO:
		if (terrainAllDebugBoundingBoxes)
		{
			setupRefreshPipeline(m_refreshTerrainPipelineDebug);
			m_refreshTerrainPipelineDebug.cs.debug = true;
			cmd.bindPipe(m_refreshTerrainPipelineDebug);
			cmd.dispatch(std::max(roundUpToMultiple(m_refreshTerrainPipelineDebug.cs.wholeVertexCount.x, 121u) / 121u, 1u), 1, 1);
		}
		else
		{
			setupRefreshPipeline(m_refreshTerrainPipeline);
			m_refreshTerrainPipeline.cs.debug = false;
			cmd.bindPipe(m_refreshTerrainPipeline);
			cmd.dispatch(std::max(roundUpToMultiple(m_refreshTerrainPipeline.cs.wholeVertexCount.x, 121u) / 121u, 1u), 1, 1);
		}
		
	}

	void TerrainRenderer::updateTransform(const Matrix4f& mat)
	{
		m_lastMatrix = mat;
		m_matrixUpdate = true;
	}

	uint32_t TerrainRenderer::clusterCount() const
	{
		ASSERT(m_cellCount >= 0, "Can't have negative cell count!");
		return static_cast<uint32_t>(m_cellCount);
	}

	void TerrainRenderer::render(
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
		const Vector3f& location)
	{
		return;
		CPU_MARKER(cmd.api(), "Render terrain");
		GPU_MARKER(cmd, "Render terrain");

		cmd.setRenderTargets({ target }, dsv);
		
		auto setupPipeline = [&](engine::Pipeline<shaders::Terrain>& pipeline)
		{
			pipeline.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
			pipeline.vs.cameraWorldPosition = camera.position();
			pipeline.vs.heightMapSize = { 
				static_cast<uint32_t>(m_heightMapImage->width()), 
				static_cast<uint32_t>(m_heightMapImage->height()) };
			pipeline.vs.heightMapTexelSize = { 
				1.0f / static_cast<float>(m_heightMapImage->width()), 
				1.0f / static_cast<float>(m_heightMapImage->height()) };

			pipeline.vs.WorldPosition = location;
			pipeline.vs.WorldSize = m_settings.worldSize;
			pipeline.vs.SectorCount = m_settings.sectorCount;
			pipeline.vs.CellCount = m_settings.cellCount;
			pipeline.vs.NodeCount = m_settings.nodeCount;
			pipeline.vs.VertexCount = Vector3f{ pipeline.vs.NodeCount.x, pipeline.vs.NodeCount.y, pipeline.vs.NodeCount.z } + Vector3f{ 1.0f, 0.0f, 1.0f };
			pipeline.vs.SectorSize = m_settings.sectorSize;
			pipeline.vs.CellSize = m_settings.sectorSize / m_settings.cellCount;

			/*
			const float3 WorldPosition = float3(
		-(WorldSize.x / 2.0f),
		30.0f,
		-(WorldSize.z / 2.0f)) - float3(0.0f, 350.0f, 0.0f) + float3(0.01f, 0.0f, 0.05f);
		*/

			// Constants
			pipeline.ps.heightMapSize = {
				static_cast<uint32_t>(m_heightMapImage->width()),
				static_cast<uint32_t>(m_heightMapImage->height()) };
			pipeline.ps.heightMapTexelSize = {
				1.0f / static_cast<float>(m_heightMapImage->width()),
				1.0f / static_cast<float>(m_heightMapImage->height()) };

			pipeline.ps.WorldPosition = location;
			pipeline.ps.WorldSize = m_settings.worldSize;
			pipeline.ps.SectorCount = m_settings.sectorCount;
			pipeline.ps.CellCount = m_settings.cellCount;
			pipeline.ps.NodeCount = m_settings.nodeCount;
			pipeline.ps.VertexCount = Vector3f{ pipeline.vs.NodeCount.x, pipeline.vs.NodeCount.y, pipeline.vs.NodeCount.z } +Vector3f{ 1.0f, 0.0f, 1.0f };
			pipeline.ps.SectorSize = m_settings.sectorSize;
			pipeline.ps.CellSize = m_settings.sectorSize / m_settings.cellCount;

			pipeline.ps.shadowSize = Float2{ 1.0f / static_cast<float>(ShadowMapWidth), 1.0f / static_cast<float>(ShadowMapHeight) };
			pipeline.ps.binSize.x = binSize(camera.width());
			pipeline.ps.binSize.y = binSize(camera.height());
			pipeline.ps.cameraWorldSpacePosition = camera.position();
			pipeline.ps.exposure = camera.exposure();
			pipeline.ps.environmentStrength = camera.environmentMapStrength();
			if (camera.environmentIrradiance().valid() && camera.environmentIrradiance().texture().arraySlices() == 1)
			{
				pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(false);
				pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(true);
			}
			else
			{
				pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(true);
				pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(false);
			}

			pipeline.ps.hasEnvironmentSpecular.x = camera.environmentSpecular().valid();
			pipeline.ps.probePositionRange = Float4{ probePosition.x, probePosition.y, probePosition.z, probeRange };
			pipeline.ps.probeBBmin = Float4{ probePosition.x - probeRange, probePosition.y - probeRange, probePosition.z - probeRange, 0.0f };
			pipeline.ps.probeBBmax = Float4{ probePosition.x + probeRange, probePosition.y + probeRange, probePosition.z + probeRange, 0.0f };
			pipeline.ps.cameraInverseProjectionMatrix = fromMatrix(camera.projectionMatrix().inverse());
			pipeline.ps.cameraInverseViewMatrix = fromMatrix(camera.viewMatrix().inverse());
			pipeline.ps.frameSize.x = virtualResolution.x;
			pipeline.ps.frameSize.y = virtualResolution.y;
			pipeline.ps.usingProbe.x = 0;
			pipeline.ps.debugMode.x = debugMode;
			pipeline.ps.resolution = Float3{ static_cast<float>(camera.width()), static_cast<float>(camera.height()), 1.0f };

			// Resources
			if (camera.environmentIrradiance().valid() && camera.environmentIrradiance().texture().arraySlices() == 1)
			{
				pipeline.ps.environmentIrradianceCubemap = TextureSRV();
				pipeline.ps.environmentIrradiance = camera.environmentIrradiance();
			}
			else
			{
				pipeline.ps.environmentIrradianceCubemap = camera.environmentIrradiance();
				pipeline.ps.environmentIrradiance = TextureSRV();
			}
			pipeline.ps.environmentSpecular = camera.environmentSpecular();
			pipeline.ps.environmentBrdfLut = camera.environmentBrdfLUT();

			pipeline.ps.ssao = ssao;
			pipeline.ps.shadowMap = shadowMap;
			pipeline.ps.materialTextures = m_device.modelResources().textures();
			pipeline.ps.shadowVP = shadowVP;
			if (lights.count() > 0)
			{
				pipeline.ps.lightWorldPosition = lights.worldPositions();
				pipeline.ps.lightDirection = lights.directions();
				pipeline.ps.lightColor = lights.colors();
				pipeline.ps.lightIntensity = lights.intensities();
				pipeline.ps.lightRange = lights.ranges();
				pipeline.ps.lightType = lights.types();
				pipeline.ps.lightParameters = lights.parameters();
				pipeline.ps.lightBins = lightBins;
				pipeline.ps.lightIndexToShadowIndex = lightIndexToShadowIndex;
			}
			pipeline.ps.instanceMaterials = m_device.modelResources().gpuBuffers().instanceMaterial();
		};

		if (drawTerrain)
		{
			setupPipeline(m_terrainPipeline);
			m_terrainPipeline.ps.wireframeMode = { 0 };
			m_terrainPipeline.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back).fillMode(FillMode::Solid));
			cmd.bindPipe(m_terrainPipeline);
			cmd.drawIndexedInstanced(
				m_cell.indexes(),
				m_cell.indexes().desc().elements, 10000, 0, 0, 0);
		}

		if (drawWireframe)
		{
			setupPipeline(m_terrainWireframePipeline);
			m_terrainWireframePipeline.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back).fillMode(FillMode::Wireframe));
			m_terrainWireframePipeline.ps.wireframeMode = { 1 };
			m_terrainWireframePipeline.ps.wireframeColor = { 1.0f, 0.0f, 0.0f, };
			cmd.bindPipe(m_terrainWireframePipeline);
			cmd.drawIndexedInstanced(
				m_cell.indexes(),
				m_cell.indexes().desc().elements, 10000, 0, 0, 0);
		}
	}

	struct TerrainFwdRenderParameters
	{
		Device& device;
		Camera& camera;
		ClusterDataLine& clusters;
		TextureSRV shadowMap;
		BufferSRV shadowVP;
		BufferSRV lightIndexToShadowIndex;
		TextureSRV ssao;
		int width; int height;
		Vector3f probePosition;
		float probeRange;
		LightData& lights;
		BufferSRV lightBins;
		const Matrix4f& previousCameraViewMatrix;
		const Matrix4f& previousCameraProjectionMatrix;
		const TerrainSettings& settings;
		size_t heightMapWidth; size_t heightMapHeight;
		uint32_t clusterGpuIndex;
		uint32_t debugMode;
		const Vector2<int>& virtualResolution;
		float heightBias;
		uint wireframeMode;
		Vector3f wireframeColor;
	};


	template<typename T>
	void setupTerrainPipelineForRendering(
		TerrainFwdRenderParameters& params,
		T& pipeline)
	{
		pipeline.vs.vertices = params.device.modelResources().gpuBuffers().vertex();
		pipeline.vs.normals = params.device.modelResources().gpuBuffers().normal();
		pipeline.vs.tangents = params.device.modelResources().gpuBuffers().tangent();
		pipeline.vs.uv = params.device.modelResources().gpuBuffers().uv();
		pipeline.vs.scales = params.device.modelResources().gpuBuffers().instanceScale();
		// pipeline.vs.clusters -> filled in later
		pipeline.vs.clusterData = params.device.modelResources().gpuBuffers().clusterBinding();
		pipeline.vs.transformHistory = params.device.modelResources().gpuBuffers().instanceTransform();
		
		pipeline.vs.baseIndexes = params.device.modelResources().gpuBuffers().index();
		pipeline.ps.transformHistory = params.device.modelResources().gpuBuffers().instanceTransform();
		pipeline.ps.instanceMaterials = params.device.modelResources().gpuBuffers().instanceMaterial();

		pipeline.vs.clusters = params.clusters.clustersSRV;

		pipeline.vs.viewProjectionMatrix = fromMatrix(params.camera.projectionMatrix() * params.camera.viewMatrix());
		pipeline.vs.previousViewProjectionMatrix = fromMatrix(params.previousCameraProjectionMatrix * params.previousCameraViewMatrix);
		pipeline.vs.jitterViewProjectionMatrix = fromMatrix(params.camera.jitterMatrix(params.device.frameNumber(), params.virtualResolution) * params.camera.projectionMatrix() * params.camera.viewMatrix());

		pipeline.vs.cameraWorldPosition = params.camera.position();
		pipeline.vs.heightBias = params.heightBias;
		pipeline.vs.heightMapSize = { static_cast<uint32_t>(params.heightMapWidth), static_cast<uint32_t>(params.heightMapHeight) };
		pipeline.vs.heightMapTexelSize = { 1.0f / static_cast<float>(params.heightMapWidth), 1.0f / static_cast<float>(params.heightMapHeight) };
		pipeline.vs.WorldSize = params.settings.worldSize;
		pipeline.vs.clusterStartPtr.x = params.clusterGpuIndex;
		pipeline.vs.SectorCount = params.settings.sectorCount;
		pipeline.vs.CellCount = params.settings.cellCount;
		pipeline.vs.NodeCount = params.settings.nodeCount;
		pipeline.vs.VertexCount = Vector3f{ params.settings.nodeCount.x, params.settings.nodeCount.y, params.settings.nodeCount.z } +Vector3f{ 1.0f, 0.0f, 1.0f };
		pipeline.vs.SectorSize = params.settings.sectorSize;
		pipeline.vs.CellSize = params.settings.sectorSize / params.settings.cellCount;

		pipeline.ps.heightMapSize = { static_cast<uint32_t>(params.heightMapWidth), static_cast<uint32_t>(params.heightMapHeight) };
		pipeline.ps.heightMapTexelSize = { 1.0f / static_cast<float>(params.heightMapWidth), 1.0f / static_cast<float>(params.heightMapHeight) };
		pipeline.ps.WorldSize = params.settings.worldSize;
		pipeline.ps.SectorCount = params.settings.sectorCount;
		pipeline.ps.CellCount = params.settings.cellCount;
		pipeline.ps.NodeCount = params.settings.nodeCount;
		pipeline.ps.VertexCount = Vector3f{ params.settings.nodeCount.x, params.settings.nodeCount.y, params.settings.nodeCount.z } +Vector3f{ 1.0f, 0.0f, 1.0f };
		pipeline.ps.SectorSize = params.settings.sectorSize;
		pipeline.ps.CellSize = params.settings.sectorSize / params.settings.cellCount;

		pipeline.ps.cameraWorldSpacePosition = params.camera.position();

		pipeline.ps.environmentStrength = params.camera.environmentMapStrength();
		if (params.camera.environmentIrradiance().valid() && params.camera.environmentIrradiance().texture().arraySlices() == 1)
		{
			pipeline.ps.environmentIrradiance = params.camera.environmentIrradiance();
			pipeline.ps.environmentIrradianceCubemap = TextureSRV();
			pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(false);
			pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(true);
		}
		else
		{
			pipeline.ps.environmentIrradiance = TextureSRV();
			pipeline.ps.environmentIrradianceCubemap = params.camera.environmentIrradiance();
			pipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(true);
			pipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(false);
		}
		pipeline.ps.environmentSpecular = params.camera.environmentSpecular();
		pipeline.ps.environmentBrdfLut = params.camera.environmentBrdfLUT();
		pipeline.ps.hasEnvironmentSpecular.x = params.camera.environmentSpecular().valid();
		pipeline.ps.shadowSize = Float2{ 1.0f / static_cast<float>(ShadowMapWidth), 1.0f / static_cast<float>(ShadowMapHeight) };
		pipeline.ps.shadowMap = params.shadowMap;
		pipeline.ps.shadowVP = params.shadowVP;

		pipeline.ps.cameraInverseProjectionMatrix = fromMatrix(params.camera.projectionMatrix().inverse());
		pipeline.ps.cameraInverseViewMatrix = fromMatrix(params.camera.viewMatrix().inverse());

		pipeline.ps.ssao = params.ssao;
		pipeline.ps.resolution = Float3{ static_cast<float>(params.camera.width()), static_cast<float>(params.camera.height()), 1.0f };

		pipeline.ps.frameSize.x = params.width;
		pipeline.ps.frameSize.y = params.height;

		pipeline.ps.materialTextures = params.device.modelResources().textures();

		pipeline.ps.probePositionRange = Float4{ params.probePosition.x, params.probePosition.y, params.probePosition.z, params.probeRange };
		pipeline.ps.probeBBmin = Float4{ params.probePosition.x - params.probeRange, params.probePosition.y - params.probeRange, params.probePosition.z - params.probeRange, 0.0f };
		pipeline.ps.probeBBmax = Float4{ params.probePosition.x + params.probeRange, params.probePosition.y + params.probeRange, params.probePosition.z + params.probeRange, 0.0f };

		pipeline.ps.usingProbe.x = 0;
		pipeline.ps.binSize.x = binSize(params.camera.width());
		pipeline.ps.binSize.y = binSize(params.camera.height());

		if (params.lights.count() > 0)
		{
			pipeline.ps.lightWorldPosition = params.lights.worldPositions();
			pipeline.ps.lightDirection = params.lights.directions();
			pipeline.ps.lightColor = params.lights.colors();
			pipeline.ps.lightIntensity = params.lights.intensities();
			pipeline.ps.lightRange = params.lights.ranges();
			pipeline.ps.lightType = params.lights.types();
			pipeline.ps.lightParameters = params.lights.parameters();
			pipeline.ps.lightBins = params.lightBins;
			pipeline.ps.lightIndexToShadowIndex = params.lightIndexToShadowIndex;
		}
		pipeline.ps.exposure = params.camera.exposure();
		pipeline.ps.debugMode.x = params.debugMode;

		pipeline.ps.wireframeMode.x = params.wireframeMode;
		pipeline.ps.wireframeColor = params.wireframeColor;
	};

	static int delay = 500;

	void TerrainRenderer::renderForward(
		CommandList& cmd,
		ClusterDataLine& clusters,
		IndexDataLine& /*indexes*/,
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
		int terrainDebugMode)
	{
		cmd.setRenderTargets({ rtv, motion }, depth);

		bool opaquePass = terrainDebugMode == 0 || terrainDebugMode == 2;
		bool wireframePass = terrainDebugMode == 1 || terrainDebugMode == 2;

		TerrainFwdRenderParameters params{ 
			m_device,
			camera,
			clusters,
			shadowMap,
			shadowVP,
			lightIndexToShadowIndex,
			ssao,
			virtualResolution.x, virtualResolution.y,
			probePosition,
			probeRange,
			lights,
			lightBins,
			previousCameraViewMatrix,
			previousCameraProjectionMatrix,
			m_settings,
			m_heightMapImage->width(), m_heightMapImage->height(),
			static_cast<uint32_t>(m_clusters.modelResource.gpuIndex),
			debugMode,
			virtualResolution,
			0.0f,
			0u,
			{ 1.0f, 0.0f, 0.0f } };

		if (jitter == JitterOption::Disabled &&
			depthTest == DepthTestOption::GreaterEqual)
		{
			CPU_MARKER(cmd.api(), "Render terrain forward (jitter = OFF, depthTest = GreaterEqual)");
			GPU_MARKER(cmd, "Render terrain forward (jitter = OFF, depthTest = GreaterEqual)");

			if (debugMode == 0)
			{
				if (opaquePass)
				{
					setupTerrainPipelineForRendering(params, m_defaultGreaterEqual);
					cmd.bindPipe(m_defaultGreaterEqual);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_defaultGreaterEqualWireframe);
					cmd.bindPipe(m_defaultGreaterEqualWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
			else
			{
				if (opaquePass)
				{
					setupTerrainPipelineForRendering(params, m_defaultGreaterEqualDebug);
					cmd.bindPipe(m_defaultGreaterEqualDebug);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_defaultGreaterEqualDebugWireframe);
					cmd.bindPipe(m_defaultGreaterEqualDebugWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
		}
		else if (jitter == JitterOption::Enabled &&
			depthTest == DepthTestOption::GreaterEqual)
		{
			CPU_MARKER(cmd.api(), "Render terrain forward (jitter = ON, depthTest = GreaterEqual)");
			GPU_MARKER(cmd, "Render terrain forward (jitter = ON, depthTest = GreaterEqual)");

			if (debugMode == 0)
			{
				if (opaquePass)
				{
					setupTerrainPipelineForRendering(params, m_jitterGreaterEqual);
					cmd.bindPipe(m_jitterGreaterEqual);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_jitterGreaterEqualWireframe);
					cmd.bindPipe(m_jitterGreaterEqualWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
			else
			{
				if (opaquePass)
				{
					setupTerrainPipelineForRendering(params, m_jitterGreaterEqualDebug);
					cmd.bindPipe(m_jitterGreaterEqualDebug);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_jitterGreaterEqualDebugWireframe);
					cmd.bindPipe(m_jitterGreaterEqualDebugWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
		}
		else if (jitter == JitterOption::Disabled &&
			depthTest == DepthTestOption::Equal)
		{
			CPU_MARKER(cmd.api(), "Render terrain forward (jitter = OFF, depthTest = Equal)");
			GPU_MARKER(cmd, "Render terrain forward (jitter = OFF, depthTest = Equal)");

			if (debugMode == 0)
			{
				if (opaquePass)
				{
					setupTerrainPipelineForRendering(params, m_defaultEqual);
					cmd.bindPipe(m_defaultEqual);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_defaultGreaterEqualWireframe);
					cmd.bindPipe(m_defaultGreaterEqualWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
			else
			{
				if (opaquePass)
				{
					setupTerrainPipelineForRendering(params, m_defaultEqualDebug);
					cmd.bindPipe(m_defaultEqualDebug);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_defaultGreaterEqualDebugWireframe);
					cmd.bindPipe(m_defaultGreaterEqualDebugWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
		}

		else if (jitter == JitterOption::Enabled &&
			depthTest == DepthTestOption::Equal)
		{
			CPU_MARKER(cmd.api(), "Render terrain forward (jitter = ON, depthTest = Equal)");
			GPU_MARKER(cmd, "Render terrain forward (jitter = ON, depthTest = Equal)");

			if (debugMode == 0)
			{
				if (opaquePass)
				{
					if (m_lastMatrix.m00 < 0.0f || m_lastMatrix.m11 < 0.0f || m_lastMatrix.m22 < 0.0f)
					{
						setupTerrainPipelineForRendering(params, m_jitterEqualFlipped);
						cmd.bindPipe(m_jitterEqualFlipped);
						cmd.drawIndexedInstancedIndirect(
							m_cell.indexes(),
							draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
							draws.clusterRendererExecuteIndirectCount.buffer, 0);
					}
					else
					{
						setupTerrainPipelineForRendering(params, m_jitterEqual);
						cmd.bindPipe(m_jitterEqual);
						cmd.drawIndexedInstancedIndirect(
							m_cell.indexes(),
							draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
							draws.clusterRendererExecuteIndirectCount.buffer, 0);
					}
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_jitterGreaterEqualWireframe);
					cmd.bindPipe(m_jitterGreaterEqualWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
			else
			{
				if (opaquePass)
				{
					setupTerrainPipelineForRendering(params, m_jitterEqualDebug);
					cmd.bindPipe(m_jitterEqualDebug);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
				if (wireframePass)
				{
					params.heightBias = 0.01f;
					params.wireframeMode = 1u;
					setupTerrainPipelineForRendering(params, m_jitterGreaterEqualDebugWireframe);
					cmd.bindPipe(m_jitterGreaterEqualDebugWireframe);
					cmd.drawIndexedInstancedIndirect(
						m_cell.indexes(),
						draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
						draws.clusterRendererExecuteIndirectCount.buffer, 0);
				}
			}
		}
	}

	template<typename T>
	void setupTerrainDepthPipeline(
		Device& device,
		T& pipeline,
		Camera& camera,
		ClusterDataLine& clusters,
		size_t width, size_t height,
		const TerrainSettings& settings,
		uint32_t clusterGpuIndex)
	{
		pipeline.vs.vertices = device.modelResources().gpuBuffers().vertex();
		pipeline.vs.scales = device.modelResources().gpuBuffers().instanceScale();
		pipeline.vs.clusterData = device.modelResources().gpuBuffers().clusterBinding();
		pipeline.vs.transformHistory = device.modelResources().gpuBuffers().instanceTransform();
		pipeline.vs.baseIndexes = device.modelResources().gpuBuffers().index();
		pipeline.vs.clusters = clusters.clustersSRV;
		pipeline.vs.cameraWorldPosition = camera.position();
		pipeline.vs.heightBias = 0.0f;
		pipeline.vs.heightMapSize = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		pipeline.vs.heightMapTexelSize = { 1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height) };
		pipeline.vs.WorldSize = settings.worldSize;
		pipeline.vs.clusterStartPtr.x = clusterGpuIndex;
		pipeline.vs.SectorCount = settings.sectorCount;
		pipeline.vs.CellCount = settings.cellCount;
		pipeline.vs.NodeCount = settings.nodeCount;
		pipeline.vs.VertexCount = Vector3f{ settings.nodeCount.x, settings.nodeCount.y, settings.nodeCount.z } +Vector3f{ 1.0f, 0.0f, 1.0f };
		pipeline.vs.SectorSize = settings.sectorSize;
		pipeline.vs.CellSize = settings.sectorSize / settings.cellCount;
	}

	void TerrainRenderer::renderDepth(
		CommandList& cmd,
		ClusterDataLine& clusters,
		IndexDataLine& /*indexes*/,
		DrawDataLine& draws,
		TextureDSV depth,
		Camera& camera,
		JitterOption jitter,
		BiasOption bias,
		const Vector2<int>& virtualResolution)
	{
		cmd.setRenderTargets({}, depth);

		if (jitter == JitterOption::Disabled &&
			bias == BiasOption::Disabled)
		{
			CPU_MARKER(cmd.api(), "Render terrain depth (jitter = OFF, bias = OFF)");
			GPU_MARKER(cmd, "Render terrain depth (jitter = OFF, bias = OFF)");

			setupTerrainDepthPipeline(m_device, m_default, camera, clusters, m_heightMapImage->width(), m_heightMapImage->height(), m_settings, static_cast<uint32_t>(m_clusters.modelResource.gpuIndex));
			m_default.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());

			cmd.bindPipe(m_default);
			cmd.drawIndexedInstancedIndirect(
				m_cell.indexes(),
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if (jitter == JitterOption::Enabled &&
			bias == BiasOption::Disabled)
		{
			CPU_MARKER(cmd.api(), "Render terrain depth (jitter = ON, bias = OFF)");
			GPU_MARKER(cmd, "Render terrain depth (jitter = ON, bias = OFF)");

			setupTerrainDepthPipeline(m_device, m_jitter, camera, clusters, m_heightMapImage->width(), m_heightMapImage->height(), m_settings, static_cast<uint32_t>(m_clusters.modelResource.gpuIndex));
			m_jitter.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());

			cmd.bindPipe(m_jitter);
			cmd.drawIndexedInstancedIndirect(
				m_cell.indexes(),
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if (jitter == JitterOption::Disabled &&
			bias == BiasOption::Enabled)
		{
			CPU_MARKER(cmd.api(), "Render terrain depth (jitter = OFF, bias = OFF)");
			GPU_MARKER(cmd, "Render terrain depth (jitter = OFF, bias = OFF)");

			setupTerrainDepthPipeline(m_device, m_defaultBias, camera, clusters, m_heightMapImage->width(), m_heightMapImage->height(), m_settings, static_cast<uint32_t>(m_clusters.modelResource.gpuIndex));
			m_defaultBias.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());

			cmd.bindPipe(m_defaultBias);
			cmd.drawIndexedInstancedIndirect(
				m_cell.indexes(),
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
		else if (jitter == JitterOption::Enabled &&
			bias == BiasOption::Enabled)
		{
			CPU_MARKER(cmd.api(), "Render terrain depth (jitter = ON, bias = OFF)");
			GPU_MARKER(cmd, "Render terrain depth (jitter = ON, bias = OFF)");

			setupTerrainDepthPipeline(m_device, m_jitterBias, camera, clusters, m_heightMapImage->width(), m_heightMapImage->height(), m_settings, static_cast<uint32_t>(m_clusters.modelResource.gpuIndex));
			m_jitterBias.vs.jitterViewProjectionMatrix = fromMatrix(camera.jitterMatrix(m_device.frameNumber(), virtualResolution) * camera.projectionMatrix() * camera.viewMatrix());

			cmd.bindPipe(m_jitterBias);
			cmd.drawIndexedInstancedIndirect(
				m_cell.indexes(),
				draws.clusterRendererExecuteIndirectArgumentsBuffer, 0,
				draws.clusterRendererExecuteIndirectCount.buffer, 0);
		}
	}
}
