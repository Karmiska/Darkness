#pragma once

#include "engine/graphics/CommandList.h"
#include "engine/Scene.h"
#include "engine/primitives/Matrix4.h"
#include "containers/vector.h"
#include <queue>

namespace engine
{
    class LightData
    {
    public:
        void updateLightInfo(
			Device& device, 
			CommandList& commandList, 
			const engine::vector<FlatSceneLightNode>& lights);

        uint32_t count() const;

        BufferSRV transforms();

        BufferSRV worldPositions();
        BufferSRV directions();
        BufferSRV colors();
        BufferSRV intensities();
        BufferSRV ranges();
        BufferSRV spotRanges();
        BufferSRV pointRanges();
        BufferSRV spotIds();
        BufferSRV pointIds();
        BufferSRV types();

        BufferSRV spotTransforms();

        BufferSRV parameters();

        const engine::vector<engine::Matrix4f>& cputransforms() const;

        const engine::vector<engine::Vector3f>& positions() const;

        const engine::vector<engine::Vector3f>& directionVectors() const;

        const engine::vector<float>& cpuranges() const;

        const engine::vector<float>& cpuspotranges() const;

        const engine::vector<float>& cpupointranges() const;

        const engine::vector<Vector4f>& cpuparameters() const;

        const engine::vector<bool>& shadowCaster() const;

        const engine::vector<unsigned int>& engineTypes() const;

        bool changeHappened() const;

    private:
        bool m_changeHappened = false;
        uint32_t m_lightCount;
		BufferSRVOwner m_lightTransforms;
        BufferSRVOwner m_lightWorlPositions;
        BufferSRVOwner m_lightDirections;
        BufferSRVOwner m_lightColors;
        BufferSRVOwner m_lightIntensities;
        BufferSRVOwner m_lightRanges;
		BufferSRVOwner m_spotLightRanges;
		BufferSRVOwner m_pointLightRanges;
		BufferSRVOwner m_spotLightIds;
		BufferSRVOwner m_pointLightIds;
        BufferSRVOwner m_lightTypes;
        BufferSRVOwner m_lightParameters;
		BufferSRVOwner m_spotLightTransforms;

        engine::vector<engine::Matrix4f> m_transforms;
        engine::vector<engine::Vector3f> m_positions;
        engine::vector<engine::Vector3f> m_directions;
        engine::vector<float> m_ranges;
		engine::vector<float> m_spotranges;
		engine::vector<float> m_pointranges;
        engine::vector<unsigned int> m_types;
        engine::vector<bool> m_shadowCaster;
		engine::vector<Matrix4f> m_spottransforms;
		engine::vector<Vector4f> m_parameters;
    };
}
