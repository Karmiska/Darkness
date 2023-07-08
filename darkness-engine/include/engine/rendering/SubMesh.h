#pragma once

#include "engine/primitives/BoundingBox.h"
#include "engine/primitives/BoundingSphere.h"
#include "shaders/core/shared_types/VertexScale.hlsli"
#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"
#include "engine/rendering/Material.h"
#include "engine/rendering/ModelCpu.h"
#include "tools/CompressedFile.h"
#include "engine/rendering/ResidencyManager.h"
#include "containers/vector.h"
#include "containers/string.h"

namespace engine
{
    class ModelResources;
	class ResidencyManagerV2;

    enum class MeshBlockType
    {
        Position,
        Normal,
        Tangent,
        Uv,
        Indice,
        Color,
        Material,
        ClusterVertexStart,
        ClusterIndexStart,
        ClusterIndexCount,
        ClusterBounds,
        ClusterCones,
        BoundingBox,
        AdjacencyData,
        VertexScale
    };

    struct MeshBlockHeader
    {
        MeshBlockType type;
        size_t size_bytes;
    };
    
    class SubMeshInstance
    {
    public:
        ModelResource instanceData;
        ModelResource lodBindingData;
        engine::vector<const ModelResource*> uvData;
        uint32_t clusterCount;
    };

    class SubMesh
    {
        typedef int Count;
    public:
        struct GpuData
        {
            ModelResource vertexData;
            engine::vector<ModelResource> uvData;
            ModelResource triangleData;
            ModelResource adjacencyData;
            ModelResource clusterData;
            ModelResource subMeshData;
        };
        engine::vector<GpuData> gpuData;

        ModelResource lodData;

        engine::vector<ModelPackedCpu> outputData;

        BoundingBox boundingBox;
        BoundingSphere boundingSphere;
        Material out_material;
        VertexScale meshScale;

        //size_t sizeBytes() const;
        void save(CompressedFile& file) const;
        bool load(ResidencyManagerV2& residency, ModelResources& modelResources, CompressedFile& file);

        engine::shared_ptr<SubMeshInstance> createInstance(Device& device);
        void freeInstance(Device& device, SubMeshInstance* instance);

		static engine::shared_ptr<SubMeshInstance> createInstance(
            Device& device,
			const engine::vector<ModelResource>& uvData, 
            size_t& instanceCount,
            size_t& clusterCount,
			ModelResource& lodData, 
			size_t lodCount,
			VertexScale vertexScale);
		static void freeInstance(
            Device& device,
            SubMeshInstance* instance, 
            size_t& instanceCount);

        size_t instanceCount() const;
    private:
        void writeBlockHeader(CompressedFile& file, MeshBlockHeader header) const;
        MeshBlockHeader readBlockHeader(CompressedFile& file);

        size_t m_instanceCount = 0;
        size_t m_clusterCount = 0;
        
        Count elementCount() const;

        template <typename T>
        size_t blockSize(const engine::vector<T>& data) const
        {
            if(data.size() > 0)
                return sizeof(MeshBlockHeader) + sizeof(Count) + (sizeof(T) * data.size());
            else
                return 0;
        }

        template <typename T>
        void writeBlock(CompressedFile& file, MeshBlockType type, const engine::vector<T>& data) const
        {
            if (data.size() > 0)
            {
                writeBlockHeader(file, { type, static_cast<size_t>(sizeof(Count) + (sizeof(T) * data.size())) });
                Count count = static_cast<Count>(data.size());
                file.write(reinterpret_cast<char*>(&count), sizeof(Count));
                file.write(reinterpret_cast<const char*>(&data[0]), static_cast<std::streamsize>(sizeof(T) * count));
            }
        }

        template <typename T>
        void writeBlock(CompressedFile& file, MeshBlockType type, const T& data) const
        {
            writeBlockHeader(file, { type, sizeof(T) });
            file.write(reinterpret_cast<const char*>(&data), static_cast<std::streamsize>(sizeof(T)));
        }

        void allocateIfNecessary(ModelResourceAllocator& resourceAllocator, ModelResource& data, Count count)
        {
            if (!data.allocated)
            {
                data.modelResource = resourceAllocator.allocate(count);
                data.allocated = true;
            }
        };

        template <typename T>
        void readBlock(
            ResidencyManager& residency, 
            ModelResourceAllocator& resourceAllocator, 
            CompressedFile& file, 
            ModelResource& data)
        {
            Count count;
            file.read(reinterpret_cast<char*>(&count), sizeof(Count));
            allocateIfNecessary(resourceAllocator, data, count);
            data.uploads.emplace_back(residency.createUpdateAllocation(count * sizeof(T)));
            data.uploads.back().gpuIndex = data.modelResource.gpuIndex;
            file.read(reinterpret_cast<char*>(data.uploads.back().ptr), static_cast<std::streamsize>(sizeof(T) * count));
        }

        template <typename T>
        void readBlock(CompressedFile& file, engine::vector<T>& data)
        {
            Count count;
            file.read(reinterpret_cast<char*>(&count), sizeof(Count));
            data.resize(static_cast<typename engine::vector<T>::size_type>(count));
            file.read(reinterpret_cast<char*>(&data[0]), static_cast<std::streamsize>(sizeof(T) * count));
        }

        template <typename T>
        void readBlock(CompressedFile& file, T& data)
        {
            file.read(reinterpret_cast<char*>(&data), static_cast<std::streamsize>(sizeof(T)));
        }
    };

}
