#pragma once

#include "engine/primitives/BoundingSphere.h"
#include "engine/graphics/Resources.h"
#include "engine/rendering/SubMesh.h"
#include "engine/rendering/ModelResourceAllocator.h"
#include "engine/rendering/GpuBuffers.h"
#include "engine/rendering/ResidencyManager.h"
#include "tools/ByteRange.h"
#include "tools/MemoryAllocator.h"

#include "engine/graphics/HLSLTypeConversions.h"

namespace engine
{
    namespace image
    {
        class ImageIf;
    }
    class Device;
    class CommandList;
    class SubMesh;
	class TextureSRVOwner;
    
    #include "shaders/core/shared_types/ClusterData.hlsli"
    #include "shaders/core/shared_types/SubMeshData.hlsli"
    #include "shaders/core/shared_types/InstanceMaterial.hlsli"
    #include "shaders/core/shared_types/TransformHistory.hlsli"
    #include "shaders/core/shared_types/SubMeshAdjacency.hlsli"
    #include "shaders/core/shared_types/LodBinding.hlsli"
    #include "shaders/core/shared_types/SubMeshUVLod.hlsli"
    #include "shaders/core/shared_types/VertexScale.hlsli"

    struct ModelResourceAllocationState
    {
        size_t activeInstanceCount;
        size_t activeClusterCount;
        size_t activeIndexCount;
    };

    class ModelResources
    {
    public:
        ModelResources(Device& device);

        //BufferSRV& clusterData();
        //TextureBindlessSRV& textureData();

        struct ModelAllocation
        {
            engine::shared_ptr<engine::SubMeshInstance> subMeshInstance;

            TransformHistory transforms;
        };

        engine::shared_ptr<ModelAllocation> addSubmesh(engine::shared_ptr<engine::SubMeshInstance>& subMesh);

        size_t addMaterial(const ResourceKey& key, image::ImageIf* image, bool srgb = false);
        size_t addMaterial(const ResourceKey& key, TextureSRVOwner image);
        size_t addAlphaMask(const ResourceKey& key, TextureSRVOwner image);

        void updateSubmeshTransform(ModelAllocation& model, const Matrix4f& transform);
        void updateSubmeshMaterial(ModelAllocation& model, const InstanceMaterial& material);
        void updateSubmeshObjectId(ModelAllocation& model, uint32_t objectId);
        void updateSubmeshUV(ModelAllocation& model, const engine::vector<uint32_t>& uv);

        void streamResources(CommandList& cmd);

        size_t instanceCount() const;
        size_t clusterCount() const;
        size_t indexCount() const;
        void increaseInstanceCount();

        GpuBuffers& gpuBuffers();
        //ResidencyManager& residency();
        BindlessTextureSRV textures();
        BindlessTextureSRV alphaMasks();

        void printBufferStatus();

    private:
        Device& m_device;
        GpuBuffers m_gpuBuffers;
        //ResidencyManager m_residencyManager;

        // texture storage
        BindlessTextureSRVOwner m_textures;
        BindlessTextureSRVOwner m_alphaMasks;

        void removeSubmesh(const ModelResources::ModelAllocation* instance);

        // pending uploads
        struct PendingUpload
        {
            tools::ByteRange source;
            tools::ByteRange destination;
        };
        /*engine::vector<PendingUpload> m_pendingVertex;
        engine::vector<PendingUpload> m_pendingNormal;
        engine::vector<PendingUpload> m_pendingTangent;
        engine::vector<PendingUpload> m_pendingIndice;
        engine::vector<PendingUpload> m_pendingUv;

        engine::vector<PendingUpload> m_pendingBoundingSphere;
        engine::vector<PendingUpload> m_pendingBoundingBox;
        engine::vector<PendingUpload> m_pendingBinding;*/
        engine::vector<PendingUpload> m_pendingMaterial;
        engine::vector<PendingUpload> m_pendingMaterialIdToObjectId;
        engine::vector<PendingUpload> m_pendingObjectIdToMaterialId;
        engine::vector<PendingUpload> m_pendingUV;

        engine::vector<PendingUpload> m_pendingTransform;

        /*engine::vector<engine::unique_ptr<engine::vector<uint32_t>>> m_temporaryIndexes;
        engine::vector<engine::unique_ptr<ClusterBinding>> m_temporaryBindings;*/
        engine::vector<engine::unique_ptr<InstanceMaterial>> m_temporaryMaterials;
        engine::vector<engine::unique_ptr<TransformHistory>> m_temporaryTransform;
        //engine::vector<engine::unique_ptr<BoundingSphere>> m_temporaryBoundingSpheres;
        engine::vector<engine::unique_ptr<uint32_t>> m_temporaryMaterialIdToObjectId;
        engine::vector<engine::unique_ptr<uint32_t>> m_temporaryObjectIdToMaterialId;
        engine::vector<engine::unique_ptr<engine::vector<uint32_t>>> m_temporaryUV;

        /*engine::vector<engine::unique_ptr<GpuBoundingBox>> m_temporaryBoundingBoxes;*/
        
        // 
        engine::shared_ptr<ModelResourceAllocationState> m_allocationState;
        
    };
}
