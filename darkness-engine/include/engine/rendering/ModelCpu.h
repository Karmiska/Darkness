#pragma once

#include "containers/vector.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"
#include "engine/primitives/BoundingBox.h"

struct VertexScale;

namespace engine
{
    struct ModelCpu
    {
        engine::vector<engine::Vector3f> vertex;
        engine::vector<engine::Vector3f> normal;
        engine::vector<engine::Vector3f> tangent;
        engine::vector<engine::vector<engine::Vector4f>> color;
        engine::vector<engine::vector<engine::Vector2f>> uv;
        engine::vector<uint32_t> index;
        engine::BoundingBox boundingBox;
    };

    struct ModelPackedCpu
    {
        engine::vector<engine::Vector2<uint32_t>> vertex;
        engine::vector<engine::Vector2f> normal;
        engine::vector<engine::Vector2f> tangent;
        engine::vector<engine::vector<engine::Vector4<unsigned char>>> color;
        engine::vector<engine::vector<engine::Vector2f>> uv;
        engine::vector<uint16_t> index;
        engine::BoundingBox boundingBox;

        engine::vector<uint32_t> adjacency;
        engine::vector<BoundingBox> clusterBounds;
        engine::vector<engine::Vector4f> clusterCones;
        engine::vector<uint32_t> clusterVertexStarts;
        engine::vector<uint32_t> clusterIndexStarts;
        engine::vector<uint32_t> clusterIndexCount;
    };

    engine::Vector2<uint32_t> packVertex(const engine::Vector3f& vertex, const VertexScale& vertexScale);
    engine::Vector3f unpackVertex(const engine::Vector2<uint32_t>& vertex, const VertexScale& vertexScale);

	engine::vector<Vector2<uint32_t>> packVertexBuffer(const engine::vector<Vector3f>& vertex, const VertexScale& vertexScale);

    ModelPackedCpu packModel(ModelCpu& model, VertexScale vertexScale);
}
