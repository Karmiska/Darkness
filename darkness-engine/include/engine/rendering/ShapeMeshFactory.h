#pragma once

#include "engine/graphics/ResourceOwners.h"
#include "engine/primitives/Vector3.h"

namespace engine
{
	struct Shape
	{
		engine::vector<Vector3f> vertices;
		engine::vector<Vector2f> uvs;
		engine::vector<Vector3f> normals;
		engine::vector<Vector3f> tangents;
		engine::vector<Vector3f> bitangents;
		engine::vector<uint32_t> indexes;
	};

	class Device;
	struct GpuShape
	{
		BufferSRVOwner vertices;
		BufferSRVOwner uvs;
		BufferSRVOwner normals;
		BufferSRVOwner tangents;
		BufferSRVOwner bitangents;
		BufferIBVOwner indexes;

		BufferSRV view_vertices;
		BufferSRV view_uvs;
		BufferSRV view_normals;
		BufferSRV view_tangents;
		BufferSRV view_bitangents;
		BufferIBV view_indexes;

		GpuShape(Device& device, const Shape& shape);
	};

	class ShapeMeshFactory
	{
	public:
		static Shape createTriangle(Vector3f a, Vector3f b, Vector3f c);
		static Shape createCube(Vector3f center, Vector3f size);
		static Shape createSphere(Vector3f center, float radius, int segments /* vertical lines */, int rings /* horizontal lines */);
		static Shape createSpot(Vector3f position, Vector3f direction, float range, float coneAngle, int sectors);
	};
}
