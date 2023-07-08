#include "engine/rendering/ShadowRenderer.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/graphics/Barrier.h"
#include "engine/rendering/TerrainRenderer.h"
#include "components/TerrainComponent.h"
#include "engine/Scene.h"
#include "tools/Measure.h"

using namespace tools;

#define ENABLE_SHADOW_CACHE

namespace engine
{
    ShadowRenderer::ShadowRenderer(Device& device)
        : m_device{ device }

		, m_depthRenderer{ device }

        , m_frustumCuller{ device }
        , m_indexExpansion{ device }

        , m_computeChangedScene{ device.createPipeline<shaders::ComputeChangedScene>() }

        , m_modelDataline{ device }
        , m_modelClusterDataLineAlphaClip{ device }
		, m_modelClusterDataLineTerrain{ device }
        , m_modelIndexDraw{ device }
        , m_modelIndexDrawAlphaClip{ device }
		, m_modelIndexDrawTerrain{ device }

        , m_shadowCamera{ engine::make_shared<Transform>() }
        , m_cubemapCamera{ engine::make_shared<Transform>() }
        , m_refresh{ false }
		, m_lastLightCount{ 0u }
        , m_lastShadowCasterCount{ 0u }
    {
        m_cubemapCamera.nearPlane(0.1f);
        m_cubemapCamera.farPlane(1000.0f);
        m_cubemapCamera.fieldOfView(90.0f);
        m_cubemapCamera.projection(Projection::Perspective);

        m_shadowCamera.nearPlane(0.1f);
        m_shadowCamera.farPlane(1000.0f);
        m_shadowCamera.fieldOfView(60.0f);
        m_shadowCamera.width(ShadowMapWidth);
        m_shadowCamera.height(ShadowMapHeight);
        m_shadowCamera.projection(Projection::Perspective);

        m_sceneChanges = device.createBufferUAV(BufferDescription()
            .usage(ResourceUsage::GpuRead)
            .name("SceneChange buffer")
            .structured(true)
            .elements(MaxInstances)
            .elementSize(sizeof(BoundingBox)));
        m_sceneChangesSRV = device.createBufferSRV(m_sceneChanges);

        m_changeCounter = device.createBufferUAV(BufferDescription()
            .usage(ResourceUsage::GpuRead)
            .name("SceneChange counter buffer")
            .format(Format::R32_UINT)
            .elements(2)
            .elementSize(sizeof(uint32_t)));
        m_changeCounterSRV = device.createBufferSRV(m_changeCounter);

        m_frustumMatchCount = device.createBuffer(BufferDescription()
            .format(Format::R32_UINT)
            .elements(2)
            .usage(ResourceUsage::GpuReadWrite)
            .indirectArgument(true)
            .name("Frustum match counter"));

        m_frustumMatchCountUAV = device.createBufferUAV(m_frustumMatchCount);
        m_frustumMatchCountSRV = device.createBufferSRV(m_frustumMatchCount);
    }

    void ShadowRenderer::reallocateClusterDatalines(FlatScene& scene)
    {
        auto newClusterCount = m_device.modelResources().clusterCount();
        auto oldCount = m_modelDataline.clusters.clustersSRV.resource().valid() ? m_modelDataline.clusters.clustersSRV.resource().desc().elements : 0;
        if (newClusterCount != oldCount)
        {
            m_modelDataline.clusters.resize(m_device, newClusterCount);
            m_modelClusterDataLineAlphaClip.resize(m_device, newClusterCount);
        }

        if (scene.terrains.size() > 0)
        {
            auto terrainClusters = scene.terrains[0]->terrain().clusterCount();
            if (m_modelClusterDataLineTerrain.clustersSRV.resource().desc().elements != terrainClusters)
                m_modelClusterDataLineTerrain.resize(m_device, terrainClusters);
        }
        
    }

    bool frustumCull(
        engine::vector<Vector4f>& frustumPlanes,
        const BoundingBox& aabb,
        const Vector3f cameraPosition,
        const Matrix4f& transform,
        float range)
    {
        // Jussi Knuuttilas algorithm
        // works pretty well
        Vector3f corner[8] =
        {
            transform * aabb.min,
            transform * Vector3f{ aabb.min.x, aabb.max.y, aabb.max.z },
            transform * Vector3f{ aabb.min.x, aabb.min.y, aabb.max.z },
            transform * Vector3f{ aabb.min.x, aabb.max.y, aabb.min.z },
            transform * aabb.max,
            transform * Vector3f{ aabb.max.x, aabb.min.y, aabb.min.z },
            transform * Vector3f{ aabb.max.x, aabb.max.y, aabb.min.z },
            transform * Vector3f{ aabb.max.x, aabb.min.y, aabb.max.z }
        };

        for (int i = 0; i < 6; ++i)
        {
            bool hit = false;
            for (auto&& c : corner)
            {
                if (((c - cameraPosition).magnitude() < range) && frustumPlanes[i].xyz().normalize().dot(c - cameraPosition) >= 0)
                {
                    hit = true;
                    break;
                }
            }
            if (!hit)
                return false;
        }
        return true;
    }

    uint32_t ShadowRenderer::shadowCasterCount(const engine::LightData& lightData) const
    {
        uint32_t shadowCasterMapCount = 0;
        for (uint32_t i = 0; i < lightData.count(); ++i)
        {
            if (lightData.shadowCaster()[i])
            {
                LightType type = static_cast<LightType>(lightData.engineTypes()[i]);
                if (type == LightType::Spot)
                    ++shadowCasterMapCount;
                else if (type == LightType::Directional)
                    ++shadowCasterMapCount;
                else if (type == LightType::Point)
                    shadowCasterMapCount += 6;
            }
        }
        return shadowCasterMapCount;
    }

    void ShadowRenderer::updateShadowMaps(uint32_t shadowCasterMapCount, uint32_t lightCount)
    {
        auto oldCasterCount = 0ull;
        if(m_shadowMap)
            oldCasterCount = m_shadowMap.resource().texture().description().arraySlices;
        if ((!m_shadowMap || shadowCasterMapCount != oldCasterCount || m_refresh) && shadowCasterMapCount > 0)
        {
            m_refresh = false;
            m_shadowMap = m_device.createTextureDSV(TextureDescription()
                .name("Shadow map")
                .format(Format::D32_FLOAT)
                .width(ShadowMapWidth)
                .height(ShadowMapHeight)
                .arraySlices(shadowCasterMapCount)
                .usage(ResourceUsage::DepthStencil)
                .optimizedDepthClearValue(0.0f)
                .dimension(ResourceDimension::Texture2DArray)
            );
            m_shadowMapSRV = m_device.createTextureSRV(m_shadowMap);

            m_shadowMapIndices.clear();
            for (uint32_t i = 0; i < shadowCasterMapCount; ++i)
            {
                TextureDescription desc = { m_shadowMap.resource().texture().description() };
                m_shadowMapIndices.emplace_back(
                    m_device.createTextureDSV(
                        m_shadowMap,
                        desc.dimension(ResourceDimension::Texture2DArray),
                        SubResource{ 0, 1, i, 1 }));
            }

            m_shadowVP = m_device.createBufferSRV(BufferDescription()
                .name("Shadow view projection matrixes")
                .elements(shadowCasterMapCount)
                .elementSize(sizeof(Float4x4))
                .structured(true)
                .usage(ResourceUsage::GpuReadWrite)
            );
        }

		if (m_lastLightCount != lightCount && lightCount > 0)
		{
			m_lastLightCount = lightCount;
			m_lightIndexToShadowIndex = m_device.createBufferSRV(BufferDescription()
				.name("Shadow light index to shadow index")
				.elements(lightCount)
				.format(Format::R32_UINT)
				.usage(ResourceUsage::GpuReadWrite));
		}
    }

    static const char* ShadowSpotMessages[32] = {
        "Spot light shadow 0",
        "Spot light shadow 1",
        "Spot light shadow 2",
        "Spot light shadow 3",
        "Spot light shadow 4",
        "Spot light shadow 5",
        "Spot light shadow 6",
        "Spot light shadow 7",
        "Spot light shadow 8",
        "Spot light shadow 9",

        "Spot light shadow 10",
        "Spot light shadow 11",
        "Spot light shadow 12",
        "Spot light shadow 13",
        "Spot light shadow 14",
        "Spot light shadow 15",
        "Spot light shadow 16",
        "Spot light shadow 17",
        "Spot light shadow 18",
        "Spot light shadow 19",

        "Spot light shadow 20",
        "Spot light shadow 21",
        "Spot light shadow 22",
        "Spot light shadow 23",
        "Spot light shadow 24",
        "Spot light shadow 25",
        "Spot light shadow 26",
        "Spot light shadow 27",
        "Spot light shadow 28",
        "Spot light shadow 29",

        "Spot light shadow 30",
        "Spot light shadow 31"
    };

    static const char* ShadowPointMessages[32] = {
        "Point light shadow 0",
        "Point light shadow 1",
        "Point light shadow 2",
        "Point light shadow 3",
        "Point light shadow 4",
        "Point light shadow 5",
        "Point light shadow 6",
        "Point light shadow 7",
        "Point light shadow 8",
        "Point light shadow 9",

        "Point light shadow 10",
        "Point light shadow 11",
        "Point light shadow 12",
        "Point light shadow 13",
        "Point light shadow 14",
        "Point light shadow 15",
        "Point light shadow 16",
        "Point light shadow 17",
        "Point light shadow 18",
        "Point light shadow 19",

        "Point light shadow 20",
        "Point light shadow 21",
        "Point light shadow 22",
        "Point light shadow 23",
        "Point light shadow 24",
        "Point light shadow 25",
        "Point light shadow 26",
        "Point light shadow 27",
        "Point light shadow 28",
        "Point light shadow 29",

        "Point light shadow 30",
        "Point light shadow 31"
    };

    void ShadowRenderer::render(
        CommandList& cmd,
        FlatScene& scene
    )
    {
        auto instanceCount = m_device.modelResources().instanceCount();
        if(instanceCount > 0)
        {
            CPU_MARKER(cmd.api(), "Compute changed scene");
            GPU_MARKER(cmd, "Compute changed scene");

            cmd.clearBuffer(m_changeCounter);

            m_computeChangedScene.cs.subMeshBoundingBoxes = m_device.modelResources().gpuBuffers().subMeshBoundingBox();
            m_computeChangedScene.cs.lodBinding = m_device.modelResources().gpuBuffers().lod();
            m_computeChangedScene.cs.instanceLodBinding = m_device.modelResources().gpuBuffers().instanceLodBinding();
            m_computeChangedScene.cs.transformHistory = m_device.modelResources().gpuBuffers().instanceTransform();
            m_computeChangedScene.cs.sceneChange = m_sceneChanges;
            m_computeChangedScene.cs.changedCounter = m_changeCounter;
            m_computeChangedScene.cs.instanceCount.x = static_cast<uint32_t>(m_device.modelResources().instanceCount());

            cmd.bindPipe(m_computeChangedScene);
            cmd.dispatch(
                roundUpToMultiple(m_device.modelResources().instanceCount(), 64) / 64, 1, 1);

        }

		/*static int ShadowRenderCount = 100;
		if (ShadowRenderCount == 0)
			return;
		--ShadowRenderCount;*/

        if (scene.selectedCamera == -1 || scene.cameras.size() == 0 || !scene.cameras[scene.selectedCamera])
            return;

        auto& lights = *scene.lightData;
        uint32_t shadowCasterMapCount = shadowCasterCount(*scene.lightData);

        bool refreshAllShadowMaps = 
#ifdef ENABLE_SHADOW_CACHE
            lights.count() != m_lastLightCount ||
            shadowCasterMapCount != m_lastShadowCasterCount ||
            m_refresh;
#else
            true;
#endif
            
        m_lastShadowCasterCount = shadowCasterMapCount;

        reallocateClusterDatalines(scene);
        updateShadowMaps(shadowCasterMapCount, lights.count());

        uint32_t spotLightIndex = 0u;
        uint32_t pointLightIndex = 0u;

		engine::vector<uint32_t> shadowId(lights.count());
        engine::vector<Matrix4f> shadowViewProjectionMatrices;
        {
            // fast path for opaque and transparent meshes
            int shadowMapIndex = 0;
            for (uint32_t i = 0; i < lights.count(); ++i)
            {
                if (lights.shadowCaster()[i])
                {
                    if (shadowMapIndex >= m_lightCache.size())
                        m_lightCache.emplace_back(LightCacheInfo{});

					shadowId[i] = shadowMapIndex;
                    LightType type = static_cast<LightType>(lights.engineTypes()[i]);

                    bool lightAttributesHaveChanged = 
                        m_lightCache[shadowMapIndex].position != lights.positions()[i] ||
                        m_lightCache[shadowMapIndex].direction != lights.directionVectors()[i] ||
                        m_lightCache[shadowMapIndex].range != lights.cpuranges()[i] ||
                        m_lightCache[shadowMapIndex].outerAngle != lights.cpuparameters()[i].y ||
                        refreshAllShadowMaps;

                    m_lightCache[shadowMapIndex].position = lights.positions()[i];
                    m_lightCache[shadowMapIndex].direction = lights.directionVectors()[i];
                    m_lightCache[shadowMapIndex].range = lights.cpuranges()[i];
                    m_lightCache[shadowMapIndex].outerAngle = lights.cpuparameters()[i].y;


                    if (type == LightType::Point)
                    {
                        CPU_MARKER(cmd.api(), ShadowPointMessages[pointLightIndex]);
                        GPU_MARKER(cmd, ShadowPointMessages[pointLightIndex]);

                        m_cubemapCamera.width(static_cast<int>(ShadowMapWidth));
                        m_cubemapCamera.height(static_cast<int>(ShadowMapHeight));

                        Vector3f pos = lights.positions()[i];
                        m_cubemapCamera.position(pos);

                        engine::vector<Matrix4f> sides;
                        sides.emplace_back(Matrix4f{ Camera::lookAt(pos + Vector3f(0.0f, 0.0f, 0.0f), pos + Vector3f(-1.0f,  0.0f,  0.0f), Vector3f(0.0f, 1.0f,  0.0f)) });
                        sides.emplace_back(Matrix4f{ Camera::lookAt(pos + Vector3f(0.0f, 0.0f, 0.0f), pos + Vector3f( 1.0f,  0.0f,  0.0f), Vector3f(0.0f, 1.0f,  0.0f)) });
                        sides.emplace_back(Matrix4f{ Camera::lookAt(pos + Vector3f(0.0f, 0.0f, 0.0f), pos + Vector3f( 0.0f,  1.0f,  0.0f), Vector3f(0.0f, 0.0f, -1.0f)) });
                        sides.emplace_back(Matrix4f{ Camera::lookAt(pos + Vector3f(0.0f, 0.0f, 0.0f), pos + Vector3f( 0.0f, -1.0f,  0.0f), Vector3f(0.0f, 0.0f,  1.0f)) });
                        sides.emplace_back(Matrix4f{ Camera::lookAt(pos + Vector3f(0.0f, 0.0f, 0.0f), pos + Vector3f( 0.0f,  0.0f,  1.0f), Vector3f(0.0f, 1.0f,  0.0f)) });
                        sides.emplace_back(Matrix4f{ Camera::lookAt(pos + Vector3f(0.0f, 0.0f, 0.0f), pos + Vector3f( 0.0f,  0.0f, -1.0f), Vector3f(0.0f, 1.0f,  0.0f)) });

                        m_cubemapCamera.farPlane(lights.cpuranges()[i]);

                        for(auto&& side : sides)
                        {
                            m_cubemapCamera.rotation(Quaternionf::fromMatrix(side));
                            auto viewMatrix = m_cubemapCamera.viewMatrix();
                            auto shadowProjection = m_cubemapCamera.projectionMatrix();
                            shadowViewProjectionMatrices.emplace_back(shadowProjection * viewMatrix);
                            
                            m_modelDataline.reset(cmd);
                            m_modelClusterDataLineAlphaClip.reset(cmd);
							m_modelClusterDataLineTerrain.reset(cmd);
                            m_frustumCuller.instanceShadowCull(
                                cmd, 
                                m_cubemapCamera, 
                                m_device.modelResources(), 
                                { ShadowMapWidth, ShadowMapHeight }, 
                                m_modelDataline.clusters,
                                m_modelClusterDataLineAlphaClip,
								m_modelClusterDataLineTerrain,
                                m_sceneChangesSRV,
                                m_changeCounterSRV,
                                m_frustumMatchCountUAV,
                                lightAttributesHaveChanged);

#ifdef ENABLE_SHADOW_CACHE
                            if(!lightAttributesHaveChanged)
                                cmd.setPredicate(m_frustumMatchCountSRV, 0, PredicationOp::EqualZero);
#endif
                            cmd.clearDepthStencilView(m_shadowMapIndices[shadowMapIndex], 0.0f);

                            m_frustumCuller.expandClusters(cmd, m_modelDataline.clusters);
                            m_indexExpansion.expandIndexes(m_device, cmd, m_modelDataline.clusters, m_modelDataline.indexes, m_modelIndexDraw, m_modelDataline.draws);
							m_depthRenderer.render(cmd, m_modelDataline.clusters, m_modelIndexDraw, m_modelDataline.draws, m_shadowMapIndices[shadowMapIndex], m_cubemapCamera, JitterOption::Disabled, AlphaClipOption::Disabled, BiasOption::Enabled, { ShadowMapWidth, ShadowMapHeight });

                            m_modelDataline.draws.reset(cmd);
                            m_frustumCuller.expandClustersShadowAlphaClip(cmd, m_modelClusterDataLineAlphaClip);
                            m_indexExpansion.expandIndexes(m_device, cmd, m_modelClusterDataLineAlphaClip, m_modelDataline.indexes, m_modelIndexDrawAlphaClip, m_modelDataline.draws);
							m_depthRenderer.render(cmd, m_modelClusterDataLineAlphaClip, m_modelIndexDrawAlphaClip, m_modelDataline.draws, m_shadowMapIndices[shadowMapIndex], m_cubemapCamera, JitterOption::Disabled, AlphaClipOption::Enabled, BiasOption::Enabled, { ShadowMapWidth, ShadowMapHeight });

							if (scene.terrains.size() > 0)
							{
								m_modelDataline.draws.reset(cmd);
								m_frustumCuller.expandClustersShadowTerrain(cmd, m_modelClusterDataLineTerrain);
								m_indexExpansion.expandIndexes(m_device, cmd, m_modelClusterDataLineTerrain, m_modelDataline.indexes, m_modelIndexDrawTerrain, m_modelDataline.draws, 600u);
								scene.terrains[0]->terrain().renderDepth(cmd, m_modelClusterDataLineTerrain, m_modelDataline.indexes, m_modelDataline.draws, m_shadowMapIndices[shadowMapIndex], m_cubemapCamera, JitterOption::Disabled, BiasOption::Enabled, { ShadowMapWidth, ShadowMapHeight });
							}

#ifdef ENABLE_SHADOW_CACHE
                            if (!lightAttributesHaveChanged)
                                cmd.setPredicate(BufferSRV{}, 0, PredicationOp::EqualZero);
#endif

                            ++shadowMapIndex;
                        }

						// THIS SEEMS WRONG. SIDES ARE USING THE SAME SHADOW MAP INDEX. IE. OVERWRITING EACH OTHER!

						
                        ++pointLightIndex;
                    }
                    else if (type == LightType::Spot)
                    {
                        CPU_MARKER(cmd.api(), ShadowSpotMessages[spotLightIndex]);
                        GPU_MARKER(cmd, ShadowSpotMessages[spotLightIndex]);

                        m_shadowCamera.farPlane(lights.cpuranges()[i]);
                        m_shadowCamera.position(lights.positions()[i]);
                        m_shadowCamera.rotation(scene.lights[i].rotation);
                        m_shadowCamera.fieldOfView(scene.lights[i].outerCone * 2.0f);
                        shadowViewProjectionMatrices.emplace_back(m_shadowCamera.projectionMatrix() * m_shadowCamera.viewMatrix());

                        m_modelDataline.reset(cmd);
                        m_modelClusterDataLineAlphaClip.reset(cmd);
						m_modelClusterDataLineTerrain.reset(cmd);
                        m_frustumCuller.instanceShadowCull(
                            cmd,
                            m_shadowCamera,
                            m_device.modelResources(),
                            { ShadowMapWidth, ShadowMapHeight },
                            m_modelDataline.clusters,
                            m_modelClusterDataLineAlphaClip,
							m_modelClusterDataLineTerrain,
                            m_sceneChangesSRV,
                            m_changeCounterSRV,
                            m_frustumMatchCountUAV,
                            lightAttributesHaveChanged);

#ifdef ENABLE_SHADOW_CACHE
                        if (!lightAttributesHaveChanged)
                            cmd.setPredicate(m_frustumMatchCountSRV, 0, PredicationOp::EqualZero);
#endif
                        cmd.clearDepthStencilView(m_shadowMapIndices[shadowMapIndex], 0.0f);

                        m_frustumCuller.expandClusters(cmd, m_modelDataline.clusters);
                        m_indexExpansion.expandIndexes(m_device, cmd, m_modelDataline.clusters, m_modelDataline.indexes, m_modelIndexDraw, m_modelDataline.draws);
						m_depthRenderer.render(cmd, m_modelDataline.clusters, m_modelIndexDraw, m_modelDataline.draws, m_shadowMapIndices[shadowMapIndex], m_shadowCamera, JitterOption::Disabled, AlphaClipOption::Disabled, BiasOption::Enabled, { ShadowMapWidth, ShadowMapHeight });

                        // the shadow maps gets cleared here???

                        m_modelDataline.draws.reset(cmd);
                        m_frustumCuller.expandClustersShadowAlphaClip(cmd, m_modelClusterDataLineAlphaClip);
                        m_indexExpansion.expandIndexes(m_device, cmd, m_modelClusterDataLineAlphaClip, m_modelDataline.indexes, m_modelIndexDrawAlphaClip, m_modelDataline.draws);
						m_depthRenderer.render(cmd, m_modelClusterDataLineAlphaClip, m_modelIndexDrawAlphaClip, m_modelDataline.draws, m_shadowMapIndices[shadowMapIndex], m_shadowCamera, JitterOption::Disabled, AlphaClipOption::Enabled, BiasOption::Enabled, { ShadowMapWidth, ShadowMapHeight });

						if (scene.terrains.size() > 0)
						{
							m_modelDataline.draws.reset(cmd);
							m_frustumCuller.expandClustersShadowTerrain(cmd, m_modelClusterDataLineTerrain);
							m_indexExpansion.expandIndexes(m_device, cmd, m_modelClusterDataLineTerrain, m_modelDataline.indexes, m_modelIndexDrawTerrain, m_modelDataline.draws, 600u);
							scene.terrains[0]->terrain().renderDepth(cmd, m_modelClusterDataLineTerrain, m_modelIndexDrawTerrain, m_modelDataline.draws, m_shadowMapIndices[shadowMapIndex], m_shadowCamera, JitterOption::Disabled, BiasOption::Enabled, { ShadowMapWidth, ShadowMapHeight });
						}

#ifdef ENABLE_SHADOW_CACHE
                        if (!lightAttributesHaveChanged)
                            cmd.setPredicate(BufferSRV{}, 0, PredicationOp::EqualZero);
#endif

						++shadowMapIndex;
                        ++spotLightIndex;
                    }
                }
				else
				{
					shadowId[i] = 0;
				}
            }
        }

        if(shadowCasterMapCount > 0)
            m_device.uploadBuffer(cmd, m_shadowVP, ByteRange(shadowViewProjectionMatrices));
		if (shadowId.size() > 0)
			m_device.uploadBuffer(cmd, m_lightIndexToShadowIndex, ByteRange(shadowId));

    }

    static int ChangedAreasLastCallRememberThis = 1000;
    void ShadowRenderer::computeChangedAreas(CommandList& /*cmd*/)
    {
        --ChangedAreasLastCallRememberThis;
        if (ChangedAreasLastCallRememberThis == 0)
        {
            ChangedAreasLastCallRememberThis = 1000;
            LOG("ShadowRenderer::computeChangedAreas not imlemented");
        }
    }
}
