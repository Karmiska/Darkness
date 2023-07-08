#pragma once

#include "engine/EngineComponent.h"
//#include "engine/rendering/ResidencyManager.h"
#include "tools/Property.h"
#include "engine/rendering/TerrainSettings.h"
#include "components/Transform.h"
#include "engine/graphics/Device.h"

namespace engine
{
	class TerrainRenderer;
    /*class Transform;
    class Device;

    class Patch
    {
    public:
        Patch(
            const Vector2f& heightRange,
            const Vector2f& topLeft,
            const Vector2f& bottomRight);

        void updateGPU(Device& device);

    private:
        Vector2f m_heightRange;
        Vector2f m_topLeft;
        Vector2f m_bottomRight;
        ModelResource m_resource;
    };

    class Terrain
    {
    public:
        Terrain();

        void updateCPU();
        void updateGPU(Device& device);
    private:
        engine::vector<Patch> m_patches;
    };*/

    class TerrainComponent : public EngineComponent
    {
        Property m_worldSize;
		Property m_sectorCount;
		Property m_cellCount;
		Property m_nodeCount;
		Property m_sectorSize;

		bool m_worldSizeChanged;
		bool m_sectorCountChanged;
		bool m_cellCountChanged;
		bool m_nodeCountChanged;
		bool m_sectorSizeChanged;
    public:
        TerrainComponent()
			: m_worldSize{ this, "WorldSize", Vector3f{ 8096.0f, 2048.0f, 8096.0f }, [this]() { this->m_worldSizeChanged = true; } }
			, m_sectorCount{ this, "SectorCount", Vector3f{ 1.0f, 1.0f, 1.0f }, [this]() { this->m_sectorCountChanged = true; } }
			, m_cellCount{ this, "CellCount", Vector3f{ 100.0f, 1.0f, 100.0f }, [this]() { this->m_cellCountChanged = true; } }
			, m_nodeCount{ this, "NodeCount", Vector3f{ 10.0f, 1.0f, 10.0f }, [this]() { this->m_nodeCountChanged = true; } }
			, m_sectorSize{ this, "SectorSize", Vector3f{ 16384.0f, 1.0f, 16384.0f }, [this]() { this->m_sectorSizeChanged = true; } }
			, m_worldSizeChanged{ true }
			, m_sectorCountChanged{ true }
			, m_cellCountChanged{ true }
			, m_nodeCountChanged{ true }
			, m_sectorSizeChanged{ true }
			, m_transform{ nullptr }
		{
			m_name = "TerrainComponent";
		}

		TerrainComponent(engine::shared_ptr<Transform> transform)
			: m_worldSize{ this, "WorldSize", Vector3f{ 8096.0f, 2048.0f, 8096.0f }, [this]() { this->m_worldSizeChanged = true; } }
			, m_sectorCount{ this, "SectorCount", Vector3f{ 1.0f, 1.0f, 1.0f }, [this]() { this->m_sectorCountChanged = true; } }
			, m_cellCount{ this, "CellCount", Vector3f{ 100.0f, 1.0f, 100.0f }, [this]() { this->m_cellCountChanged = true; } }
			, m_nodeCount{ this, "NodeCount", Vector3f{ 10.0f, 1.0f, 10.0f }, [this]() { this->m_nodeCountChanged = true; } }
			, m_sectorSize{ this, "SectorSize", Vector3f{ 16384.0f, 1.0f, 16384.0f }, [this]() { this->m_sectorSizeChanged = true; } }
			, m_worldSizeChanged{ true }
			, m_sectorCountChanged{ true }
			, m_cellCountChanged{ true }
			, m_nodeCountChanged{ true }
			, m_sectorSizeChanged{ true }
			, m_transform{ transform }
		{
			m_name = "TerrainComponent";
		}

        engine::shared_ptr<engine::EngineComponent> clone() const override;

        void update(Device& device);

		void settings(const TerrainSettings& settings);

		TerrainSettings settings() const;

		TerrainRenderer& terrain();

		Transform& transform();

		void updateTransform(const Matrix4f& mat);
    private:
        engine::shared_ptr<Transform> m_transform;
        engine::shared_ptr<TerrainRenderer> m_terrain;
		TerrainSettings m_settings;
		Matrix4f m_lastMatrix;
    };
}
