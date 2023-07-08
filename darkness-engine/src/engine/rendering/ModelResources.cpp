#include "engine/rendering/ModelResources.h"
#include "engine/rendering/SubMesh.h"
#include "engine/graphics/Device.h"
#include "tools/image/Image.h"
#include <algorithm>

namespace engine
{
    ModelResources::ModelResources(Device& device)
        : m_device{ device }
        , m_gpuBuffers{ device }
        //, m_residencyManager{ device }
        , m_textures{ device.createBindlessTextureSRV() }
        , m_alphaMasks{ device.createBindlessTextureSRV() }
        , m_allocationState{ engine::make_shared<ModelResourceAllocationState>() }
    {
        m_allocationState->activeInstanceCount = 0;
        m_allocationState->activeClusterCount = 0;
        m_allocationState->activeIndexCount = 0;
    }

    GpuBuffers& ModelResources::gpuBuffers()
    {
        return m_gpuBuffers;
    }

	/*ResidencyManager& ModelResources::residency()
    {
        return m_residencyManager;
    }*/

    BindlessTextureSRV ModelResources::textures()
    {
        return m_textures;
    }
    
    BindlessTextureSRV ModelResources::alphaMasks()
    {
        return m_alphaMasks;
    }

    void ModelResources::updateSubmeshTransform(ModelAllocation& model, const Matrix4f& transform)
    {
#if 1
        model.transforms.previousTransform = model.transforms.transform;
        model.transforms.transform = transform;
        model.transforms.inverseTransform = transform.inverse();

        engine::unique_ptr<TransformHistory> mat = engine::make_unique<TransformHistory>();
        (*mat).previousTransform = model.transforms.previousTransform;
        (*mat).transform     = model.transforms.transform;
        (*mat).inverseTransform = model.transforms.inverseTransform;

        auto instanceGpuIndex = model.subMeshInstance->instanceData.modelResource.gpuIndex;
        auto range = tools::ByteRange(static_cast<size_t>(instanceGpuIndex), static_cast<size_t>(instanceGpuIndex) + static_cast<size_t>(1));

        m_pendingTransform.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(mat.get()),
                reinterpret_cast<uint8_t*>(mat.get()) + sizeof(TransformHistory)),
            range });

        m_temporaryTransform.emplace_back(std::move(mat));
#endif
    }

    void ModelResources::updateSubmeshUV(ModelAllocation& model, const engine::vector<uint32_t>& uv)
    {
#if 1
        engine::unique_ptr<engine::vector<uint32_t>> uvValue = engine::make_unique<engine::vector<uint32_t>>();
        *uvValue = uv;

        auto lodBindingGpuIndex = model.subMeshInstance->lodBindingData.modelResource.gpuIndex;
        auto range = tools::ByteRange(static_cast<size_t>(lodBindingGpuIndex), static_cast<size_t>(lodBindingGpuIndex) + uv.size());

        m_pendingUV.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(uvValue.get()->data()),
                reinterpret_cast<uint8_t*>(uvValue.get()->data()) + (sizeof(uint32_t) * uv.size())),
            range });

        m_temporaryUV.emplace_back(std::move(uvValue));
#endif
    }

    void ModelResources::updateSubmeshMaterial(ModelAllocation& model, const InstanceMaterial& material)
    {
#if 1
        engine::unique_ptr<InstanceMaterial> mat = engine::make_unique<InstanceMaterial>();
        *mat = material;

        auto instanceGpuIndex = model.subMeshInstance->instanceData.modelResource.gpuIndex;
        auto range = tools::ByteRange(static_cast<size_t>(instanceGpuIndex), static_cast<size_t>(instanceGpuIndex) + static_cast<size_t>(1));

        m_pendingMaterial.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(mat.get()),
                reinterpret_cast<uint8_t*>(mat.get()) + sizeof(InstanceMaterial)),
            range });

        m_temporaryMaterials.emplace_back(std::move(mat));
#endif
    }

#if 1
    void ModelResources::updateSubmeshObjectId(ModelAllocation& /*model*/, uint32_t /*objectId*/)
    {
#if 0
        engine::unique_ptr<uint32_t> mat = engine::make_unique<uint32_t>();
        *mat = objectId;

        // materialToObjectId map uses the same locations as material
        auto range = tools::ByteRange(static_cast<size_t>(model.material), static_cast<size_t>(model.material) + static_cast<size_t>(1));

        m_pendingMaterialIdToObjectId.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(mat.get()),
                reinterpret_cast<uint8_t*>(mat.get()) + sizeof(ClusterMaterial)),
            range });

        m_temporaryMaterialIdToObjectId.emplace_back(std::move(mat));




        engine::unique_ptr<uint32_t> objmat = engine::make_unique<uint32_t>();
        *objmat = model.material;

        // materialToObjectId map uses the same locations as material
        auto ibmatrange = tools::ByteRange(static_cast<size_t>(objectId), static_cast<size_t>(objectId) + static_cast<size_t>(1));

        m_pendingObjectIdToMaterialId.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(objmat.get()),
                reinterpret_cast<uint8_t*>(objmat.get()) + sizeof(ClusterMaterial)),
            ibmatrange });

        m_temporaryObjectIdToMaterialId.emplace_back(std::move(objmat));
#endif
    }
#endif

    size_t ModelResources::addMaterial(const ResourceKey& key, image::ImageIf* image, bool srgb)
    {
        auto index = m_textures.push(m_device.createTextureSRV(key, TextureDescription()
            .name("Instance Material")
            .width(static_cast<uint32_t>(image->width()))
            .height(static_cast<uint32_t>(image->height()))
            .format(srgb ? srgbFormat(image->format()) : image->format())
            .arraySlices(static_cast<uint32_t>(image->arraySlices()))
            .mipLevels(static_cast<uint32_t>(image->mipCount()))
            .setInitialData(TextureDescription::InitialData(
                tools::ByteRange(image->data(), image->data() + image->bytes()),
                image->width(), image->width() * image->height()))));
        return static_cast<size_t>(index);
    }

    size_t ModelResources::addMaterial(const ResourceKey& /*key*/, TextureSRVOwner image)
	{
		return static_cast<size_t>(m_textures.push(image));
	}

    size_t ModelResources::addAlphaMask(const ResourceKey& /*key*/, TextureSRVOwner image)
	{
		return static_cast<size_t>(m_alphaMasks.push(image));
	}

    void ModelResources::increaseInstanceCount()
    {
        m_allocationState->activeInstanceCount++;
        ASSERT(false, "This is used only in some test and even there it's being called as hack");
    }

    engine::shared_ptr<ModelResources::ModelAllocation> ModelResources::addSubmesh(engine::shared_ptr<engine::SubMeshInstance>& subMesh)
    {
        engine::shared_ptr<ModelAllocation> resultPtr = engine::shared_ptr<ModelAllocation>(new ModelAllocation(), [this](const ModelAllocation* ptr) { this->removeSubmesh(ptr); });
        resultPtr->subMeshInstance = subMesh;
        m_allocationState->activeInstanceCount++;
        m_allocationState->activeClusterCount += subMesh->clusterCount;
        m_allocationState->activeIndexCount += subMesh->clusterCount * 256;

        /*resultPtr->transform = subMesh.transform.modelResource.gpuIndex;
        resultPtr->material = subMesh.material.modelResource.gpuIndex;
        m_allocationState->activeClusterCount += static_cast<uint32_t>(subMesh.clusterIndexCount.size());*/
        return resultPtr;
#if 0
        auto saveModelAllocator = m_modelAllocatorSave;
        auto saveIndexAllocator = m_indexAllocatorSave;
        auto saveClusterAllocator = m_clusterAllocatorSave;
        auto saveAllocationState = m_allocationState;
        engine::shared_ptr<ModelAllocation> resultPtr = engine::shared_ptr<ModelAllocation>(new ModelAllocation(), 
            [this, saveModelAllocator, saveIndexAllocator, saveClusterAllocator, saveAllocationState](const ModelAllocation* ptr)
        {
            this->removeSubmesh(*saveModelAllocator, *saveIndexAllocator, *saveClusterAllocator, *saveAllocationState, *ptr);
        });
        auto& result = *resultPtr;

        // allocate transform
        auto transform_index = m_transformAllocator.allocate(1);
        auto transform_offset = m_transformAllocator.offset(transform_index);
        result.transform = static_cast<uint32_t>(transform_offset);

        // allocate cluster vertex/normal/tangent/indice/uv data
        ASSERT(subMesh.position.size() == subMesh.normal.size(), "Curious. There are differing count of vertices and normals");
        ASSERT(subMesh.position.size() == subMesh.tangent.size(), "Curious. There are differing count of vertices and normals");

        auto index = m_modelAllocator.allocate(subMesh.position.size());
        auto offset = m_modelAllocator.offset(index);
        auto range = tools::ByteRange(offset, offset + subMesh.position.size());

        result.clusterModelIds.emplace_back(static_cast<uint32_t>(offset));

        m_pendingVertex.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(&subMesh.position[0]), 
                reinterpret_cast<uint8_t*>(&subMesh.position[0]) + (subMesh.position.size() * sizeof(Vector3f))),
            range });

        m_pendingNormal.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(&subMesh.normal[0]),
                reinterpret_cast<uint8_t*>(&subMesh.normal[0]) + (subMesh.normal.size() * sizeof(Vector3f))),
            range });

        m_pendingTangent.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(&subMesh.tangent[0]),
                reinterpret_cast<uint8_t*>(&subMesh.tangent[0]) + (subMesh.tangent.size() * sizeof(Vector3f))),
            range });

        if (subMesh.uv.size() > 0)
        {
            m_pendingUv.emplace_back(PendingUpload{
                tools::ByteRange(
                    reinterpret_cast<uint8_t*>(&subMesh.uv[0][0]),
                    reinterpret_cast<uint8_t*>(&subMesh.uv[0][0]) + (subMesh.uv[0].size() * sizeof(Vector2f))),
                range });
        }

        // allocate submesh material
        auto material_index = m_materialAllocator.allocate(1);
        auto material_offset = m_materialAllocator.offset(material_index);
        auto material_range = tools::ByteRange(material_offset, material_offset + static_cast<size_t>(1));
        result.material = static_cast<uint32_t>(material_offset);

        engine::unique_ptr<ClusterMaterial> material = engine::make_unique<ClusterMaterial>();
        material->albedo = 0;
        material->metalness = 0;
        material->roughness = 0;
        material->normal = 0;
        material->ao = 0;

        m_pendingMaterial.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(material.get()),
                reinterpret_cast<uint8_t*>(material.get()) + sizeof(ClusterMaterial)),
            material_range });

        m_temporaryMaterials.emplace_back(std::move(material));

        // setup object id
        engine::unique_ptr<uint32_t> materialIdToObjectId = engine::make_unique<uint32_t>();
        m_pendingMaterialIdToObjectId.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(materialIdToObjectId.get()),
                reinterpret_cast<uint8_t*>(materialIdToObjectId.get()) + sizeof(uint32_t)),
            material_range });

        m_temporaryMaterialIdToObjectId.emplace_back(std::move(materialIdToObjectId));

        // setup object id
        engine::unique_ptr<uint32_t> objectIdToMaterialId = engine::make_unique<uint32_t>();
        m_pendingObjectIdToMaterialId.emplace_back(PendingUpload{
            tools::ByteRange(
                reinterpret_cast<uint8_t*>(objectIdToMaterialId.get()),
                reinterpret_cast<uint8_t*>(objectIdToMaterialId.get()) + sizeof(uint32_t)),
            material_range });

        m_temporaryObjectIdToMaterialId.emplace_back(std::move(objectIdToMaterialId));




        uint32_t currentClusterIndex = 0;
        for (int i = 0; i < subMesh.clusterIndexCount.size(); ++i)
        {
            auto indexesProcessed = static_cast<uint32_t>(0);
            auto indexesToProcess = subMesh.clusterIndexCount[i];

            while (indexesProcessed < indexesToProcess)
            {
                auto indexesNow = std::min(indexesToProcess - indexesProcessed, ClusterMaxSize);
                //if (indexesNow != 192)
                //    break;

                //if (indexesNow != 192)
                //    LOG("odd index count. indexesNow: %u", indexesNow);

                // allocate cluster indexes
                auto indice_index = m_indexAllocator.allocate(ClusterMaxSize);
                auto indice_offset = m_indexAllocator.offset(indice_index);
                auto indice_range = tools::ByteRange(indice_offset, indice_offset + ClusterMaxSize);

                result.clusterIndexIds.emplace_back(static_cast<uint32_t>(indice_offset));

                engine::unique_ptr<engine::vector<uint32_t>> tempIndexes = engine::make_unique<engine::vector<uint32_t>>();

                Vector3f min{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
                Vector3f max{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

                auto currentCluster = currentClusterIndex;
                for (unsigned int ind = 0; ind < indexesNow; ++ind)
                {
                    Vector3f point = subMesh.position[subMesh.indices[currentClusterIndex]];
                    if (point.x < min.x) min.x = point.x;
                    if (point.y < min.y) min.y = point.y;
                    if (point.z < min.z) min.z = point.z;
                    if (point.x > max.x) max.x = point.x;
                    if (point.y > max.y) max.y = point.y;
                    if (point.z > max.z) max.z = point.z;

                    (*tempIndexes).emplace_back(subMesh.indices[currentClusterIndex] + static_cast<uint32_t>(offset));
                    ++currentClusterIndex;
                }
                for (unsigned int ind = 0; ind < ClusterMaxSize - indexesNow; ++ind)
                {
                    (*tempIndexes).emplace_back(static_cast<uint32_t>(-1));
                }

                auto diff = max - min;
                auto radius = diff / 2.0f;
                auto center = min + radius;
                //subMesh.boundingBox
                for (unsigned int ind = 0; ind < indexesNow; ++ind)
                {
                    Vector3f point = subMesh.position[subMesh.indices[currentCluster + ind]];
                    if ((center - point).magnitude() > radius.magnitude())
                        radius = (center - point);
                }


                //subMesh.boundingSphere = Vector4f{ center.x, center.y, center.z, radius.magnitude() };
                auto bs = engine::make_unique<BoundingSphere>(Vector4f{ center.x, center.y, center.z, radius.magnitude() });
                m_temporaryBoundingSpheres.emplace_back(std::move(bs));

                m_pendingIndice.emplace_back(PendingUpload{
                    tools::ByteRange(
                        reinterpret_cast<uint8_t*>(&(*tempIndexes)[0]),
                        reinterpret_cast<uint8_t*>(&(*tempIndexes)[0]) + (ClusterMaxSize * sizeof(uint32_t))),
                    indice_range });

                m_temporaryIndexes.emplace_back(std::move(tempIndexes));


                // allocate cluster binding data
                // NOTE: we need only one value per cluster
                auto cluster_index = m_clusterAllocator.allocate(1, 1);
                auto cluster_offset = m_clusterAllocator.offset(cluster_index);
                auto cluster_range = tools::ByteRange(cluster_offset, cluster_offset + static_cast<size_t>(1));
                
                auto bb = engine::make_unique<GpuBoundingBox>();
                bb->min = Vector4f(min, 1.0f);
                bb->max = Vector4f(max, 1.0f);
                m_temporaryBoundingBoxes.emplace_back(std::move(bb));
                m_pendingBoundingBox.emplace_back(PendingUpload{
                    tools::ByteRange(
                        reinterpret_cast<uint8_t*>(m_temporaryBoundingBoxes[m_temporaryBoundingBoxes.size() - 1].get()),
                        reinterpret_cast<uint8_t*>(m_temporaryBoundingBoxes[m_temporaryBoundingBoxes.size() - 1].get()) + sizeof(GpuBoundingBox)),
                    cluster_range });

                m_pendingBoundingSphere.emplace_back(PendingUpload{
                    tools::ByteRange(
                        reinterpret_cast<uint8_t*>(m_temporaryBoundingSpheres[m_temporaryBoundingSpheres.size()-1].get()),
                        reinterpret_cast<uint8_t*>(m_temporaryBoundingSpheres[m_temporaryBoundingSpheres.size()-1].get()) + sizeof(BoundingSphere)),
                    cluster_range });

                engine::unique_ptr<ClusterBinding> binding = engine::make_unique<ClusterBinding>();
                binding->clusterId = static_cast<uint32_t>(cluster_offset);
                binding->clusterIndexOffset = static_cast<uint32_t>(indice_offset);
                binding->indexCount = ClusterMaxSize;
                binding->transformId = static_cast<uint32_t>(transform_offset);
                binding->materialId = static_cast<uint32_t>(material_offset);

                m_pendingBinding.emplace_back(PendingUpload{
                    tools::ByteRange(
                        reinterpret_cast<uint8_t*>(binding.get()),
                        reinterpret_cast<uint8_t*>(binding.get()) + sizeof(ClusterBinding)),
                    cluster_range });

                m_temporaryBindings.emplace_back(std::move(binding));

                result.clusterIds.emplace_back(static_cast<uint32_t>(cluster_offset));

                ++m_allocationState->activeClusterCount;
                indexesProcessed += indexesNow;
            }
        }
        return resultPtr;
#endif
    }

    void ModelResources::removeSubmesh(const ModelResources::ModelAllocation* instance)
    {
        //subMesh.freeInstance(*this, &instance->subMeshInstance);
        m_allocationState->activeClusterCount -= instance->subMeshInstance->clusterCount;
        delete instance;
        m_allocationState->activeInstanceCount--;
        /*for(auto&& cluster : modelAllocation.clusterModelIds)
            modelAllocator.free(reinterpret_cast<void*>(static_cast<const uint64_t>(cluster)));

        for (auto&& cluster : modelAllocation.clusterIndexIds)
            indexAllocator.free(reinterpret_cast<void*>(static_cast<const uint64_t>(cluster)));

        for (auto&& cluster : modelAllocation.clusterIds)
        {
            clusterAllocator.free(reinterpret_cast<void*>(static_cast<const uint64_t>(cluster)));
            --allocationState.activeClusterCount;
        }*/
    }

    void ModelResources::streamResources(CommandList& cmd)
    {
#if 0
        for (auto&& upload : m_pendingVertex)
        {
            m_device.uploadBuffer(cmd, m_vertex, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingVertex.clear();

        for (auto&& upload : m_pendingNormal)
        {
            m_device.uploadBuffer(cmd, m_normal, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingNormal.clear();

        for (auto&& upload : m_pendingTangent)
        {
            m_device.uploadBuffer(cmd, m_tangent, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingTangent.clear();

        for (auto&& upload : m_pendingIndice)
        {
            m_device.uploadBuffer(cmd, m_indice, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingIndice.clear();
        m_temporaryIndexes.clear();

        for (auto&& upload : m_pendingUv)
        {
            m_device.uploadBuffer(cmd, m_uv, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingUv.clear();

        for (auto&& upload : m_pendingBoundingSphere)
        {
            m_device.uploadBuffer(cmd, m_boundingSpheres, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingBoundingSphere.clear();
        m_temporaryBoundingSpheres.clear();

        for (auto&& upload : m_pendingBoundingBox)
        {
            m_device.uploadBuffer(cmd, m_boundingBoxes, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingBoundingBox.clear();
        m_temporaryBoundingBoxes.clear();

        for (auto&& upload : m_pendingBinding)
        {
            m_device.uploadBuffer(cmd, m_bindings, upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingBinding.clear();
        m_temporaryBindings.clear();
#endif

        for (auto&& upload : m_pendingMaterial)
        {
            m_device.uploadBuffer(cmd, m_gpuBuffers.instanceMaterial(), upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingMaterial.clear();
        m_temporaryMaterials.clear();

        /*for (auto&& upload : m_pendingUV)
        {
            m_device.uploadBuffer(cmd, m_gpuBuffers.instanceUv(), upload.source, static_cast<uint32_t>(upload.destination.start));
        }*/
        m_pendingUV.clear();
        m_temporaryUV.clear();

#if 0
        for (auto&& upload : m_pendingMaterialIdToObjectId)
        {
            m_device.uploadBuffer(cmd, m_gpuBuffers.materialToObjectId(), upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingMaterialIdToObjectId.clear();
        m_temporaryMaterialIdToObjectId.clear();

        for (auto&& upload : m_pendingObjectIdToMaterialId)
        {
            m_device.uploadBuffer(cmd, m_gpuBuffers.objectIdToMaterial(), upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingObjectIdToMaterialId.clear();
        m_temporaryObjectIdToMaterialId.clear();
#endif
        for (auto&& upload : m_pendingTransform)
        {
            m_device.uploadBuffer(cmd, m_gpuBuffers.instanceTransform(), upload.source, static_cast<uint32_t>(upload.destination.start));
        }
        m_pendingTransform.clear();
        m_temporaryTransform.clear();

    }

    size_t ModelResources::instanceCount() const
    {
        return m_allocationState->activeInstanceCount;
    }

    size_t ModelResources::clusterCount() const
    {
        return m_allocationState->activeClusterCount;
    }

    size_t ModelResources::indexCount() const
    {
        return m_allocationState->activeIndexCount;
    }

    template<typename T>
    size_t sizeMb(T& allocator)
    {
        return allocator.elements() * allocator.elementSizeBytes() / 1024 / 1024;
    };

    template<typename T>
    size_t usedMb(T& allocator)
    {
        return allocator.usedElements() * allocator.elementSizeBytes() / 1024 / 1024;
    };

    void ModelResources::printBufferStatus()
    {
        ModelResourceAllocator& vertexDataAllocator = m_gpuBuffers.vertexDataAllocator();
        ModelResourceAllocator& uvDataAllocator = m_gpuBuffers.uvDataAllocator();
        ModelResourceAllocator& indexAllocator = m_gpuBuffers.indexAllocator();
        ModelResourceAllocator& adjacencyAllocator = m_gpuBuffers.adjacencyAllocator();
        ModelResourceAllocator& clusterDataAllocator = m_gpuBuffers.clusterDataAllocator();
        ModelResourceAllocator& subMeshDataAllocator = m_gpuBuffers.subMeshDataAllocator();
        ModelResourceAllocator& lodAllocator = m_gpuBuffers.lodAllocator();
		ModelResourceAllocator& instanceDataAllocator = m_gpuBuffers.instanceDataAllocator();

        #define LogAllocator(allocator) LOG(#allocator"[%zu mb]: Used: %zu mb", sizeMb(allocator), usedMb(allocator))
        LogAllocator(vertexDataAllocator);
        LogAllocator(uvDataAllocator);
        LogAllocator(indexAllocator);
        LogAllocator(adjacencyAllocator);
        LogAllocator(clusterDataAllocator);
        LogAllocator(subMeshDataAllocator);
        LogAllocator(lodAllocator);
        LogAllocator(instanceDataAllocator);
    }
}

