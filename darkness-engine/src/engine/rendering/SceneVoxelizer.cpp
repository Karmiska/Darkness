#include "engine/rendering/SceneVoxelizer.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Rect.h"
#include "components/Camera.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/rendering/LightData.h"
#include "tools/ToolsCommon.h"

#define VOXEL_GRID_RESOLUTION 512

namespace engine
{
	VoxelGrid::VoxelGrid(
		Device& device,
		int resolution,
		const Vector3f& position,
		const Vector3f& size,
		int voxelMips)
		: m_voxelize{ device.createPipeline<shaders::Voxelize>() }
		, m_clearVoxelGrids{ device.createPipeline<shaders::ClearVoxelGrids>() }
		, m_downsampleVoxels{ device.createPipeline<shaders::DownsampleVoxels>() }
		, m_lightVoxels{ device.createPipeline<shaders::LightVoxels>() }
		, m_createVoxelListDispatchArgs{ device.createPipeline<shaders::VoxelListDispatchArgs>() }
		, m_resolution{ -1 }
		, m_position{ position }
		, m_size{ size }
		, m_mips{ voxelMips }
		, m_voxelListCount{ device }
	{
		ASSERT(isPowerOfTwo(resolution), "Voxel grid supports only power of two resolution");

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

		m_voxelize.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_voxelize.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
		m_voxelize.setDepthStencilState(DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(DepthWriteMask::Zero)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));
		m_voxelize.ps.tex_sampler = device.createSampler(SamplerDescription().filter(Filter::Point));

		m_lightVoxels.cs.shadow_sampler = device.createSampler(SamplerDescription()
			.addressU(TextureAddressMode::Mirror)
			.addressV(TextureAddressMode::Mirror)
			.filter(Filter::Comparison));
		m_lightVoxels.cs.shadowSize = Float2{ 1.0f / static_cast<float>(ShadowMapWidth), 1.0f / static_cast<float>(ShadowMapHeight) };

		m_voxelListUAV = device.createBufferUAV(BufferDescription()
			.elements(resolution * resolution * resolution)
			.elementSize(formatBytes(Format::R32_UINT))
			.format(Format::R32_UINT)
			.name("Voxel list"));
		m_voxelListSRV = device.createBufferSRV(m_voxelListUAV);

		m_voxelListDispatchArgs = device.createBufferUAV(BufferDescription()
			.elements(4)
			.elementSize(sizeof(uint32_t))
			.structured(true)
			.usage(ResourceUsage::GpuReadWrite)
			.indirectArgument(true)
			.name("Voxel list dispatch args")
		);

		refreshBuffers(device, resolution);
	}

	void VoxelGrid::resolution(int gridResolution)
	{
		m_resolution = gridResolution;
	}

	int VoxelGrid::resolution() const
	{
		return m_resolution;
	}

	void VoxelGrid::position(const Vector3f& position)
	{
		m_position = position;
	}

	Vector3f VoxelGrid::position() const
	{
		return m_position;
	}

	void VoxelGrid::size(const Vector3f& size)
	{
		m_size = size;
	}

	Vector3f VoxelGrid::size() const
	{
		return m_size;
	}

	static const char* VoxelGridNames[] =
	{
		"Voxel grid 0",
		"Voxel grid 1",
		"Voxel grid 2",
		"Voxel grid 3",
		"Voxel grid 4",
		"Voxel grid 5",
		"Voxel grid 6",
		"Voxel grid 7",
		"Voxel grid 8",
		"Voxel grid 9",
		"Voxel grid 10",
		"Voxel grid 11",
		"Voxel grid 12",
		"Voxel grid 13",
		"Voxel grid 14"
	};

	void VoxelGrid::refreshBuffers(Device& device, int resolution)
	{
		if (resolution == m_resolution)
			return;

		m_resolution = resolution;

		m_voxelsUAV.clear();
		m_voxelsSRV.clear();
		m_voxels.clear();

		auto mips = (m_mips != -1) ? m_mips : mipCount(resolution, resolution);

		auto size = resolution;
		for (int i = 0; i < mips; ++i)
		{
			m_voxelsUAV.emplace_back(device.createTextureUAV(TextureDescription()
				.width(size)
				.height(size)
				.depth(size)
				.dimension(ResourceDimension::Texture3D)
				.format(Format::R8_UINT)
				.name(VoxelGridNames[i])));
			m_voxelsSRV.emplace_back(device.createTextureSRV(m_voxelsUAV.back()));

			m_voxels.emplace_back(m_voxelsSRV.back());
			size = std::max(size >> 1, 1);
		}
		
		m_normalsUAV = device.createTextureUAV(TextureDescription()
			.width(resolution)
			.height(resolution)
			.depth(resolution)
			.dimension(ResourceDimension::Texture3D)
			.format(Format::R11G11B10_FLOAT)
			.name("Voxel grid normals"));
		m_normalsSRV = device.createTextureSRV(m_normalsUAV);
		m_normals = m_normalsSRV;

		m_colorsUAV = device.createBufferUAV(BufferDescription()
			.elements(resolution * resolution * resolution * 2)
			.elementSize(formatBytes(Format::R32_UINT))
			.format(Format::R32_UINT)
			.name("Voxel grid colors"));
		m_colorsSRV = device.createBufferSRV(m_colorsUAV);
		m_colors = m_colorsSRV;
	}

	void VoxelGrid::clear(CommandList& cmd)
	{
		CPU_MARKER(cmd.api(), "Clear voxel grids");
		GPU_MARKER(cmd, "Clear voxel grids");

		auto resource = m_voxelsUAV[0].resource();
		m_clearVoxelGrids.cs.voxels = m_voxelsUAV[0];
		m_clearVoxelGrids.cs.voxelNormals = m_normalsUAV;
		m_clearVoxelGrids.cs.voxelColorgrid = m_colorsUAV;
		m_clearVoxelGrids.cs.voxelDepth = static_cast<float>(m_resolution);
		m_clearVoxelGrids.cs.voxelList = m_voxelListSRV;
		m_clearVoxelGrids.cs.voxelListCount = m_voxelListCount.srv;

		cmd.bindPipe(m_clearVoxelGrids);
		cmd.dispatchIndirect(m_voxelListDispatchArgs.resource().buffer(), 0u);

		cmd.clearBuffer(m_voxelListCount.uav);
	}

	void VoxelGrid::voxelize(
		Device& device,
		CommandList& cmd,
		BufferSRV clusters,
		BufferIBV indexes,
		Buffer indirectArgs,
		Buffer count)
	{
		CPU_MARKER(cmd.api(), "Voxelize scene");
		GPU_MARKER(cmd, "Voxelize scene");

		cmd.setRenderTargets({});
		cmd.setViewPorts({
			engine::Viewport{
			0, 0,
			static_cast<float>(m_resolution),
			static_cast<float>(m_resolution),
			0.0f, 1.0f } });
		cmd.setScissorRects({ engine::Rectangle{ 0, 0,
			m_resolution,
			m_resolution } });

		Camera cam;
		cam.projection(Projection::Orthographic);
		cam.width(static_cast<int>(m_size.x));
		cam.height(static_cast<int>(m_size.y));
		cam.nearPlane(0.0f);
		cam.farPlane(m_size.z);

#if 1
		{
			CPU_MARKER(cmd.api(), "Z Axis render");
			GPU_MARKER(cmd, "Z Axis render");

			Vector3f cameraPosition = m_position + Vector3f{ m_size.x / 2.0f, m_size.y / 2.0f, 0.0f };
			Vector3f cameraTarget = m_position + Vector3f{ m_size.x / 2.0f, m_size.y / 2.0f, m_size.z };
			Vector3f cameraDirection = (cameraTarget - cameraPosition).normalize();

			cam.position(cameraPosition);
			cam.rotation(Quaternionf::fromMatrix(Camera::lookAt(cameraPosition, cameraTarget, { 0.0f, 1.0f, 0.0f })));
			
			voxelize(
				device, 
				cmd, 
				clusters, 
				indexes, 
				indirectArgs, 
				count, 
				cam,
				cameraDirection,
				0);
		}
#endif

#if 1
		{
			CPU_MARKER(cmd.api(), "X Axis render");
			GPU_MARKER(cmd, "X Axis render");
			
			Vector3f cameraPosition = m_position + Vector3f{ 0.0f, m_size.y / 2.0f, m_size.z / 2.0f };
			Vector3f cameraTarget = m_position + Vector3f{ m_size.x, m_size.y / 2.0f, m_size.z / 2.0f };
			Vector3f cameraDirection = (cameraTarget - cameraPosition).normalize();

			cam.position(cameraPosition);
			cam.rotation(Quaternionf::fromMatrix(Camera::lookAt(cameraPosition, cameraTarget, { 0.0f, 1.0f, 0.0f })));

			voxelize(
				device,
				cmd,
				clusters,
				indexes,
				indirectArgs,
				count,
				cam,
				cameraDirection,
				1);
		}
#endif

#if 1
		{
			CPU_MARKER(cmd.api(), "Y Axis render");
			GPU_MARKER(cmd, "Y Axis render");
			
			Vector3f cameraPosition = m_position + Vector3f{ m_size.x / 2.0f, 0.0f, m_size.z / 2.0f };
			Vector3f cameraTarget = m_position + Vector3f{ m_size.x / 2.0f, m_size.y, m_size.z / 2.0f };
			Vector3f cameraDirection = (cameraTarget - cameraPosition).normalize();

			cam.position(cameraPosition);
			cam.rotation(Quaternionf::fromMatrix(Camera::lookAt(cameraPosition, cameraTarget, { 0.0f, 0.0f, 1.0f })));

			voxelize(
				device,
				cmd,
				clusters,
				indexes,
				indirectArgs,
				count,
				cam,
				cameraDirection,
				2);
		}
#endif

		{
			CPU_MARKER(cmd.api(), "Downsample voxels");
			GPU_MARKER(cmd, "Downsample voxels");

			m_downsampleVoxels.cs.voxels = m_voxelsSRV[0];
			m_downsampleVoxels.cs.voxelMip1 = m_voxelsUAV[1];
			m_downsampleVoxels.cs.voxelMip2 = m_voxelsUAV[2];
			m_downsampleVoxels.cs.voxelMip3 = m_voxelsUAV[3];
			m_downsampleVoxels.cs.voxelMip4 = m_voxelsUAV[4];

			auto resource = m_voxelsSRV[0].resource();
			cmd.bindPipe(m_downsampleVoxels);
			cmd.dispatch(
				roundUpToMultiple(resource.width() / 2u, 8u) / 8u,
				roundUpToMultiple(resource.height() / 2u, 8u) / 8u,
				roundUpToMultiple(resource.depth() / 2u, 8u) / 8u);
		}

		{
			CPU_MARKER(cmd.api(), "Create voxel list dispatch args");
			GPU_MARKER(cmd, "Create voxel list dispatch args");

			m_createVoxelListDispatchArgs.cs.voxelListCount = m_voxelListCount.srv;
			m_createVoxelListDispatchArgs.cs.voxelListDispatchArgs = m_voxelListDispatchArgs;
			cmd.bindPipe(m_createVoxelListDispatchArgs);
			cmd.dispatch(1u, 1u, 1u);
		}
	}

	void VoxelGrid::voxelize(
		Device& device,
		CommandList& cmd,
		BufferSRV clusters,
		BufferIBV indexes,
		Buffer indirectArgs,
		Buffer count,
		const Camera& camera,
		const Vector3f& /*cameraDirection*/,
		int mode)
	{
		
		m_voxelize.vs.uv = device.modelResources().gpuBuffers().uv();
		m_voxelize.vs.baseIndexes = device.modelResources().gpuBuffers().index();
		m_voxelize.vs.vertices = device.modelResources().gpuBuffers().vertex();
		m_voxelize.vs.normals = device.modelResources().gpuBuffers().normal();
		m_voxelize.vs.scales = device.modelResources().gpuBuffers().instanceScale();
		m_voxelize.vs.clusterData = device.modelResources().gpuBuffers().clusterBinding();
		m_voxelize.vs.transformHistory = device.modelResources().gpuBuffers().instanceTransform();
		m_voxelize.vs.clusters = clusters;
		m_voxelize.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());

		m_voxelize.ps.instanceMaterials = device.modelResources().gpuBuffers().instanceMaterial();
		m_voxelize.ps.voxels = m_voxelsUAV[0];
		m_voxelize.ps.voxelNormals = m_normalsUAV;
		m_voxelize.ps.voxelColorgrid = m_colorsUAV;
		m_voxelize.ps.materialTextures = device.modelResources().textures();
		m_voxelize.ps.voxelDepth = static_cast<float>(m_resolution);
		m_voxelize.ps.mode.x = mode;
		m_voxelize.ps.voxelList = m_voxelListUAV;
		m_voxelize.ps.voxelListCount = m_voxelListCount.uav;

		m_voxelize.ps.voxelGridPosition = m_position;
		m_voxelize.ps.voxelGridSize = m_size;

		cmd.bindPipe(m_voxelize);
		cmd.drawIndexedInstancedIndirect(
			indexes,
			indirectArgs, 0,
			count, 0);
	}

	void VoxelGrid::light(
		CommandList& cmd,
		TextureSRV shadowMap,
		BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
		LightData& lights)
	{
		CPU_MARKER(cmd.api(), "Light voxels");
		GPU_MARKER(cmd, "Light voxels");

		m_lightVoxels.cs.shadowMap = shadowMap;
		m_lightVoxels.cs.shadowVP = shadowVP;

		if (lights.count() > 0)
		{
			m_lightVoxels.cs.lightWorldPosition = lights.worldPositions();
			m_lightVoxels.cs.lightDirection = lights.directions();
			m_lightVoxels.cs.lightColor = lights.colors();
			m_lightVoxels.cs.lightParameters = lights.parameters();
			m_lightVoxels.cs.lightType = lights.types();
			m_lightVoxels.cs.lightIntensity = lights.intensities();
			m_lightVoxels.cs.lightRange = lights.ranges();
			m_lightVoxels.cs.lightIndexToShadowIndex = lightIndexToShadowIndex;
		}
		m_lightVoxels.cs.voxels = m_voxelsSRV[0];
		m_lightVoxels.cs.voxelNormals = m_normalsSRV;
		m_lightVoxels.cs.voxelColorgrid = m_colorsUAV;

		m_lightVoxels.cs.voxelDepth = static_cast<float>(m_voxelsSRV[0].resource().width());
		m_lightVoxels.cs.voxelGridPosition = m_position;
		m_lightVoxels.cs.voxelGridSize = m_size;


		m_lightVoxels.cs.lightCount.x = lights.count();

		auto resource = m_voxelsSRV[0].resource();
		cmd.bindPipe(m_lightVoxels);
		cmd.dispatch(
			roundUpToMultiple(resource.width(), 8u) / 8u,
			roundUpToMultiple(resource.height(), 8u) / 8u,
			roundUpToMultiple(resource.depth(), 8u) / 8u);
	}

	VoxelFrustumCull::VoxelFrustumCull(Device& device)
		: m_frustumCuller{ device }
		, m_indexExpansion{ device }
		, m_countBuffer{ device }
		, m_drawOutput{ device }
		, m_indexDataLine{ device, static_cast<size_t>(15000000) }
		, m_indexDataLine2{ device, static_cast<size_t>(15000000) }
		, m_drawDataLine{ device }
	{
	}

	void VoxelFrustumCull::cullScene(
		Device& device,
		VoxelGrid& voxelGrid,
		CommandList& cmd)
	{
		CPU_MARKER(cmd.api(), "Frustum culling");
		GPU_MARKER(cmd, "Frustum culling");

		Vector3f cameraPosition = voxelGrid.position();
		Vector3f cameraTarget = voxelGrid.position() + Vector3f{ 0.0f, 0.0f, 1.0f };
		Vector3f cameraDirection = (cameraTarget - cameraPosition).normalize();

		Camera cam;
		cam.projection(Projection::Orthographic);
		cam.position(cameraPosition);
		auto rotation = Camera::lookAt(cameraPosition, cameraTarget, { 0.0f, 1.0f, 0.0f });
		cam.rotation(Quaternionf::fromMatrix(rotation));
		//Camera::lookAt({ 0.0f, 0.0f, 0.0f }, );
		cam.width(static_cast<int>(voxelGrid.size().x));
		cam.height(static_cast<int>(voxelGrid.size().y));
		cam.nearPlane(0.0f);
		cam.farPlane(voxelGrid.size().z);

		m_countBuffer.reset(cmd);
		m_drawOutput.reset(cmd);
		m_indexDataLine.reset(cmd);
		m_drawDataLine.reset(cmd);

		m_frustumCuller.instanceCullNoDepth(
			cmd,
			cam,
			device.modelResources(),
			{ voxelGrid.resolution(), voxelGrid.resolution() },
			m_countBuffer,
			m_drawOutput);

		m_frustumCuller.expandClusters(cmd, m_drawOutput);

		m_indexExpansion.expandIndexes(
			device,
			cmd,
			m_drawOutput,
			m_indexDataLine,
			m_indexDataLine2,
			m_drawDataLine);
	}

	BufferSRV VoxelFrustumCull::clusters()
	{
		return m_drawOutput.clustersSRV;
	}

	BufferIBV VoxelFrustumCull::indexes()
	{
		return m_indexDataLine2.indexBuffer;
	}

	Buffer VoxelFrustumCull::indirectArgs()
	{
		return m_drawDataLine.clusterRendererExecuteIndirectArgumentsBuffer;
	}

	Buffer VoxelFrustumCull::counts()
	{
		return m_drawDataLine.clusterRendererExecuteIndirectCount.buffer;
	}

	SceneVoxelizer::SceneVoxelizer(Device& device)
		: m_device{ device }
		, m_createVoxelGridDebugDraws{ device.createPipeline<shaders::CreateVoxelGridDebugDraws>() }
		, m_createVoxelGridDebugDrawsFromGrid{ device.createPipeline<shaders::CreateVoxelGridDebugDrawsFromGrid>() }
		, m_createVoxelDebugDrawArgs{ device.createPipeline<shaders::CreateVoxelDebugDrawArgs>() }
		, m_drawVoxelDebug{ device.createPipeline<shaders::DrawVoxelDebug>() }
		, m_drawVoxelGridDebug{ device.createPipeline<shaders::DrawVoxelGridDebugCube>() }
		, m_drawVoxelGridDebugWireframe{ device.createPipeline<shaders::DrawVoxelGridDebugCube>() }
		, m_voxelFrustumCull{ device }
		, m_debugDrawArgs{
			device.createBuffer(BufferDescription()
				.elementSize(sizeof(DrawIndexIndirectArgs))
				.elements(1)
				.usage(ResourceUsage::GpuReadWrite)
				.structured(true)
				.indirectArgument(true)
				.name("Voxel debug draw args")
			) }
		, m_debugDrawArgsUAV{ device.createBufferUAV(m_debugDrawArgs) }
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

		m_drawVoxelDebug.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_drawVoxelDebug.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_drawVoxelDebug.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

		m_drawVoxelGridDebug.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_drawVoxelGridDebug.setRasterizerState(RasterizerDescription().cullMode(CullMode::Back));
		m_drawVoxelGridDebug.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));
		m_drawVoxelGridDebug.setBlendState(BlendDescription().renderTarget(
			0, RenderTargetBlendDescription()
			.blendEnable(true)

			.srcBlend(Blend::SrcAlpha)
			.dstBlend(Blend::InvSrcAlpha)
			.blendOp(BlendOperation::Add)

			.srcBlendAlpha(Blend::InvSrcAlpha)
			.dstBlendAlpha(Blend::Zero)
			.blendOpAlpha(BlendOperation::Add)

			.renderTargetWriteMask(1 | 2 | 4 | 8)
		));

		m_drawVoxelGridDebugWireframe.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_drawVoxelGridDebugWireframe.setRasterizerState(RasterizerDescription()
			.cullMode(CullMode::None)
			.fillMode(FillMode::Wireframe));
		m_drawVoxelGridDebugWireframe.setDepthStencilState(DepthStencilDescription()
			.depthEnable(false)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

		m_grids.emplace_back(VoxelGrid(
			device, 
			VOXEL_GRID_RESOLUTION,
			Vector3f{ -8.0f, -8.0f, -8.0f }, 
			Vector3f{ 16.0f, 16.0f, 16.0f }, 
			5));

		/*m_grids.emplace_back(VoxelGrid(
			device,
			VOXEL_GRID_RESOLUTION,
			Vector3f{ -16.0f, -16.0f, -40.0f },
			Vector3f{ 32.0f, 32.0f, 32.0f },
			5));*/
	}

	void SceneVoxelizer::lightVoxels(
		CommandList& cmd,
		TextureSRV shadowMap,
		BufferSRV shadowVP,
		BufferSRV lightIndexToShadowIndex,
		LightData& lights)
	{
		for (auto&& grid : m_grids)
		{
			grid.light(cmd, shadowMap, shadowVP, lightIndexToShadowIndex, lights);
		}
	}

	void SceneVoxelizer::clearDebug()
	{
		if (m_debugVertexUAV)
		{
			m_debugVertexSRV = BufferSRVOwner();
			m_debugVertexUAV = BufferUAVOwner();
			
			m_debugColorSRV = BufferSRVOwner();
			m_debugColorUAV = BufferUAVOwner();

			m_debugIndexIBV = BufferIBVOwner();
			m_debugIndexUAV = BufferUAVOwner();

			m_debugAllocationsSRV = BufferSRVOwner();
			m_debugAllocationsUAV = BufferUAVOwner();
		}
	}

	void SceneVoxelizer::voxelize(
		CommandList& cmd,
		const Camera& /*camera*/)
	{
		for(auto&& grid : m_grids)
		{
			grid.clear(cmd);

			m_voxelFrustumCull.cullScene(m_device, grid, cmd);

			grid.voxelize(
				m_device, 
				cmd,
				m_voxelFrustumCull.clusters(),
				m_voxelFrustumCull.indexes(),
				m_voxelFrustumCull.indirectArgs(),
				m_voxelFrustumCull.counts());
		}
	}

	void SceneVoxelizer::createDebugVoxelData(
		CommandList& cmd,
		const Camera& /*camera*/,
		int debugVoxelMip)
	{
		if (!m_debugVertexUAV)
		{
			constexpr int DebugVoxelCount = 100000000;
			m_debugVertexUAV = m_device.createBufferUAV(BufferDescription()
				.elements(DebugVoxelCount)
				.elementSize(sizeof(float) * 3)
				.structured(true)
				.usage(ResourceUsage::GpuReadWrite)
				.name("Voxel debug vertex"));
			m_debugVertexSRV = m_device.createBufferSRV(m_debugVertexUAV);

			m_debugColorUAV = m_device.createBufferUAV(BufferDescription()
				.elements(DebugVoxelCount)
				.elementSize(sizeof(float) * 3)
				.structured(true)
				.usage(ResourceUsage::GpuReadWrite)
				.name("Voxel debug color"));
			m_debugColorSRV = m_device.createBufferSRV(m_debugColorUAV);

			m_debugIndexUAV = m_device.createBufferUAV(BufferDescription()
				.elements(DebugVoxelCount)
				.elementSize(formatBytes(Format::R32_UINT))
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("Voxel debug index"));
			m_debugIndexIBV = m_device.createBufferIBV(m_debugIndexUAV);

			m_debugAllocationsUAV = m_device.createBufferUAV(BufferDescription()
				.elements(1)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite)
				.name("Voxel debug allocations"));
			m_debugAllocationsSRV = m_device.createBufferSRV(m_debugAllocationsUAV);
		}

		

		/*{
			CPU_MARKER(cmd.api(), "Create voxel debug draws");
			GPU_MARKER(cmd, "Create voxel debug draws");

			cmd.clearBuffer(m_debugAllocationsUAV);

			m_createVoxelGridDebugDraws.cs.voxels = m_voxelsSRV;
			m_createVoxelGridDebugDraws.cs.normals = m_normalsSRV;
			m_createVoxelGridDebugDraws.cs.colorgrid = m_colorsSRV;
			m_createVoxelGridDebugDraws.cs.vertex = m_debugVertexUAV;
			m_createVoxelGridDebugDraws.cs.color = m_debugColorUAV;
			m_createVoxelGridDebugDraws.cs.index = m_debugIndexUAV;
			m_createVoxelGridDebugDraws.cs.allocations = m_debugAllocationsUAV;
			m_createVoxelGridDebugDraws.cs.depth = static_cast<float>(VOXEL_GRID_RESOLUTION);
			m_createVoxelGridDebugDraws.cs.voxelList = m_voxelListSRV;
			m_createVoxelGridDebugDraws.cs.voxelListCount = m_voxelListCount.srv;

			cmd.bindPipe(m_createVoxelGridDebugDraws);
			cmd.dispatchIndirect(m_voxelListDispatchArgs.resource().buffer(), 0);
		}*/

		{
			CPU_MARKER(cmd.api(), "Create voxel debug draws");
			GPU_MARKER(cmd, "Create voxel debug draws");

			cmd.clearBuffer(m_debugAllocationsUAV);

			for (auto&& grid : m_grids)
			{
				auto resource = grid.voxels()[debugVoxelMip];
				m_createVoxelGridDebugDrawsFromGrid.cs.voxels = resource;
				m_createVoxelGridDebugDrawsFromGrid.cs.voxelNormals = grid.normals();
				m_createVoxelGridDebugDrawsFromGrid.cs.voxelColorgrid = grid.colors();
				m_createVoxelGridDebugDrawsFromGrid.cs.vertex = m_debugVertexUAV;
				m_createVoxelGridDebugDrawsFromGrid.cs.color = m_debugColorUAV;
				m_createVoxelGridDebugDrawsFromGrid.cs.index = m_debugIndexUAV;
				m_createVoxelGridDebugDrawsFromGrid.cs.allocations = m_debugAllocationsUAV;
				m_createVoxelGridDebugDrawsFromGrid.cs.voxelDepth = static_cast<float>(resource.width());
				m_createVoxelGridDebugDrawsFromGrid.cs.voxelGridPosition = grid.position();
				m_createVoxelGridDebugDrawsFromGrid.cs.voxelGridSize = grid.size();
				m_createVoxelGridDebugDrawsFromGrid.cs.voxelMip.x = debugVoxelMip;

				cmd.bindPipe(m_createVoxelGridDebugDrawsFromGrid);
				cmd.dispatch(
					roundUpToMultiple(resource.width(), 8u) / 8u,
					roundUpToMultiple(resource.height(), 8u) / 8u,
					roundUpToMultiple(resource.depth(), 8u) / 8u);
			}
		}

		{
			CPU_MARKER(cmd.api(), "Create voxel debug draw args");
			GPU_MARKER(cmd, "Create voxel debug draw args");

			m_createVoxelDebugDrawArgs.cs.allocations = m_debugAllocationsSRV;
			m_createVoxelDebugDrawArgs.cs.debugDrawArgs = m_debugDrawArgsUAV;

			cmd.bindPipe(m_createVoxelDebugDrawArgs);
			cmd.dispatch(1u, 1u, 1u);
		}
	}

	void SceneVoxelizer::renderDebug(
		CommandList& cmd,
		const Camera& camera,
		TextureRTV target,
		TextureDSV dsv,
		bool debugVoxelGrids)
	{
		{
			if (!m_debugVertexUAV)
				return;

			cmd.clearRenderTargetView(target);
			cmd.clearDepthStencilView(dsv, 0);

			CPU_MARKER(cmd.api(), "Draw debug voxels");
			GPU_MARKER(cmd, "Draw debug voxels");

			cmd.setRenderTargets({ target }, dsv);

			m_drawVoxelDebug.vs.vertex = m_debugVertexSRV;
			m_drawVoxelDebug.vs.color = m_debugColorSRV;
			m_drawVoxelDebug.vs.viewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());

			cmd.bindPipe(m_drawVoxelDebug);
			cmd.drawIndexedIndirect(m_debugIndexIBV, m_debugDrawArgs, 0);
		}

		if(debugVoxelGrids)
		{
			CPU_MARKER(cmd.api(), "Draw debug grid voxels");
			GPU_MARKER(cmd, "Draw debug grid voxels");

			for (auto&& grid : m_grids)
			{
				{
					m_drawVoxelGridDebug.vs.cornera = grid.position();
					m_drawVoxelGridDebug.vs.cornerb = grid.position() + grid.size();
					m_drawVoxelGridDebug.vs.jitterViewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
					m_drawVoxelGridDebug.ps.color = { 1.0f, 0.0f, 0.0f, 0.2f };

					cmd.bindPipe(m_drawVoxelGridDebug);
					cmd.draw(36);
				}
				{
					m_drawVoxelGridDebugWireframe.vs.cornera = grid.position();
					m_drawVoxelGridDebugWireframe.vs.cornerb = grid.position() + grid.size();
					m_drawVoxelGridDebugWireframe.vs.jitterViewProjectionMatrix = fromMatrix(camera.projectionMatrix() * camera.viewMatrix());
					m_drawVoxelGridDebugWireframe.ps.color = { 0.0f, 1.0f, 0.0f, 1.0f };

					cmd.bindPipe(m_drawVoxelGridDebugWireframe);
					cmd.draw(36);
				}
			}
		}
	}
	
}
