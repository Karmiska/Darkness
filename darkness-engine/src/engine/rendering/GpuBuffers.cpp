#include "engine/rendering/GpuBuffers.h"
#include "engine/rendering/ModelResources.h"
#include "engine/rendering/SubMesh.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include <algorithm>
#include "engine/rendering/BufferSettings.h"

namespace engine
{
    GpuBuffers::GpuBuffers(Device& device)
        // per vertex data
        : m_device{ device }
        , clusterTrackingClusterInstancesUAV{}
        , clusterTrackingClusterInstancesSRV{}
        , m_vertexDataAllocator{ 
            device, 
            MaxVertexCount,
            {
                Format::R32G32_UINT,    // vertex
                Format::R32G32_FLOAT,   // normal
                Format::R32G32_FLOAT,   // tangent
                Format::R8G8B8A8_UNORM  // color
            },
            "Vertex data", {} }

        // uv data
        , m_uvDataAllocator{ device, MaxUvCount,{ Format::R32G32_FLOAT }, "UV data", {} }

        // per triangle data
        , m_indexAllocator{ device, MaxIndexCount, { Format::R16_UINT }, "Triangle data", { { ExtraViewType::Index, 0, 1 } } }

        // per triangle data
        , m_adjacencyAllocator{ device, MaxIndexCount*2,{ Format::R32_UINT }, "Adjacency data", { { ExtraViewType::Index, 0, 1 } } }

        // per cluster data
        , m_clusterDataAllocator{ 
            device, 
            MaxClusterCount,
            {
                sizeof(ClusterData),    // 16 bytes
                sizeof(BoundingBox),    // 24 bytes
                sizeof(Vector4f)        // 16 bytes
            },
            "Cluster data" }

        // per submesh data
        , m_subMeshDataAllocator{
            device, 
            MaxSubModels,
            {
                sizeof(SubMeshData),    // 16 bytes
                sizeof(BoundingBox),    // 24 bytes
                sizeof(BoundingSphere),  // 16 bytes
                sizeof(SubMeshAdjacency)
            },
            "SubMesh data" }

        // lod binding
        , m_lodAllocator{
            device, 
            MaxLods, 
            {},
            {
                sizeof(SubMeshUVLod)    // 16 bytes
            },
            "Lod binding data" }

        // per instance data
        , m_instanceDataAllocator{
            device, 
            MaxInstances,
            {
                engine::Format::R32_UINT
            },
            {
                sizeof(TransformHistory),  // 192 bytes
                sizeof(LodBinding),                                     // 16 bytes
                sizeof(InstanceMaterial),                               // 64 bytes
                sizeof(VertexScale)
            },
            "Instance data" }
        , m_clusterTrackingClusterInstancesAllocator{ tools::ByteRange{
            static_cast<unsigned long long>(0u),
            static_cast<unsigned long long>(0u) }, 1u }
        , m_maxInstanceIndexInUse{ InvalidMaxInstanceIndex }
        , m_trackedClusterCount{ 0 }
    {}

	ModelResourceAllocator& GpuBuffers::vertexDataAllocator() { return m_vertexDataAllocator; };
	BufferSRV GpuBuffers::vertex()
	{
		return m_vertexDataAllocator.gpuBuffers()[static_cast<int>(VertexDataIndex::Vertex)];
	};
	BufferSRV GpuBuffers::normal() { return m_vertexDataAllocator.gpuBuffers()[static_cast<int>(VertexDataIndex::Normal)]; };
	BufferSRV GpuBuffers::tangent() { return m_vertexDataAllocator.gpuBuffers()[static_cast<int>(VertexDataIndex::Tangent)]; };
	BufferSRV GpuBuffers::color() { return m_vertexDataAllocator.gpuBuffers()[static_cast<int>(VertexDataIndex::Color)]; };

	// uv data
	ModelResourceAllocator& GpuBuffers::uvDataAllocator() { return m_uvDataAllocator; };
    BufferSRV GpuBuffers::uv() { return m_uvDataAllocator.gpuBuffers().size() > 0 ? m_uvDataAllocator.gpuBuffers()[0] : BufferSRV{}; };

	// per triangle data
	ModelResourceAllocator& GpuBuffers::indexAllocator() { return m_indexAllocator; };
	BufferSRV GpuBuffers::index()
    { 
        return m_indexAllocator.gpuBuffers().size() > 0 ? m_indexAllocator.gpuBuffers()[0] : BufferSRV{};
    }
	BufferIBV GpuBuffers::indexIBV()
    {
        return m_indexAllocator.gpuIndexBuffers().size() > 0 ? m_indexAllocator.gpuIndexBuffers()[0] : BufferIBV{};
    }

	// adjacency data
	ModelResourceAllocator& GpuBuffers::adjacencyAllocator() { return m_adjacencyAllocator; };
	BufferIBV GpuBuffers::adjacency() { return m_adjacencyAllocator.gpuIndexBuffers()[0]; }

	// per cluster data
	ModelResourceAllocator& GpuBuffers::clusterDataAllocator() { return m_clusterDataAllocator; };
	BufferSRV GpuBuffers::clusterBinding()
    {
        return static_cast<int>(ClusterDataIndex::Binding) < m_clusterDataAllocator.gpuBuffers().size() ?
            m_clusterDataAllocator.gpuBuffers()[static_cast<int>(ClusterDataIndex::Binding)] : BufferSRV{};
    };
	BufferSRV GpuBuffers::clusterBoundingBox()
    {
        return static_cast<int>(ClusterDataIndex::BoundingBox) < m_clusterDataAllocator.gpuBuffers().size() ?
            m_clusterDataAllocator.gpuBuffers()[static_cast<int>(ClusterDataIndex::BoundingBox)] : BufferSRV{};
    };
	BufferSRV GpuBuffers::clusterCone() { return m_clusterDataAllocator.gpuBuffers()[static_cast<int>(ClusterDataIndex::Cone)]; };
	BufferUAV GpuBuffers::clusterBoundingBoxUAV() { return m_clusterDataAllocator.gpuBuffersUAV()[static_cast<int>(ClusterDataIndex::BoundingBox)]; };

	// per submesh data
	ModelResourceAllocator& GpuBuffers::subMeshDataAllocator() { return m_subMeshDataAllocator; };
	BufferSRV GpuBuffers::subMeshData() { return m_subMeshDataAllocator.gpuBuffers()[static_cast<int>(SubMeshDataIndex::SubMeshData)]; };
	BufferSRV GpuBuffers::subMeshBoundingBox() { return m_subMeshDataAllocator.gpuBuffers()[static_cast<int>(SubMeshDataIndex::BoundingBox)]; };
	BufferSRV GpuBuffers::subMeshBoundingSphere() { return m_subMeshDataAllocator.gpuBuffers()[static_cast<int>(SubMeshDataIndex::BoundingSphere)]; };
	BufferSRV GpuBuffers::subMeshAdjacency() { return m_subMeshDataAllocator.gpuBuffers()[static_cast<int>(SubMeshDataIndex::Adjacency)]; };
	BufferUAV GpuBuffers::subMeshBoundingBoxUAV() { return m_subMeshDataAllocator.gpuBuffersUAV()[static_cast<int>(SubMeshDataIndex::BoundingBox)]; };

	// lod binding data
	ModelResourceAllocator& GpuBuffers::lodAllocator() { return m_lodAllocator; };
	BufferSRV GpuBuffers::lod() { return m_lodAllocator.gpuBuffers()[0]; }

	// per instance data
	ModelResourceAllocator& GpuBuffers::instanceDataAllocator() { return m_instanceDataAllocator; };
	BufferSRV GpuBuffers::instanceTransform() { return m_instanceDataAllocator.gpuBuffers()[static_cast<int>(InstanceDataIndex::Transform)]; };
	BufferSRVOwner& GpuBuffers::instanceTransformOwner() { return m_instanceDataAllocator.gpuBufferOwners()[static_cast<int>(InstanceDataIndex::Transform)]; };
	BufferSRV GpuBuffers::instanceLodBinding() { return m_instanceDataAllocator.gpuBuffers()[static_cast<int>(InstanceDataIndex::LodBinding)]; };
	BufferSRV GpuBuffers::instanceMaterial() { return m_instanceDataAllocator.gpuBuffers()[static_cast<int>(InstanceDataIndex::Material)]; };
	BufferSRV GpuBuffers::instanceScale() { return m_instanceDataAllocator.gpuBuffers()[static_cast<int>(InstanceDataIndex::VertexScaling)]; };
    BufferSRV GpuBuffers::instanceClusterTracking() { return m_instanceDataAllocator.gpuBuffers()[static_cast<int>(InstanceDataIndex::ClusterTracking)]; };

    BufferUAV GpuBuffers::instanceTransformUAV() { return m_instanceDataAllocator.gpuBuffersUAV()[static_cast<int>(InstanceDataIndex::Transform)]; };
    BufferUAV GpuBuffers::instanceLodBindingUAV() { return m_instanceDataAllocator.gpuBuffersUAV()[static_cast<int>(InstanceDataIndex::LodBinding)]; };
    BufferUAV GpuBuffers::instanceMaterialUAV() { return m_instanceDataAllocator.gpuBuffersUAV()[static_cast<int>(InstanceDataIndex::Material)]; };
    BufferUAV GpuBuffers::instanceScaleUAV() { return m_instanceDataAllocator.gpuBuffersUAV()[static_cast<int>(InstanceDataIndex::VertexScaling)]; };
    BufferUAV GpuBuffers::instanceClusterTrackingUAV() { return m_instanceDataAllocator.gpuBuffersUAV()[static_cast<int>(InstanceDataIndex::ClusterTracking)]; };

    ModelResourceAllocation GpuBuffers::allocateClusterTracking(size_t clusterCount)
    {
        auto allocateGpuResource = [](
            Device& device,
            tools::MemoryAllocator& allocator,
            BufferUAVOwner& uav,
            BufferSRVOwner& srv,
            size_t clusterCount,
            bool bitAccess)->void*
        {
            auto clusterAllocation = allocator.allocate(clusterCount);
            if (clusterAllocation == reinterpret_cast<void*>(tools::MemoryAllocatorInvalidPtr))
            {
                auto currentElements = uav ? uav.resource().desc().elements : 0;
                size_t currentSize = currentElements;
                if (bitAccess)
                    currentSize *= 32;

                auto newSize = tools::gpuAllocationStrategy(sizeof(uint32_t), currentSize, clusterCount);
                auto newElements = std::max(roundUpToMultiple(newSize, 32ull) / 32ull, 1ull);

                if (newElements != currentElements)
                {
                    auto uavTemp = device.createBufferUAV(BufferDescription()
                        .usage(ResourceUsage::GpuRead)
                        .format(Format::R32_UINT)
                        .name("clusterTrackingClusterInstances")
                        .elements(newElements)
                        .elementSize(formatBytes(Format::R32_UINT)));
                    auto srvTemp = device.createBufferSRV(uavTemp);

                    if(currentElements > 0)
                    {
                        auto cmd = device.createCommandList("Resize cluster tracking command list");
                        cmd.copyBuffer(
                            srv.resource().buffer(),
                            uavTemp.resource().buffer(),
                            currentElements);
                        device.submitBlocking(cmd);
                    }

                    uav = uavTemp;
                    srv = srvTemp;

                    allocator.resize(tools::ByteRange{
                        static_cast<unsigned long long>(0ull),
                        static_cast<unsigned long long>(newSize) });
                }

                clusterAllocation = allocator.allocate(clusterCount);
                ASSERT(clusterAllocation != reinterpret_cast<void*>(tools::MemoryAllocatorInvalidPtr), "Failed to allocate cluster tracking");
            }
            return clusterAllocation;
        };

        auto clusterAllocation = allocateGpuResource(
            m_device,
            m_clusterTrackingClusterInstancesAllocator,
            clusterTrackingClusterInstancesUAV,
            clusterTrackingClusterInstancesSRV,
            clusterCount,
            true);

        ModelResourceAllocation res;
        res.ptr = clusterAllocation;
        res.gpuIndex = m_clusterTrackingClusterInstancesAllocator.offset(res.ptr);
        res.elements = clusterCount;
        res.range = tools::ByteRange(res.gpuIndex, res.gpuIndex + clusterCount);

        return res;
    }

    void GpuBuffers::maxInstanceIndexInUse(size_t value)
    {
        m_maxInstanceIndexInUse = value;
    }

    size_t GpuBuffers::maxInstanceIndexInUse() const
    {
        return m_maxInstanceIndexInUse;
    }

    void GpuBuffers::addInstance(SubMeshInstance* instance)
    {
        m_trackedInstances.emplace_back(instance);
    }

    void GpuBuffers::removeInstance(SubMeshInstance* instance)
    {
        if (m_trackedInstances.back() == instance)
        {
            m_trackedInstances.pop_back();
            return;
        }
        auto i = std::find(m_trackedInstances.begin(), m_trackedInstances.end(), instance);
        if (i != m_trackedInstances.end())
        {
            m_trackedInstances.erase(i);
            //LOG("Searched and Removed instance: %p", instance);
        }
        else
        {
            LOG("Tried to remove instance but failed: %p", instance);
        }
    }

    void GpuBuffers::relocateInstance(size_t oldIndex, size_t newIndex)
    {
        std::iter_swap(m_trackedInstances.begin() + oldIndex, m_trackedInstances.begin() + newIndex);
        m_trackedInstances[newIndex]->instanceData.modelResource = m_trackedInstances[oldIndex]->instanceData.modelResource;
    }
}
