#include "engine/rendering/LightData.h"
#include "tools/ByteRange.h"

using namespace tools;

namespace engine
{
    uint32_t LightData::count() const
    {
        return m_lightCount;
    }

    BufferSRV LightData::transforms()
    {
        return m_lightTransforms;
    }

    BufferSRV LightData::worldPositions()
    {
        return m_lightWorlPositions;
    }
    BufferSRV LightData::directions()
    {
        return m_lightDirections;
    }
    BufferSRV LightData::colors()
    {
        return m_lightColors;
    }
    BufferSRV LightData::intensities()
    {
        return m_lightIntensities;
    }
    BufferSRV LightData::ranges()
    {
        return m_lightRanges;
    }
    BufferSRV LightData::spotRanges()
    {
        return m_spotLightRanges;
    }
    BufferSRV LightData::pointRanges()
    {
        return m_pointLightRanges;
    }
    BufferSRV LightData::spotIds()
    {
        return m_spotLightIds;
    }
    BufferSRV LightData::pointIds()
    {
        return m_pointLightIds;
    }
    BufferSRV LightData::types()
    {
        return m_lightTypes;
    }

    BufferSRV LightData::spotTransforms()
    {
        return m_spotLightTransforms;
    }

    BufferSRV LightData::parameters()
    {
        return m_lightParameters;
    }

    const engine::vector<engine::Matrix4f>& LightData::cputransforms() const
    {
        return m_transforms;
    }

    const engine::vector<engine::Vector3f>& LightData::positions() const
    {
        return m_positions;
    }

    const engine::vector<engine::Vector3f>& LightData::directionVectors() const
    {
        return m_directions;
    }

    const engine::vector<float>& LightData::cpuranges() const
    {
        return m_ranges;
    }

    const engine::vector<float>& LightData::cpuspotranges() const
    {
        return m_spotranges;
    }

    const engine::vector<float>& LightData::cpupointranges() const
    {
        return m_pointranges;
    }

    const engine::vector<Vector4f>& LightData::cpuparameters() const
    {
        return m_parameters;
    }

    const engine::vector<bool>& LightData::shadowCaster() const
    {
        return m_shadowCaster;
    }

    const engine::vector<unsigned int>& LightData::engineTypes() const
    {
        return m_types;
    }

    bool LightData::changeHappened() const
    {
        return m_changeHappened;
    }

    void LightData::updateLightInfo(Device& device, CommandList& commandList, const engine::vector<FlatSceneLightNode>& lights)
    {
        bool sizeChanged = static_cast<uint32_t>(lights.size()) != m_lightCount;
        engine::vector<Vector4f> positions;
        engine::vector<Vector4f> directions;
        engine::vector<Vector4f> colors;
        engine::vector<float> intensities;
        engine::vector<float> ranges;
		engine::vector<float> spotranges;
		engine::vector<float> pointranges;
        engine::vector<Matrix4f> transforms;
		engine::vector<Matrix4f> spottransforms;
		engine::vector<uint32_t> spotids;
		engine::vector<uint32_t> pointids;
        engine::vector<unsigned int> types;
        engine::vector<Vector4f> parameters;

        bool positionsChanged = false;
        bool directionsChanged = false;
        bool colorsChanged = false;
        bool intensitiesChanged = false;
        bool parametersChanged = false;
        bool rangesChanged = false;
        bool typesChanged = false;
        for (auto&& light : lights)
        {
            positionsChanged |= light.positionChanged;
            directionsChanged |= light.rotationChanged;
            colorsChanged |= light.light->colorChanged(true);
            intensitiesChanged |= light.light->intensityChanged(true);
            rangesChanged |= light.light->rangeChanged(true);
            typesChanged |= light.light->lightTypeChanged(true);
            parametersChanged |= light.light->lightParametersChanged(true);
        }

        m_changeHappened =
            //positionsChanged |
            //directionsChanged |
            //colorsChanged |
            //intensitiesChanged |
            rangesChanged |
            typesChanged |
            parametersChanged;

        m_transforms.clear();
        m_positions.clear();
        m_directions.clear();
        m_ranges.clear();
		m_spotranges.clear();
		m_pointranges.clear();
		m_spottransforms.clear();
		m_parameters.clear();

        m_types.clear();
        m_shadowCaster.clear();

		uint32_t index = 0u;
        for (auto&& light : lights)
        {
            if (sizeChanged || positionsChanged)
                positions.emplace_back(Vector4f(light.position, 1.0f));

            if (sizeChanged || directionsChanged)
                directions.emplace_back(Vector4f(light.direction, 1.0f));

            if (sizeChanged || colorsChanged)
                colors.emplace_back(Vector4f(light.light->color(), 1.0f));

            if (sizeChanged || intensitiesChanged)
                intensities.emplace_back(light.light->intensity());

			if (sizeChanged || positionsChanged || directionsChanged)
				transforms.emplace_back(light.transform);

            if ((sizeChanged || positionsChanged || directionsChanged) && (light.light->lightType() == LightType::Spot))
                spottransforms.emplace_back(light.transform);

			if (sizeChanged || rangesChanged)
			{
				ranges.emplace_back(light.light->range());
				if (light.light->lightType() == LightType::Spot)
				{
					spotranges.emplace_back(light.light->range());
					spotids.emplace_back(index);
				}
				else if (light.light->lightType() == LightType::Point)
				{
					pointranges.emplace_back(light.light->range());
					pointids.emplace_back(index);
				}
			}

            if (sizeChanged || typesChanged)
                types.emplace_back(static_cast<unsigned int>(light.light->lightType()));

            if (sizeChanged || parametersChanged)
                parameters.emplace_back(light.light->parameters());

            m_transforms.emplace_back(light.transform);
            m_positions.emplace_back(light.position);
            m_directions.emplace_back(light.direction);
            m_ranges.emplace_back(light.range);
			m_spotranges.emplace_back(light.range);
			m_pointranges.emplace_back(light.range);
            m_types.emplace_back(static_cast<unsigned int>(light.type));
            m_shadowCaster.emplace_back(light.shadowCaster);
			m_spottransforms.emplace_back(light.transform);
			m_parameters.emplace_back(light.light->parameters());

			++index;
        }

        if (sizeChanged)
        {
            m_lightCount = static_cast<uint32_t>(lights.size());

            if (m_lightCount > 0)
            {
                m_lightTransforms = device.createBufferSRV(BufferDescription()
                    .name("lightTransforms")
                    .usage(ResourceUsage::GpuReadWrite)
                    .structured(true)
                    .elements(transforms.size())
                    .elementSize(sizeof(float4x4))
                    .setInitialData(BufferDescription::InitialData(transforms)));

                m_lightWorlPositions = device.createBufferSRV(BufferDescription()
                    .name("lightPositions")
                    .format(Format::R32G32B32A32_FLOAT)
                    .setInitialData(BufferDescription::InitialData(positions)));

                m_lightDirections = device.createBufferSRV(BufferDescription()
                    .name("lightDirections")
                    .format(Format::R32G32B32A32_FLOAT)
                    .setInitialData(BufferDescription::InitialData(directions)));

                m_lightColors = device.createBufferSRV(BufferDescription()
                    .name("lightColors")
                    .format(Format::R32G32B32A32_FLOAT)
                    .setInitialData(BufferDescription::InitialData(colors)));

                m_lightIntensities = device.createBufferSRV(BufferDescription()
                    .name("lightIntensities")
                    .format(Format::R32_FLOAT)
                    .setInitialData(BufferDescription::InitialData(intensities)));

				if(ranges.size() > 0)
                m_lightRanges = device.createBufferSRV(BufferDescription()
                    .name("lightRanges")
                    .format(Format::R32_FLOAT)
                    .setInitialData(BufferDescription::InitialData(ranges)));

				if(spotranges.size() > 0)
					m_spotLightRanges = device.createBufferSRV(BufferDescription()
						.name("spotLightRanges")
						.format(Format::R32_FLOAT)
						.setInitialData(BufferDescription::InitialData(spotranges)));

				if(pointranges.size() > 0)
					m_pointLightRanges = device.createBufferSRV(BufferDescription()
						.name("pointLightRanges")
						.format(Format::R32_FLOAT)
						.setInitialData(BufferDescription::InitialData(pointranges)));

				if (spotids.size() > 0)
					m_spotLightIds = device.createBufferSRV(BufferDescription()
						.name("spotLightIds")
						.format(Format::R32_UINT)
						.setInitialData(BufferDescription::InitialData(spotids)));

				if (pointids.size() > 0)
					m_pointLightIds = device.createBufferSRV(BufferDescription()
						.name("pointLightIds")
						.format(Format::R32_UINT)
						.setInitialData(BufferDescription::InitialData(pointids)));

                m_lightTypes = device.createBufferSRV(BufferDescription()
                    .name("lightTypes")
                    .format(Format::R32_UINT)
                    .setInitialData(BufferDescription::InitialData(types)));

                m_lightParameters = device.createBufferSRV(BufferDescription()
                    .name("lightParameters")
                    .format(Format::R32G32B32A32_FLOAT)
                    .setInitialData(BufferDescription::InitialData(parameters)));

				if(spottransforms.size() > 0)
					m_spotLightTransforms = device.createBufferSRV(BufferDescription()
						.name("spotTransforms")
						.usage(ResourceUsage::GpuReadWrite)
						.structured(true)
						.elements(spottransforms.size())
						.elementSize(sizeof(float4x4))
						.setInitialData(BufferDescription::InitialData(spottransforms)));
            }
            else
            {
				m_lightWorlPositions = {};
                m_lightDirections = {};
                m_lightColors = {};
                m_lightIntensities = {};
                m_lightRanges = {};
				m_spotLightRanges = {};
				m_pointLightRanges = {};
				m_spotLightIds = {};
				m_pointLightIds = {};
                m_lightTypes = {};
                m_lightParameters = {};
				m_spotLightTransforms = {};
            }
        }
        else
        {
            if (m_lightCount == 0)
            {
                m_lightWorlPositions = {};
                m_lightDirections = {};
                m_lightColors = {};
                m_lightIntensities = {};
                m_lightRanges = {};
				m_spotLightRanges = {};
				m_pointLightRanges = {};
				m_spotLightIds = {};
				m_pointLightIds = {};
                m_lightTypes = {};
                m_lightParameters = {};
				m_spotLightTransforms = {};
            }
            else
            {
                // update existing
                if (positionsChanged)
                    device.uploadBuffer(commandList, m_lightWorlPositions, ByteRange{ positions });

                if (directionsChanged)
                    device.uploadBuffer(commandList, m_lightDirections, ByteRange{ directions });

                if (colorsChanged)
                    device.uploadBuffer(commandList, m_lightColors, ByteRange{ colors });

                if (intensitiesChanged)
                    device.uploadBuffer(commandList, m_lightIntensities, ByteRange{ intensities });

                if (rangesChanged && m_lightRanges)
                    device.uploadBuffer(commandList, m_lightRanges, ByteRange{ ranges });

				if (rangesChanged && m_spotLightRanges)
					device.uploadBuffer(commandList, m_spotLightRanges, ByteRange{ spotranges });

				if (rangesChanged && m_pointLightRanges)
					device.uploadBuffer(commandList, m_pointLightRanges, ByteRange{ pointranges });

				if (rangesChanged && m_spotLightIds)
					device.uploadBuffer(commandList, m_spotLightIds, ByteRange{ spotids });

				if (rangesChanged && m_pointLightIds)
					device.uploadBuffer(commandList, m_pointLightIds, ByteRange{ pointids });

                if (typesChanged)
                    device.uploadBuffer(commandList, m_lightTypes, ByteRange{ types });

                if (parametersChanged)
                    device.uploadBuffer(commandList, m_lightParameters, ByteRange{ parameters });

                if (rangesChanged || positionsChanged || directionsChanged)
                    device.uploadBuffer(commandList, m_lightTransforms, ByteRange{ transforms });

				if ((rangesChanged || positionsChanged || directionsChanged) && m_spotLightTransforms)
					device.uploadBuffer(commandList, m_spotLightTransforms, ByteRange{ spottransforms });
            }
        }

    }
}
