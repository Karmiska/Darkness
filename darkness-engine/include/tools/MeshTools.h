#pragma once

#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"
#include "containers/vector.h"

namespace engine
{
    engine::vector<uint32_t> meshGenerateAdjacency(const engine::vector<uint32_t>& indices, const engine::vector<Vector3f>& vertices);
    void meshGenerateAdjacency(
        uint32_t* indexes, uint32_t indexCount,
        Vector3f* vertices, uint32_t vertexCount,
        uint32_t* results, uint32_t resultCount,
        uint32_t startingIndex);

    // Packs a 3-component normal to 2 channels using octahedron normals
    Vector2f packNormalOctahedron(const Vector3f& v);

    // Unpacking from octahedron normals, input is the output from pack_normal_octahedron
    Vector3f unpackNormalOctahedron(const Vector2f& packed_nrm);
}
