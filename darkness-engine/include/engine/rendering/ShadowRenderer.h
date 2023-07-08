#pragma once

#include "engine/graphics/CommandList.h"
#include "components/Transform.h"
#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/LightComponent.h"
#include "components/Camera.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/Scene.h"
#include "engine/rendering/LightData.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/culling/FrustumCuller.h"
#include "engine/rendering/culling/OcclusionCuller.h"
#include "engine/rendering/culling/IndexExpansion.h"
#include "engine/rendering/DepthPyramid.h"
#include "engine/rendering/culling/ModelDataLine.h"
#include "containers/memory.h"
#include "engine/rendering/renderers/RenderDepth.h"
#include "shaders/core/shadow/ComputeChangedScene.h"

namespace engine
{
    constexpr int ShadowDataLineCount = 2;

    class ShadowRenderer
    {
    public:
        ShadowRenderer(Device& device);

        void render(
            CommandList& cmd,
            FlatScene& scene
        );

        void computeChangedAreas(CommandList& cmd);

        void refresh()
        {
            m_refresh = true;
        }

        engine::Camera& shadowCamera()
        {
            return m_shadowCamera;
        }

        TextureSRV shadowMap()
        {
            return m_shadowMapSRV;
        }

        BufferSRV shadowVP()
        {
            return m_shadowVP;
        }

		BufferSRV lightIndexToShadowIndex()
		{
			return m_lightIndexToShadowIndex;
		}

    private:
        uint32_t shadowCasterCount(const engine::LightData& lightData) const;
        void updateShadowMaps(uint32_t shadowCasterMapCount, uint32_t lightCount);

    private:
        Device& m_device;
		RenderDepth m_depthRenderer;

        FrustumCuller m_frustumCuller;
        IndexExpansion m_indexExpansion;

        engine::Pipeline<shaders::ComputeChangedScene> m_computeChangedScene;

        ModelDataLine m_modelDataline;
        ClusterDataLine m_modelClusterDataLineAlphaClip;
		ClusterDataLine m_modelClusterDataLineTerrain;
        IndexDataLine m_modelIndexDraw;
        IndexDataLine m_modelIndexDrawAlphaClip;
		IndexDataLine m_modelIndexDrawTerrain;

        engine::Camera m_shadowCamera;
        TextureDSVOwner m_shadowMap;
        TextureSRVOwner m_shadowMapSRV;
        BufferSRVOwner m_shadowVP;
		BufferSRVOwner m_lightIndexToShadowIndex;
        engine::vector<TextureDSVOwner> m_shadowMapIndices;

        engine::Camera m_cubemapCamera;
        bool m_refresh;
		uint32_t m_lastLightCount;
        uint32_t m_lastShadowCasterCount;

        BufferUAVOwner m_sceneChanges;
        BufferUAVOwner m_changeCounter;
        BufferSRVOwner m_sceneChangesSRV;
        BufferSRVOwner m_changeCounterSRV;

        BufferOwner m_frustumMatchCount;
        BufferUAVOwner m_frustumMatchCountUAV;
        BufferSRVOwner m_frustumMatchCountSRV;

        struct LightCacheInfo
        {
            Vector3f position;
            Vector3f direction;
            float range;
            float outerAngle;
        };
        engine::vector<LightCacheInfo> m_lightCache;

        void reallocateClusterDatalines(FlatScene& scene);
    };
}
