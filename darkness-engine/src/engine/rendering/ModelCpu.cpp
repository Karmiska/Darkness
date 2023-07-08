#include "engine/rendering/ModelCpu.h"
#include "tools/MeshTools.h"
#include "shaders/core/shared_types/VertexScale.hlsli"
#include <algorithm>

namespace engine
{
    engine::Vector2<uint32_t> packVertex(const engine::Vector3f& vertex, const VertexScale& vertexScale)
    {
        // 21 bits per vertex value = 63 bits = 8 bytes
        Vector3f memBits{ 2097151.0f, 2097151.0f, 2097151.0f };
        Vector3f scaledVert = ((vertex - vertexScale.origo) / vertexScale.range) * memBits;
        uint32_t xpart = static_cast<uint32_t>(std::max(std::min(scaledVert.x, memBits.x), 0.0f));
        uint32_t ypart = static_cast<uint32_t>(std::max(std::min(scaledVert.y, memBits.y), 0.0f));
        uint32_t zpart = static_cast<uint32_t>(std::max(std::min(scaledVert.z, memBits.z), 0.0f));
        return {
            ((xpart & 0x1fffff) << 11) | (ypart & 0x7ff),               // xpart 21 bits, ypart 11 low bits
            ((zpart & 0x1fffff) << 11) | ((ypart & 0x1ff800) >> 10) };   // zpart 21 bits, ypart 10 high bits, 1 FREE BIT
    }

    engine::Vector3f unpackVertex(const engine::Vector2<uint32_t>& vertex, const VertexScale& vertexScale)
    {
        uint32_t xpart = (vertex.x & 0xfffff800) >> 11;
        uint32_t zpart = (vertex.y & 0xfffff800) >> 11;
        uint32_t ypart = (vertex.x & 0x7ff) | ((vertex.y & 0x7fe) << 10);

        Vector3f memBits{ 2097151.0f, 2097151.0f, 2097151.0f };

        return {
            ((static_cast<float>(xpart) / memBits.x) * vertexScale.range.x) + vertexScale.origo.x,
            ((static_cast<float>(ypart) / memBits.y) * vertexScale.range.y) + vertexScale.origo.y,
            ((static_cast<float>(zpart) / memBits.z) * vertexScale.range.z) + vertexScale.origo.z
        };
    }

	engine::vector<Vector2<uint32_t>> packVertexBuffer(
		const engine::vector<Vector3f>& vertex, 
		const VertexScale& vertexScale)
	{
		engine::vector<Vector2<uint32_t>> result(vertex.size());

		// scale all the verts with that value
		for (int i = 0; i < static_cast<int>(vertex.size()); ++i)
		{
			// 21 bits per vertex value = 63 bits = 8 bytes
			auto packedVertex = packVertex(vertex[i], vertexScale);

			result[i].x = packedVertex.x;
			result[i].y = packedVertex.y;
		}

		return result;
	}

    ModelPackedCpu packModel(ModelCpu& model, VertexScale vertexScale)
    {
        ModelPackedCpu outputData;
        // pack vertex positions
        {
			outputData.vertex = packVertexBuffer(model.vertex, vertexScale);
        }

        // pack normals
        {
            outputData.normal.reserve(model.normal.size());
            for (auto&& normal : model.normal)
            {
                outputData.normal.emplace_back(packNormalOctahedron(normal));
            }
        }

        // pack tangents
        {
            outputData.tangent.reserve(model.tangent.size());
            for (auto&& tangent : model.tangent)
            {
                outputData.tangent.emplace_back(packNormalOctahedron(tangent));
            }
        }

        // pack colors
        {
            outputData.color.resize(model.color.size());
            for (int channel = 0; channel < model.color.size(); ++channel)
            {
                outputData.color[channel].resize(model.color[channel].size());
                for (int i = 0; i < model.color[channel].size(); ++i)
                {
                    outputData.color[channel][i].x = static_cast<unsigned char>(model.color[channel][i].x * 255.0f);
                    outputData.color[channel][i].y = static_cast<unsigned char>(model.color[channel][i].y * 255.0f);
                    outputData.color[channel][i].z = static_cast<unsigned char>(model.color[channel][i].z * 255.0f);
                    outputData.color[channel][i].w = static_cast<unsigned char>(model.color[channel][i].w * 255.0f);
                }
            }
        }

        outputData.uv = model.uv;
        for (auto&& index : model.index)
        {
            ASSERT(index < 65536, "Invalid index");
            outputData.index.emplace_back(static_cast<uint16_t>(index));
        }
        outputData.boundingBox = model.boundingBox;

        return outputData;
    }

}
