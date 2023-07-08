#pragma once

#include "engine/graphics/ResourceOwners.h"
#include "engine/rendering/ModelResourceAllocator.h"

namespace engine
{
    constexpr uint32_t InvalidMaxInstanceIndex = 0xFFFFFFFF;

    class SubMeshInstance;
    class Device;
    class GpuBuffers
    {
    public:
        GpuBuffers(Device& device);

        // per vertex data
        enum class VertexDataIndex
        {
            Vertex,
            Normal,
            Tangent,
            Color
        };

        enum class ClusterDataIndex
        {
            Binding,
            BoundingBox,
            Cone
        };

        enum class SubMeshDataIndex
        {
            SubMeshData,
            BoundingBox,
            BoundingSphere,
            Adjacency
        };

        enum class InstanceDataIndex
        {
            ClusterTracking,
            Transform,
            LodBinding,
            Material,
            VertexScaling
        };

        // per vertex data
        ModelResourceAllocator& vertexDataAllocator();
        BufferSRV vertex();
        BufferSRV normal();
        BufferSRV tangent();
        BufferSRV color();

        // uv data
        ModelResourceAllocator& uvDataAllocator();
        BufferSRV uv();
        
        // per triangle data
        ModelResourceAllocator& indexAllocator();
		BufferSRV index();
		BufferIBV indexIBV();

        // adjacency data
        ModelResourceAllocator& adjacencyAllocator();
		BufferIBV adjacency();

        // per cluster data
        ModelResourceAllocator& clusterDataAllocator();
        BufferSRV clusterBinding();
        BufferSRV clusterBoundingBox();
        BufferSRV clusterCone();
		BufferUAV clusterBoundingBoxUAV();
        
        // per submesh data
        ModelResourceAllocator& subMeshDataAllocator();
        BufferSRV subMeshData();
        BufferSRV subMeshBoundingBox();
        BufferSRV subMeshBoundingSphere();
        BufferSRV subMeshAdjacency();
		BufferUAV subMeshBoundingBoxUAV();

        // lod binding data
        ModelResourceAllocator& lodAllocator();
		BufferSRV lod();

        // per instance data
		ModelResourceAllocator& instanceDataAllocator();
        BufferSRV instanceTransform();
		BufferSRVOwner& instanceTransformOwner();
        BufferSRV instanceLodBinding();
        BufferSRV instanceMaterial();
        BufferSRV instanceScale();
        BufferSRV instanceClusterTracking();
        BufferUAV instanceTransformUAV();
        BufferUAV instanceLodBindingUAV();
        BufferUAV instanceMaterialUAV();
        BufferUAV instanceScaleUAV();
        BufferUAV instanceClusterTrackingUAV();

        // cluster tracking
        BufferUAVOwner clusterTrackingClusterInstancesUAV;
        BufferSRVOwner clusterTrackingClusterInstancesSRV;
        ModelResourceAllocation allocateClusterTracking(size_t clusterCount);

        void maxInstanceIndexInUse(size_t value);
        size_t maxInstanceIndexInUse() const;

        void addInstance(SubMeshInstance* instance);
        void removeInstance(SubMeshInstance* instance);
        void relocateInstance(size_t oldIndex, size_t newIndex);
    private:
        Device& m_device;
        // model storage
        ModelResourceAllocator m_vertexDataAllocator;
        ModelResourceAllocator m_uvDataAllocator;
        ModelResourceAllocator m_indexAllocator;
        ModelResourceAllocator m_adjacencyAllocator;
        ModelResourceAllocator m_clusterDataAllocator;
        ModelResourceAllocator m_subMeshDataAllocator;
        ModelResourceAllocator m_lodAllocator;
		ModelResourceAllocator m_instanceDataAllocator;
        tools::MemoryAllocator m_clusterTrackingClusterInstancesAllocator;
        size_t m_trackedClusterCount;
        size_t m_maxInstanceIndexInUse;
        engine::vector<SubMeshInstance*> m_trackedInstances;
    };
}
