#pragma once

#include "engine/primitives/Vector3.h"
#include "engine/primitives/BoundingBox.h"
#include "tools/Octree.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include <functional>

namespace engine
{
    struct Triangle
    {
        uint32_t index[3];
        bool emitted;
    };

    struct Edge
    {
        uint32_t a;
        uint32_t b;
    };

    class Clusterize
    {
    public:
        engine::vector<uint16_t> clusterize(const engine::vector<Vector3f>& vertex, const engine::vector<uint16_t>& index);
        engine::vector<uint32_t> clusterize(const engine::vector<Vector3f>& vertex, const engine::vector<uint32_t>& index);
        void clusterize32to16(
            const vector<Vector3f>& vertex,
            const vector<Vector3f>& normal,
            const vector<Vector3f>& tangent,
            const engine::vector<engine::vector<engine::Vector4f>>& color,
            const engine::vector<engine::vector<engine::Vector2f>>& uv,
            const vector<uint32_t>& index,
            engine::vector<Vector3f>& vertexOutput,
            engine::vector<Vector3f>& normalOutput,
            engine::vector<Vector3f>& tangentOutput,
            engine::vector<engine::vector<engine::Vector4f>>& colorOutput,
            engine::vector<engine::vector<engine::Vector2f>>& uvOutput,
            engine::vector<uint16_t>& indexOutput,
            engine::vector<uint32_t>& clusterVertexStartPointers);

        void clusterize32to16Metis(
            const vector<Vector3f>& vertex,
            const vector<Vector3f>& normal,
            const vector<Vector3f>& tangent,
            const engine::vector<engine::vector<engine::Vector4f>>& color,
            const engine::vector<engine::vector<engine::Vector2f>>& uv,
            const vector<uint32_t>& index,
            engine::vector<Vector3f>& vertexOutput,
            engine::vector<Vector3f>& normalOutput,
            engine::vector<Vector3f>& tangentOutput,
            engine::vector<engine::vector<engine::Vector4f>>& colorOutput,
            engine::vector<engine::vector<engine::Vector2f>>& uvOutput,
            engine::vector<uint16_t>& indexOutput,
            engine::vector<uint32_t>& clusterIndexStartPointers,
            engine::vector<uint32_t>& clusterVertexStartPointers);

    private:
        void generateTriangleList(const engine::vector<Vector3f>& vertex, const engine::vector<uint16_t>& index);
        void generateTriangleList(const engine::vector<Vector3f>& vertex, const engine::vector<uint32_t>& index);

        engine::vector<Triangle> m_triangles;
        engine::vector<uint32_t> m_notEmittedTriangle;
        BoundingBox m_meshBoundingBox;
    };
}
