#include "engine/rendering/ShapeMeshFactory.h"
#include "engine/graphics/Device.h"

namespace engine
{
	Shape ShapeMeshFactory::createTriangle(Vector3f a, Vector3f b, Vector3f c)
	{
		return Shape{
			{ a, b, c },																// vertices
			{ { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f } },							// uvs
			{ { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },		// normals
			{ { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },		// tangents
			{ { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },	// bitangents
			{ 0, 1, 2 }
		};
	}

	Shape ShapeMeshFactory::createCube(Vector3f center, Vector3f size)
	{
		auto h = size / 2.0f;
		return {
			{   // vertices
				{ center.x - h.x, center.y + h.y, center.z - h.z },	// top		0
				{ center.x - h.x, center.y + h.y, center.z + h.z },
				{ center.x + h.x, center.y + h.y, center.z + h.z },
				{ center.x + h.x, center.y + h.y, center.z - h.z },			//	3

				{ center.x - h.x, center.y - h.y, center.z + h.z },	// bottom	4
				{ center.x - h.x, center.y - h.y, center.z - h.z },
				{ center.x + h.x, center.y - h.y, center.z - h.z },
				{ center.x + h.x, center.y - h.y, center.z + h.z },			//	7

				{ center.x - h.x, center.y + h.y, center.z + h.z },	// front	8
				{ center.x - h.x, center.y - h.y, center.z + h.z },
				{ center.x + h.x, center.y - h.y, center.z + h.z },
				{ center.x + h.x, center.y + h.y, center.z + h.z },			//	11

				{ center.x + h.x, center.y + h.y, center.z - h.z },	// back		12
				{ center.x + h.x, center.y - h.y, center.z - h.z },
				{ center.x - h.x, center.y - h.y, center.z - h.z },
				{ center.x - h.x, center.y + h.y, center.z - h.z },			//	15

				{ center.x + h.x, center.y + h.y, center.z + h.z },	// right	16
				{ center.x + h.x, center.y - h.y, center.z + h.z },
				{ center.x + h.x, center.y - h.y, center.z - h.z },
				{ center.x + h.x, center.y + h.y, center.z - h.z },			//	19

				{ center.x - h.x, center.y + h.y, center.z - h.z },	// left		20
				{ center.x - h.x, center.y - h.y, center.z - h.z },
				{ center.x - h.x, center.y - h.y, center.z + h.z },
				{ center.x - h.x, center.y + h.y, center.z + h.z }			//	23
			},
			{	// uvs
				{ 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f },	// top
				{ 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f },	// bottom
				{ 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f },	// front
				{ 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f },	// back
				{ 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f },	// right
				{ 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f },	// left
			},
			{	// normals
				{ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },		// top
				{ 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, // bottom
				{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },		// front
				{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, // back
				{ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },		// right
				{ -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, // left
			},
			{	// tangents
				{ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },			// top
				{ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },			// bottom
				{ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },			// front
				{ -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },		// back
				{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },		// right
				{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },			// left
			},
			{	// bitangents
				{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },			// top
				{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },		// bottom
				{ 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },		// front
				{ 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },		// back
				{ 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },		// right
				{ 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },		// left
			},
			{
				0, 1, 2, 2, 3, 0,		// top
				4, 5, 6, 6, 7, 4,		// bottom
				8, 9, 10, 10, 11, 8,	// front
				12, 13, 14, 14, 15, 12,	// back
				16, 17, 18, 18, 19, 16,	// right
				20, 21, 22, 22, 23, 20
			}
		};
	}

	Shape ShapeMeshFactory::createSphere(Vector3f /*center*/, float radius, int segments /* vertical lines */, int rings /* horizontal lines */)
	{
		Shape shape;
		
		{
			float verticalAngleInc = static_cast<float>(PI) / static_cast<float>(rings - 1);
			float angleInc = static_cast<float>(TWO_PI) / static_cast<float>(segments - 1);

			float verticalAngle = 0.0f;
			auto ringStart = shape.vertices.size();
			for (int horiz = 0; horiz < rings; ++horiz)
			{
				float radiusCurve = radius * sinf(verticalAngle);
				float angle = 0.0f;

				auto previousRingStart = ringStart;
				ringStart = shape.vertices.size();

				for (int vertical = 0; vertical < segments; ++vertical)
				{
					shape.vertices.emplace_back(Vector3f{ radiusCurve * cosf(angle), radius * cosf(verticalAngle), radiusCurve * sinf(angle) });
					shape.uvs.emplace_back(Vector2f{ static_cast<float>(vertical) / static_cast<float>(segments), static_cast<float>(horiz) / static_cast<float>(rings) });
					shape.normals.emplace_back(shape.vertices.back().normalize());
					angle += angleInc;

					if (horiz > 0 && vertical > 0)
					{
						shape.indexes.emplace_back(static_cast<uint32_t>(previousRingStart + (vertical - 1)));
						shape.indexes.emplace_back(static_cast<uint32_t>(ringStart + (vertical - 1)));
						shape.indexes.emplace_back(static_cast<uint32_t>(ringStart + vertical));
						shape.indexes.emplace_back(static_cast<uint32_t>(ringStart + vertical));
						shape.indexes.emplace_back(static_cast<uint32_t>(previousRingStart + vertical));
						shape.indexes.emplace_back(static_cast<uint32_t>(previousRingStart + (vertical - 1)));
					}
				}
				verticalAngle += verticalAngleInc;
			}
		}
		{
			auto ringStart = shape.tangents.size();
			for (int horiz = 0; horiz < rings; ++horiz)
			{
				ringStart = shape.tangents.size();
				for (int vertical = 0; vertical < segments; ++vertical)
				{
					auto prev = vertical - 1;
					if (prev < 0)
						prev = segments - 1;
					auto next = vertical + 1;
					if (next > segments - 1)
						next = 0;
					shape.tangents.emplace_back((shape.vertices[ringStart + next] - shape.vertices[ringStart + prev]).normalize());
					
					// todo
					shape.bitangents.emplace_back(shape.tangents.back());
				}
			}
		}

		return shape;
	}

	Shape ShapeMeshFactory::createSpot(Vector3f /*position*/, Vector3f /*direction*/, float range, float coneAngle, int sectors)
	{
		Shape result;
		float angleIncrementRad = static_cast<float>(TWO_PI) / static_cast<float>(sectors - 1);
		float currentAngle = 0.0f;
		float distance = cos(coneAngle * DEG_TO_RAD) * range;
		for (int i = 0; i < sectors; ++i)
		{
			result.vertices.emplace_back(Vector3f{range * cosf(currentAngle), range * sinf(currentAngle), distance });
			result.uvs.emplace_back(Vector2f{ 0.0f, 0.0f });
			result.normals.emplace_back(Vector3f{ 0.0f, 0.0f, 0.0f });
			result.tangents.emplace_back(Vector3f{ 0.0f, 0.0f, 0.0f });
			result.bitangents.emplace_back(Vector3f{ 0.0f, 0.0f, 0.0f });
			currentAngle += angleIncrementRad;
		}
		auto zeroIndex = result.vertices.size();
		result.vertices.emplace_back(Vector3f{ 0.0f, 0.0f, 0.0f });
		auto farIndex = result.vertices.size();
		result.vertices.emplace_back(Vector3f{ 0.0f, 0.0f, distance });
		for (int i = 0; i < sectors; ++i)
		{
			auto index_this = i;
			auto index_next = i + 1;
			if (index_next > sectors - 1)
				index_next = 0;

			result.indexes.emplace_back(static_cast<uint32_t>(index_this));
			result.indexes.emplace_back(static_cast<uint32_t>(zeroIndex));
			result.indexes.emplace_back(static_cast<uint32_t>(index_next));

			result.indexes.emplace_back(static_cast<uint32_t>(index_next));
			result.indexes.emplace_back(static_cast<uint32_t>(farIndex));
			result.indexes.emplace_back(static_cast<uint32_t>(index_this));
		}

		return result;
	}

	GpuShape::GpuShape(Device& device, const Shape& shape)
		: vertices{ device.createBufferSRV(BufferDescription().name("GpuShapeVertices").setInitialData(shape.vertices).format(Format::R32G32B32_FLOAT)) }
		, uvs{ device.createBufferSRV(BufferDescription().name("GpuShapeUvs").setInitialData(shape.uvs).format(Format::R32G32_FLOAT)) }
		, normals{ device.createBufferSRV(BufferDescription().name("GpuShapeNormals").setInitialData(shape.normals).format(Format::R32G32B32_FLOAT)) }
		, tangents{ device.createBufferSRV(BufferDescription().name("GpuShapeTangents").setInitialData(shape.tangents).format(Format::R32G32B32_FLOAT)) }
		, bitangents{ device.createBufferSRV(BufferDescription().name("GpuShapeBitangents").setInitialData(shape.bitangents).format(Format::R32G32B32_FLOAT)) }
		, indexes{ device.createBufferIBV(BufferDescription().name("GpuShapeIndexes").setInitialData(shape.indexes).format(Format::R32_UINT)) }
		, view_vertices{ vertices.resource() }
		, view_uvs{ uvs.resource() }
		, view_normals{ normals.resource() }
		, view_tangents{ tangents.resource() }
		, view_bitangents{ bitangents.resource() }
		, view_indexes{ indexes.resource() }
	{}
}