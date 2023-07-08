#include "components/TerrainComponent.h"
#include "components/Transform.h"
#include "engine/graphics/Device.h"
#include "engine/rendering/TerrainRenderer.h"

namespace engine
{
    /*Patch::Patch(
        const Vector2f& heightRange,
        const Vector2f& topLeft,
        const Vector2f& bottomRight)
        : m_heightRange{ heightRange }
        , m_topLeft{ topLeft }
        , m_bottomRight{ bottomRight }
    {}

    void Patch::updateGPU(Device& device)
    {
        auto& residency =  device.modelResources().residency();

        // allocate vertex data
        m_resource.modelResource = device.modelResources().gpuBuffers().vertexDataAllocator().allocate(64);

        // allocate upload data
        auto upload = residency.createUpdateAllocation(64 * sizeof(Vector2<uint32_t>));

        upload.gpuIndex = m_resource.modelResource.gpuIndex;

        void* src = nullptr;
        size_t bytes = 0;
        memcpy(reinterpret_cast<char*>(upload.ptr), src, bytes);

        residency.makeResident(upload);
        residency.freeUpdateAllocation(upload);
    }

    Terrain::Terrain()
    {

    }

    void Terrain::updateCPU()
    {

    }

    void Terrain::updateGPU(Device& device)
    {
        
    }

    TerrainComponent::TerrainComponent()
        : m_height{ this, "height", float(10.0f), [this]() { 
            //this->m_rangeChanged = true;
        }}
        , m_terrain{ engine::make_shared<Terrain>() }
    {
        m_name = "Terrain";
    }*/

	bool SceneDeserialize::String(const char* str, SizeType length, bool /*copy*/)
	{
		/*if(copy)
			LOG_INFO("Scene serialization does not implement string copy");*/
		if (m_keyStack.top() == serialization::KeyTypes::NodeName)
		{
			m_nodeCurrent.top()->name(engine::string(str, length));
		}
		else if (m_keyStack.top() == serialization::KeyTypes::ComponentName)
		{
			engine::shared_ptr<EngineComponent> newComponent = nullptr;
			engine::string componentName = engine::string(str, length);
			if (componentName == "Transform")
				newComponent = engine::make_shared<Transform>();
			else if (componentName == "MeshRenderer")
				newComponent = engine::make_shared<MeshRendererComponent>();
			else if (componentName == "Camera")
				newComponent = engine::make_shared<Camera>();
			else if (componentName == "MaterialComponent")
				newComponent = engine::make_shared<MaterialComponent>();
			else if (componentName == "LightComponent")
				newComponent = engine::make_shared<LightComponent>();
			else if (componentName == "PostprocessComponent")
				newComponent = engine::make_shared<PostprocessComponent>();
			else if (componentName == "Probe")
				newComponent = engine::make_shared<ProbeComponent>();
			else if (componentName == "TerrainComponent")
				newComponent = engine::make_shared<TerrainComponent>();
			else
				ASSERT(false, "Scene could not deserialize component. Unknown component: %s", componentName.c_str());

			m_nodeCurrent.top()->addComponent(newComponent);
			m_componentCurrent = newComponent;
		}
		else if (m_keyStack.top() == serialization::KeyTypes::PropertyName)
		{
			engine::string propertyValue(str, length);
			m_propertyName = engine::string(str, length);
		}
		else if (m_keyStack.top() == serialization::KeyTypes::PropertyValue)
		{
			engine::string propertyValue(str, length);
			switch (m_propertyTypeId)
			{
			case serialization::TypeId::InvalidId:
			{
				break;
			}
			case serialization::TypeId::Vector2f:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<Vector2f>(readJsonValue_vector2f(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<Vector2f>(m_propertyName, readJsonValue_vector2f(propertyValue));
				break;
			}
			case serialization::TypeId::Vector3f:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<Vector3f>(readJsonValue_vector3f(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<Vector3f>(m_propertyName, readJsonValue_vector3f(propertyValue));
				break;
			}
			case serialization::TypeId::Vector4f:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<Vector4f>(readJsonValue_vector4f(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<Vector4f>(m_propertyName, readJsonValue_vector4f(propertyValue));
				break;
			}
			case serialization::TypeId::Matrix3f:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<Matrix3f>(readJsonValue_matrix3f(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<Matrix3f>(m_propertyName, readJsonValue_matrix3f(propertyValue));
				break;
			}
			case serialization::TypeId::Matrix4f:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<Matrix4f>(readJsonValue_matrix4f(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<Matrix4f>(m_propertyName, readJsonValue_matrix4f(propertyValue));
				break;
			}
			case serialization::TypeId::Quaternionf:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<Quaternionf>(readJsonValue_quaternionf(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<Quaternionf>(m_propertyName, readJsonValue_quaternionf(propertyValue));
				break;
			}
			case serialization::TypeId::String:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<engine::string>(readJsonValue_string(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<engine::string>(m_propertyName, readJsonValue_string(propertyValue));
				break;
			}
			case serialization::TypeId::Projection:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<engine::Projection>(readJsonValue_projection(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<engine::Projection>(m_propertyName, readJsonValue_projection(propertyValue));
				break;
			}
			case serialization::TypeId::LightType:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<engine::LightType>(readJsonValue_lightType(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<engine::LightType>(m_propertyName, readJsonValue_lightType(propertyValue));
				break;
			}
			case serialization::TypeId::CollisionShape:
			{
				if (m_componentCurrent->hasVariant(m_propertyName))
					m_componentCurrent->variant(m_propertyName).value<engine::CollisionShape>(readJsonValue_collisionShape(propertyValue));
				else
					m_componentCurrent->insertLoadedValue<engine::CollisionShape>(m_propertyName, readJsonValue_collisionShape(propertyValue));
				break;
			}
			default:
			{
				ASSERT(false, "Serialization failed");
				break;
			}
			}
		}
		m_keyStack.pop();

		//engine::string temp(str, length);
		//OutputDebugStringA((identString() + engine::string("String("+temp+")\n")).data());
		return true;
	}


    engine::shared_ptr<engine::EngineComponent> TerrainComponent::clone() const
    {
        auto terrain = engine::make_shared<engine::TerrainComponent>();
		terrain->settings(settings());
		return terrain;
    }

    void TerrainComponent::update(Device& device)
    {
		if (!m_transform)
			m_transform = getComponent<Transform>();

		if (!m_terrain)
		{
			m_terrain = std::make_shared<TerrainRenderer>(device);
			m_terrain->updateTransform(m_lastMatrix);
		}

		if (m_worldSizeChanged)
		{
			m_worldSizeChanged = false;
			m_settings.worldSize = m_worldSize.value<Vector3f>();
		}

		if (m_sectorCountChanged)
		{
			m_sectorCountChanged = false;
			m_settings.sectorCount = m_sectorCount.value<Vector3f>();
		}

		if (m_cellCountChanged)
		{
			m_cellCountChanged = false;
			m_settings.cellCount = m_cellCount.value<Vector3f>();
		}

		if (m_nodeCountChanged)
		{
			m_nodeCountChanged = false;
			m_settings.nodeCount = m_nodeCount.value<Vector3f>();
		}

		if (m_sectorSizeChanged)
		{
			m_sectorSizeChanged = false;
			m_settings.sectorSize = m_sectorSize.value<Vector3f>();
		}

		m_terrain->settings(m_settings);
    }

	void TerrainComponent::settings(const TerrainSettings& settings)
	{
		m_settings = settings;
	}

	TerrainSettings TerrainComponent::settings() const
	{
		return m_settings;
	}

	TerrainRenderer& TerrainComponent::terrain()
	{
		return *m_terrain;
	}

	Transform& TerrainComponent::transform()
	{
		return *m_transform;
	}

	void TerrainComponent::updateTransform(const Matrix4f& mat)
	{
		if (m_terrain)
			m_terrain->updateTransform(mat);
		else
			m_lastMatrix = mat;
	}
}
