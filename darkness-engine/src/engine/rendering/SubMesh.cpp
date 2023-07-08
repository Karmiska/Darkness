#include "engine/rendering/SubMesh.h"
#include "engine/rendering/ModelResources.h"
#include "engine/primitives/Vector3.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Queue.h"
#include "tools/MeshTools.h"
#include "tools/Debug.h"

#include <inttypes.h>

using namespace engine;
#define USE_RESIDENCY_V2

namespace engine
{

    /*size_t SubMesh::sizeBytes() const
    {
        return blockSize<Vector3f>(position);
    }*/

    void SubMesh::writeBlockHeader(CompressedFile& file, MeshBlockHeader header) const
    {
        file.write(reinterpret_cast<char*>(&header), sizeof(MeshBlockHeader));
    }

    MeshBlockHeader SubMesh::readBlockHeader(CompressedFile& file)
    {
        MeshBlockHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(MeshBlockHeader));
        return header;
    }

    bool SubMesh::load(ResidencyManagerV2& residency, ModelResources& modelResources, CompressedFile& file)
    {
        //change MESH code to load submeshes on request (not all of them by default)
        //separate uploading to gpu (residency) to a different system
        Count elements;
        file.read(reinterpret_cast<char*>(&elements), sizeof(Count));

        if (file.eof())
            return false;

        uint32_t currentLod = 0;
        while (elements)
        {
            
            auto blockHeader = readBlockHeader(file);
            switch (blockHeader.type)
            {
                case MeshBlockType::Position: 
                {
                    currentLod = static_cast<uint32_t>(gpuData.size());
                    gpuData.resize(gpuData.size() + 1);

#ifdef USE_RESIDENCY_V2
                    auto& data = gpuData[currentLod].vertexData;
					Count count;
					file.read(reinterpret_cast<char*>(&count), sizeof(Count));
					allocateIfNecessary(modelResources.gpuBuffers().vertexDataAllocator(), data, count);
					engine::vector<char> uploaddata(count * sizeof(Vector2<uint32_t>));
					file.read(uploaddata.data(), static_cast<std::streamsize>(sizeof(Vector2<uint32_t>) * count));
					auto uploadFuture = residency.uploadTemp(
						modelResources.gpuBuffers().vertex().buffer(),
						data.modelResource.gpuIndex * sizeof(Vector2<uint32_t>),
						&uploaddata[0], uploaddata.size());
					uploadFuture.blockUntilUploaded();
#else
                    readBlock<Vector2<uint32_t>>(modelResources.residency(), modelResources.gpuBuffers().vertexDataAllocator(), file, gpuData[currentLod].vertexData);
                    gpuData[currentLod].vertexData.uploads.back().buffer = modelResources.gpuBuffers().vertex();
                    modelResources.residency().makeResident(gpuData[currentLod].vertexData.uploads.back());
                    modelResources.residency().freeUpdateAllocation(gpuData[currentLod].vertexData.uploads.back());
                    gpuData[currentLod].vertexData.uploads.pop_back();
#endif
                    break;
                }
                case MeshBlockType::Normal:
                {
#ifdef USE_RESIDENCY_V2
                    auto& data = gpuData[currentLod].vertexData;
                    Count count;
                    file.read(reinterpret_cast<char*>(&count), sizeof(Count));
                    allocateIfNecessary(modelResources.gpuBuffers().vertexDataAllocator(), data, count);
                    engine::vector<char> uploaddata(count * sizeof(Vector2f));
                    file.read(uploaddata.data(), static_cast<std::streamsize>(sizeof(Vector2f) * count));
                    auto uploadFuture = residency.uploadTemp(
                        modelResources.gpuBuffers().normal().buffer(),
                        data.modelResource.gpuIndex * sizeof(Vector2f),
                        &uploaddata[0], uploaddata.size());
                    uploadFuture.blockUntilUploaded();
#else
                    readBlock<Vector2f>(modelResources.residency(), modelResources.gpuBuffers().vertexDataAllocator(), file, gpuData[currentLod].vertexData);
                    gpuData[currentLod].vertexData.uploads.back().buffer = modelResources.gpuBuffers().normal();
                    modelResources.residency().makeResident(gpuData[currentLod].vertexData.uploads.back());
                    modelResources.residency().freeUpdateAllocation(gpuData[currentLod].vertexData.uploads.back());
                    gpuData[currentLod].vertexData.uploads.pop_back();

#endif

                    // we don't clear the uploads because of adjacency calculation
                    break;
                }
                case MeshBlockType::Tangent: 
                {
#ifdef USE_RESIDENCY_V2
                    auto& data = gpuData[currentLod].vertexData;
                    Count count;
                    file.read(reinterpret_cast<char*>(&count), sizeof(Count));
                    allocateIfNecessary(modelResources.gpuBuffers().vertexDataAllocator(), data, count);
                    engine::vector<char> uploaddata(count * sizeof(Vector2f));
                    file.read(uploaddata.data(), static_cast<std::streamsize>(sizeof(Vector2f) * count));
                    auto uploadFuture = residency.uploadTemp(
                        modelResources.gpuBuffers().tangent().buffer(),
                        data.modelResource.gpuIndex * sizeof(Vector2f),
                        &uploaddata[0], uploaddata.size());
                    uploadFuture.blockUntilUploaded();
#else
                    readBlock<Vector2f>(modelResources.residency(), modelResources.gpuBuffers().vertexDataAllocator(), file, gpuData[currentLod].vertexData);
                    gpuData[currentLod].vertexData.uploads.back().buffer = modelResources.gpuBuffers().tangent();
                    modelResources.residency().makeResident(gpuData[currentLod].vertexData.uploads.back());
                    modelResources.residency().freeUpdateAllocation(gpuData[currentLod].vertexData.uploads.back());
                    gpuData[currentLod].vertexData.uploads.pop_back();
#endif
                    break;
                }
                case MeshBlockType::Uv: 
                {
#ifdef USE_RESIDENCY_V2
                    ModelResource data;
                    Count count;
                    file.read(reinterpret_cast<char*>(&count), sizeof(Count));
                    allocateIfNecessary(modelResources.gpuBuffers().uvDataAllocator(), data, count);
                    engine::vector<char> uploaddata(count * sizeof(Vector2f));
                    file.read(uploaddata.data(), static_cast<std::streamsize>(sizeof(Vector2f) * count));
                    auto uploadFuture = residency.uploadTemp(
                        modelResources.gpuBuffers().uv().buffer(),
                        data.modelResource.gpuIndex * sizeof(Vector2f),
                        &uploaddata[0], uploaddata.size());
                    uploadFuture.blockUntilUploaded();
                    gpuData[currentLod].uvData.emplace_back(std::move(data));
#else
                    ModelResource u;
                    readBlock<Vector2f>(modelResources.residency(), modelResources.gpuBuffers().uvDataAllocator(), file, u);
                    u.uploads.back().buffer = modelResources.gpuBuffers().uv();
                    modelResources.residency().makeResident(u.uploads.back());
                    modelResources.residency().freeUpdateAllocation(u.uploads.back());
                    u.uploads.pop_back();
                    gpuData[currentLod].uvData.emplace_back(std::move(u));
#endif
                    break;
                }
                case MeshBlockType::Indice: 
                {
#ifdef USE_RESIDENCY_V2
                    auto& data = gpuData[currentLod].triangleData;
                    Count count;
                    file.read(reinterpret_cast<char*>(&count), sizeof(Count));
                    allocateIfNecessary(modelResources.gpuBuffers().indexAllocator(), data, count);
                    engine::vector<char> uploaddata(count * sizeof(uint16_t));
                    file.read(uploaddata.data(), static_cast<std::streamsize>(sizeof(uint16_t)* count));
                    auto uploadFuture = residency.uploadTemp(
                        modelResources.gpuBuffers().index().buffer(),
                        data.modelResource.gpuIndex * sizeof(uint16_t),
                        &uploaddata[0], uploaddata.size());
                    uploadFuture.blockUntilUploaded();
#else
                    readBlock<uint16_t>(modelResources.residency(), modelResources.gpuBuffers().indexAllocator(), file, gpuData[currentLod].triangleData);
                    gpuData[currentLod].triangleData.uploads.back().buffer = modelResources.gpuBuffers().index();
                    modelResources.residency().makeResident(gpuData[currentLod].triangleData.uploads.back());
                    modelResources.residency().freeUpdateAllocation(gpuData[currentLod].triangleData.uploads.back());
                    gpuData[currentLod].triangleData.uploads.pop_back();
#endif

                    break;
                }
                case MeshBlockType::AdjacencyData:
                {
                    auto& data = gpuData[currentLod].adjacencyData;
                    Count count;
                    file.read(reinterpret_cast<char*>(&count), sizeof(Count));
                    allocateIfNecessary(modelResources.gpuBuffers().adjacencyAllocator(), data, count);
                    engine::vector<char> uploaddata(count * sizeof(uint32_t));
                    file.read(uploaddata.data(), static_cast<std::streamsize>(sizeof(uint32_t) * count));
                    auto uploadFuture = residency.uploadTemp(
                        modelResources.gpuBuffers().adjacency().buffer(),
                        data.modelResource.gpuIndex * sizeof(uint32_t),
                        &uploaddata[0], uploaddata.size());
                    uploadFuture.blockUntilUploaded();

                    //ASSERT(false, "Implement V2 residency here");
                    /*readBlock<uint32_t>(modelResources.residency(), modelResources.gpuBuffers().adjacencyAllocator(), file, gpuData[currentLod].adjacencyData);
                    gpuData[currentLod].adjacencyData.uploads.back().indexBuffer = modelResources.gpuBuffers().adjacency();
					gpuData[currentLod].adjacencyData.uploads.back().buffer = {};
                    auto ptr = reinterpret_cast<uint32_t*>(gpuData[currentLod].adjacencyData.uploads.back().ptr);
                    for (uint32_t i = 0; i < gpuData[currentLod].adjacencyData.modelResource.elements; ++i)
                    {
                        *ptr += gpuData[currentLod].vertexData.modelResource.gpuIndex;
                        ++ptr;
                    }
                    modelResources.residency().makeResident(gpuData[currentLod].adjacencyData.uploads.back());
                    modelResources.residency().freeUpdateAllocation(gpuData[currentLod].adjacencyData.uploads.back());
                    gpuData[currentLod].adjacencyData.uploads.pop_back();*/
                    break;
                }
                case MeshBlockType::Color: 
                {
#ifdef USE_RESIDENCY_V2
                    auto& data = gpuData[currentLod].vertexData;
                    Count count;
                    file.read(reinterpret_cast<char*>(&count), sizeof(Count));
                    allocateIfNecessary(modelResources.gpuBuffers().vertexDataAllocator(), data, count);
                    engine::vector<char> uploaddata(count * sizeof(Vector4<unsigned char>));
                    file.read(uploaddata.data(), static_cast<std::streamsize>(sizeof(Vector4<unsigned char>)* count));
                    auto uploadFuture = residency.uploadTemp(
                        modelResources.gpuBuffers().color().buffer(),
                        data.modelResource.gpuIndex * sizeof(Vector4<unsigned char>),
                        &uploaddata[0], uploaddata.size());
                    uploadFuture.blockUntilUploaded();
#else
                    readBlock<Vector4<unsigned char>>(modelResources.residency(), modelResources.gpuBuffers().vertexDataAllocator(), file, gpuData[currentLod].vertexData);
                    gpuData[currentLod].vertexData.uploads.back().buffer = modelResources.gpuBuffers().color();
                    modelResources.residency().makeResident(gpuData[currentLod].vertexData.uploads.back());
                    modelResources.residency().freeUpdateAllocation(gpuData[currentLod].vertexData.uploads.back());
                    gpuData[currentLod].vertexData.uploads.pop_back();
                    // we don't clear the uploads because of adjacency calculation
#endif
                    break;
                }
                case MeshBlockType::Material:
                {
                    if (blockHeader.size_bytes > 0)
                    {
                        engine::vector<uint8_t> data(blockHeader.size_bytes);
                        file.read(reinterpret_cast<char*>(&data[0]), blockHeader.size_bytes);
                        out_material.load(data);
                    }
                    else
                    {
                        LOG("found null material in the model file");
                    }
                    break;
                }
                case MeshBlockType::BoundingBox:
                {
                    readBlock<BoundingBox>(file, boundingBox);
                    break;
                }
                case MeshBlockType::VertexScale:
                {
                    readBlock<VertexScale>(file, meshScale);
                    break;
                }
                case MeshBlockType::ClusterVertexStart:
                {
                    if(currentLod >= outputData.size())
                        outputData.resize(currentLod + 1);
                    readBlock<uint32_t>(file, outputData[currentLod].clusterVertexStarts);
                    break;
                }
                case MeshBlockType::ClusterIndexStart:
                {
                    if (currentLod >= outputData.size())
                        outputData.resize(currentLod + 1);
                    readBlock<uint32_t>(file, outputData[currentLod].clusterIndexStarts);
                    break;
                }
                case MeshBlockType::ClusterIndexCount:
                {
                    if (currentLod >= outputData.size())
                        outputData.resize(currentLod + 1);
                    readBlock<uint32_t>(file, outputData[currentLod].clusterIndexCount);
                    break;
                }
                case MeshBlockType::ClusterBounds:
                {
                    if (currentLod >= outputData.size())
                        outputData.resize(currentLod + 1);
                    readBlock<BoundingBox>(file, outputData[currentLod].clusterBounds);
                    break;
                }
                case MeshBlockType::ClusterCones:
                {
                    if (currentLod >= outputData.size())
                        outputData.resize(currentLod + 1);
                    readBlock<Vector4f>(file, outputData[currentLod].clusterCones);
                    break;
                }
                default: file.seekg(static_cast<std::streamoff>(blockHeader.size_bytes), std::ios::cur);
            }
            --elements;
        }

        // vertex, uv and triangle data has already been handled above
        // now we need to handle cluster and submesh data
        m_clusterCount = 0;
        for (int i = 0; i < gpuData.size(); ++i)
        {

            // cluster data
            {
#ifdef USE_RESIDENCY_V2
                gpuData[i].clusterData.modelResource = modelResources.gpuBuffers().clusterDataAllocator().allocate(static_cast<uint32_t>(outputData[i].clusterIndexStarts.size()));
                m_clusterCount += outputData[i].clusterIndexStarts.size();
                engine::vector<ClusterData> cData(outputData[i].clusterIndexStarts.size());
                engine::vector<BoundingBox> bbData(outputData[i].clusterIndexStarts.size());
                engine::vector<Vector4f> bsData(outputData[i].clusterIndexStarts.size());
                size_t indexStart = gpuData[i].triangleData.modelResource.gpuIndex;
                for (int a = 0; a < outputData[i].clusterIndexStarts.size(); ++a)
                {
                    cData[a].indexCount = outputData[i].clusterIndexCount[a];
                    cData[a].indexPointer = static_cast<uint>(indexStart);
                    cData[a].vertexPointer = static_cast<uint>(gpuData[i].vertexData.modelResource.gpuIndex + outputData[i].clusterVertexStarts[a]);
                    bbData[a] = outputData[i].clusterBounds[a];
                    bsData[a] = outputData[i].clusterCones[a]; // TODO: there actually isn't any bounding sphere data currently

                    indexStart += outputData[i].clusterIndexCount[a];
                }

                auto uploadFuture0 = residency.uploadTemp(modelResources.gpuBuffers().clusterBinding().buffer(), gpuData[i].clusterData.modelResource.gpuIndex * sizeof(ClusterData), &cData[0], sizeof(ClusterData) * cData.size());
                auto uploadFuture1 = residency.uploadTemp(modelResources.gpuBuffers().clusterBoundingBox().buffer(), gpuData[i].clusterData.modelResource.gpuIndex * sizeof(BoundingBox), &bbData[0], sizeof(BoundingBox) * bbData.size());
                auto uploadFuture2 = residency.uploadTemp(modelResources.gpuBuffers().clusterCone().buffer(), gpuData[i].clusterData.modelResource.gpuIndex * sizeof(Vector4f), &bsData[0], sizeof(Vector4f) * bsData.size());
                uploadFuture0.blockUntilUploaded();
                uploadFuture1.blockUntilUploaded();
                uploadFuture2.blockUntilUploaded();
#else
                gpuData[i].clusterData.modelResource = modelResources.gpuBuffers().clusterDataAllocator().allocate(static_cast<uint32_t>(outputData[i].clusterId.size()));
                gpuData[i].clusterData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(ClusterData)* outputData[i].clusterId.size())));
                gpuData[i].clusterData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(BoundingBox)* outputData[i].clusterId.size())));
                gpuData[i].clusterData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(Vector4f)* outputData[i].clusterId.size())));

                gpuData[i].clusterData.uploads[0].gpuIndex = gpuData[i].clusterData.modelResource.gpuIndex;
                gpuData[i].clusterData.uploads[1].gpuIndex = gpuData[i].clusterData.modelResource.gpuIndex;
                gpuData[i].clusterData.uploads[2].gpuIndex = gpuData[i].clusterData.modelResource.gpuIndex;

                gpuData[i].clusterData.uploads[0].buffer = modelResources.gpuBuffers().clusterBinding();
                gpuData[i].clusterData.uploads[1].buffer = modelResources.gpuBuffers().clusterBoundingBox();
                gpuData[i].clusterData.uploads[2].buffer = modelResources.gpuBuffers().clusterCone();

                ClusterData* cData = reinterpret_cast<ClusterData*>(gpuData[i].clusterData.uploads[0].ptr);
                BoundingBox* bbData = reinterpret_cast<BoundingBox*>(gpuData[i].clusterData.uploads[1].ptr);
                Vector4f* bsData = reinterpret_cast<Vector4f*>(gpuData[i].clusterData.uploads[2].ptr);
                uint32_t indexStart = gpuData[i].triangleData.modelResource.gpuIndex;
                for (int a = 0; a < outputData[i].clusterId.size(); ++a)
                {
                    cData->indexCount = outputData[i].clusterIndexCount[a];
                    cData->indexPointer = indexStart;
                    cData->vertexPointer = gpuData[i].vertexData.modelResource.gpuIndex + outputData[i].clusterId[a];
                    *bbData = outputData[i].clusterBounds[a];
                    *bsData = outputData[i].clusterCones[a]; // TODO: there actually isn't any bounding sphere data currently

                    indexStart += outputData[i].clusterIndexCount[a];
                    cData++;
                    bbData++;
                    bsData++;

                }

                modelResources.residency().makeResident(gpuData[i].clusterData.uploads[0]);
                modelResources.residency().makeResident(gpuData[i].clusterData.uploads[1]);
                modelResources.residency().makeResident(gpuData[i].clusterData.uploads[2]);
                modelResources.residency().freeUpdateAllocation(gpuData[i].clusterData.uploads[0]);
                modelResources.residency().freeUpdateAllocation(gpuData[i].clusterData.uploads[1]);
                modelResources.residency().freeUpdateAllocation(gpuData[i].clusterData.uploads[2]);
                gpuData[i].clusterData.uploads.clear();
#endif
            }

            // submesh data
            {
#ifdef USE_RESIDENCY_V2
                gpuData[i].subMeshData.modelResource = modelResources.gpuBuffers().subMeshDataAllocator().allocate(1);
                SubMeshAdjacency   sAdjacency;
                SubMeshData        sMeshData;
                BoundingBox        sbbData;
                BoundingSphere     sbsData;
                
                sAdjacency.adjacencyPointer = static_cast<uint>(gpuData[i].adjacencyData.modelResource.gpuIndex);
                sAdjacency.adjacencyCount = static_cast<uint>(gpuData[i].adjacencyData.modelResource.elements);
                sAdjacency.baseVertexPointer = static_cast<uint>(gpuData[i].vertexData.modelResource.gpuIndex);
                sMeshData.clusterCount = static_cast<uint>(outputData[i].clusterIndexStarts.size());
                sMeshData.clusterPointer = static_cast<uint>(gpuData[i].clusterData.modelResource.gpuIndex);
                
                sbbData = boundingBox;
                sbsData = boundingSphere;

                auto uploadFuture0 = residency.uploadTemp(modelResources.gpuBuffers().subMeshAdjacency().buffer(), gpuData[i].subMeshData.modelResource.gpuIndex * sizeof(SubMeshAdjacency), &sAdjacency, sizeof(SubMeshAdjacency));
                auto uploadFuture1 = residency.uploadTemp(modelResources.gpuBuffers().subMeshData().buffer(), gpuData[i].subMeshData.modelResource.gpuIndex * sizeof(SubMeshData), &sMeshData, sizeof(SubMeshData));
                auto uploadFuture2 = residency.uploadTemp(modelResources.gpuBuffers().subMeshBoundingBox().buffer(), gpuData[i].subMeshData.modelResource.gpuIndex * sizeof(BoundingBox), &sbbData, sizeof(BoundingBox));
                auto uploadFuture3 = residency.uploadTemp(modelResources.gpuBuffers().subMeshBoundingSphere().buffer(), gpuData[i].subMeshData.modelResource.gpuIndex * sizeof(BoundingSphere), &sbsData, sizeof(BoundingSphere));
                uploadFuture0.blockUntilUploaded();
                uploadFuture1.blockUntilUploaded();
                uploadFuture2.blockUntilUploaded();
                uploadFuture3.blockUntilUploaded();
#else
                gpuData[i].subMeshData.modelResource = modelResources.gpuBuffers().subMeshDataAllocator().allocate(1);
                gpuData[i].subMeshData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(SubMeshAdjacency)));
                gpuData[i].subMeshData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(SubMeshData)));
                gpuData[i].subMeshData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(BoundingBox)));
                gpuData[i].subMeshData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(BoundingSphere)));

                gpuData[i].subMeshData.uploads[0].gpuIndex = gpuData[i].subMeshData.modelResource.gpuIndex;
                gpuData[i].subMeshData.uploads[1].gpuIndex = gpuData[i].subMeshData.modelResource.gpuIndex;
                gpuData[i].subMeshData.uploads[2].gpuIndex = gpuData[i].subMeshData.modelResource.gpuIndex;
                gpuData[i].subMeshData.uploads[3].gpuIndex = gpuData[i].subMeshData.modelResource.gpuIndex;

                gpuData[i].subMeshData.uploads[0].buffer = modelResources.gpuBuffers().subMeshAdjacency();
                gpuData[i].subMeshData.uploads[1].buffer = modelResources.gpuBuffers().subMeshData();
                gpuData[i].subMeshData.uploads[2].buffer = modelResources.gpuBuffers().subMeshBoundingBox();
                gpuData[i].subMeshData.uploads[3].buffer = modelResources.gpuBuffers().subMeshBoundingSphere();

                SubMeshAdjacency* sAdjacency = reinterpret_cast<SubMeshAdjacency*>(gpuData[i].subMeshData.uploads[0].ptr);
                SubMeshData* sMeshData = reinterpret_cast<SubMeshData*>(gpuData[i].subMeshData.uploads[1].ptr);
                BoundingBox* sbbData = reinterpret_cast<BoundingBox*>(gpuData[i].subMeshData.uploads[2].ptr);
                BoundingSphere* sbsData = reinterpret_cast<BoundingSphere*>(gpuData[i].subMeshData.uploads[3].ptr);

                sAdjacency->adjacencyPointer = gpuData[i].adjacencyData.modelResource.gpuIndex;
                sAdjacency->adjacencyCount = gpuData[i].adjacencyData.modelResource.elements;
                sAdjacency->baseVertexPointer = gpuData[i].vertexData.modelResource.gpuIndex;
                sMeshData->clusterCount = static_cast<uint>(outputData[i].clusterId.size());
                sMeshData->clusterPointer = gpuData[i].clusterData.modelResource.gpuIndex;
                *sbbData = boundingBox;
                *sbsData = boundingSphere;

                modelResources.residency().makeResident(gpuData[i].subMeshData.uploads[0]);
                modelResources.residency().makeResident(gpuData[i].subMeshData.uploads[1]);
                modelResources.residency().makeResident(gpuData[i].subMeshData.uploads[2]);
                modelResources.residency().makeResident(gpuData[i].subMeshData.uploads[3]);
                modelResources.residency().freeUpdateAllocation(gpuData[i].subMeshData.uploads[0]);
                modelResources.residency().freeUpdateAllocation(gpuData[i].subMeshData.uploads[1]);
                modelResources.residency().freeUpdateAllocation(gpuData[i].subMeshData.uploads[2]);
                modelResources.residency().freeUpdateAllocation(gpuData[i].subMeshData.uploads[3]);
                gpuData[i].subMeshData.uploads.clear();
#endif
            }
        }

        // lod binding data
#ifdef USE_RESIDENCY_V2
        lodData.modelResource = modelResources.gpuBuffers().lodAllocator().allocate(static_cast<uint32_t>(gpuData.size()));
        engine::vector<SubMeshUVLod> lodBindingPtr(gpuData.size());
        for (int i = 0; i < gpuData.size(); ++i)
        {
            lodBindingPtr[i].submeshPointer = static_cast<uint>(gpuData[i].subMeshData.modelResource.gpuIndex);
            if (gpuData[i].uvData.size() > 0)
                lodBindingPtr[i].uvPointer = static_cast<uint>(gpuData[i].uvData[0].modelResource.gpuIndex);
            else
                lodBindingPtr[i].uvPointer = 0u;
        }
        auto uploadFuture = residency.uploadTemp(modelResources.gpuBuffers().lod().buffer(), lodData.modelResource.gpuIndex * sizeof(SubMeshUVLod), &lodBindingPtr[0], sizeof(SubMeshUVLod) * lodBindingPtr.size());
        uploadFuture.blockUntilUploaded();
#else
        lodData.modelResource = modelResources.gpuBuffers().lodAllocator().allocate(static_cast<uint32_t>(gpuData.size()));
        lodData.uploads.emplace_back(modelResources.residency().createUpdateAllocation(static_cast<uint32_t>(sizeof(SubMeshUVLod)* gpuData.size())));
        lodData.uploads[0].gpuIndex = lodData.modelResource.gpuIndex;
        lodData.uploads[0].buffer = modelResources.gpuBuffers().lod();
        SubMeshUVLod* lodBindingPtr = reinterpret_cast<SubMeshUVLod*>(lodData.uploads[0].ptr);
        for (int i = 0; i < gpuData.size(); ++i)
        {
            lodBindingPtr->submeshPointer = gpuData[i].subMeshData.modelResource.gpuIndex;
            if (gpuData[i].uvData.size() > 0)
                lodBindingPtr->uvPointer = gpuData[i].uvData[0].modelResource.gpuIndex;
            else
                lodBindingPtr->uvPointer = 0;
            ++lodBindingPtr;
        }
        modelResources.residency().makeResident(lodData.uploads[0]);
        modelResources.residency().freeUpdateAllocation(lodData.uploads[0]);
#endif

        return true;
    }

	engine::shared_ptr<SubMeshInstance> SubMesh::createInstance(Device& device)
	{
        return createInstance(
            device,
            gpuData[0].uvData,
            m_instanceCount,
            m_clusterCount,
			lodData, 
			gpuData.size(),
			meshScale);
	}

	void SubMesh::freeInstance(Device& device, SubMeshInstance* instance)
	{
		freeInstance(device, instance, m_instanceCount);
	}

	engine::shared_ptr<SubMeshInstance> SubMesh::createInstance(
        Device& device,
		const engine::vector<ModelResource>& uvData,
		size_t& instanceCount, 
        size_t& clusterCount,
		ModelResource& lodData, 
		size_t lodCount,
		VertexScale vertexScale)
    {
        engine::shared_ptr<SubMeshInstance> instance = engine::shared_ptr<SubMeshInstance>(
            new SubMeshInstance(), 
            [&device, &instanceCount](SubMeshInstance* ptr)
            {
                SubMesh::freeInstance(device, ptr, instanceCount); delete ptr;
            });

        
        instance->clusterCount = static_cast<uint32_t>(clusterCount);

        ModelResource& modres = instance->instanceData;

        for (auto&& uv : uvData)
        {
            instance->uvData.emplace_back(&uv);
        }

        modres.modelResource = device.modelResources().gpuBuffers().instanceDataAllocator().allocate(1);

        device.modelResources().gpuBuffers().addInstance(instance.get());

        auto maxInstanceIndex = device.modelResources().gpuBuffers().maxInstanceIndexInUse();
        if (maxInstanceIndex == InvalidMaxInstanceIndex)
            device.modelResources().gpuBuffers().maxInstanceIndexInUse(modres.modelResource.gpuIndex);
        else if (maxInstanceIndex < modres.modelResource.gpuIndex)
            device.modelResources().gpuBuffers().maxInstanceIndexInUse(modres.modelResource.gpuIndex);

#ifdef USE_RESIDENCY_V2

        TransformHistory transform = TransformHistory{};
        LodBinding lodBinding = { static_cast<uint>(lodData.modelResource.gpuIndex), static_cast<uint32_t>(lodCount), 0, 0 };
        InstanceMaterial material;
        material.albedo = 0;
        material.metalness = 0;
        material.roughness = 0;
        material.normal = 0;
        material.ao = 0;
        material.materialSet = 0;
        material.roughnessStrength = 0.0f;
        material.metalnessStrength = 0.0f;
        material.occlusionStrength = 0.0f;
        material.scaleX = 0.0f;
        material.scaleY = 0.0f;
        material.padding = 0;
        material.color = {};

        VertexScale vertexScalePtr = vertexScale;
        
        auto clusterTracking = device.modelResources().gpuBuffers().allocateClusterTracking(clusterCount);
        uint32_t clusterTrackingPtr = static_cast<uint32_t>(clusterTracking.gpuIndex);

        auto uploadFuture0 = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().instanceTransform().buffer(), modres.modelResource.gpuIndex * sizeof(TransformHistory), &transform, sizeof(TransformHistory));
        auto uploadFuture1 = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().instanceLodBinding().buffer(), modres.modelResource.gpuIndex * sizeof(LodBinding), &lodBinding, sizeof(LodBinding));
        auto uploadFuture2 = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().instanceMaterial().buffer(), modres.modelResource.gpuIndex * sizeof(InstanceMaterial), &material, sizeof(InstanceMaterial));
        auto uploadFuture3 = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().instanceScale().buffer(), modres.modelResource.gpuIndex * sizeof(VertexScale), &vertexScalePtr, sizeof(VertexScale));
        auto uploadFuture4 = device.residencyV2().uploadTemp(device.modelResources().gpuBuffers().instanceClusterTracking().buffer(), modres.modelResource.gpuIndex * sizeof(uint32_t), &clusterTrackingPtr, sizeof(uint32_t));
        uploadFuture0.blockUntilUploaded();
        uploadFuture1.blockUntilUploaded();
        uploadFuture2.blockUntilUploaded();
        uploadFuture3.blockUntilUploaded();
        uploadFuture4.blockUntilUploaded();


#else
        modres.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(TransformHistory)));
        modres.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(LodBinding)));
        modres.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(InstanceMaterial)));
        modres.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(VertexScale)));
        modres.uploads.emplace_back(modelResources.residency().createUpdateAllocation(sizeof(uint32_t)));
        
        modres.uploads[0].gpuIndex = modres.modelResource.gpuIndex;
        modres.uploads[1].gpuIndex = modres.modelResource.gpuIndex;
        modres.uploads[2].gpuIndex = modres.modelResource.gpuIndex;
        modres.uploads[3].gpuIndex = modres.modelResource.gpuIndex;
        modres.uploads[4].gpuIndex = modres.modelResource.gpuIndex;
        
        modres.uploads[0].buffer = modelResources.gpuBuffers().instanceTransform();
        modres.uploads[1].buffer = modelResources.gpuBuffers().instanceLodBinding();
        modres.uploads[2].buffer = modelResources.gpuBuffers().instanceMaterial();
        modres.uploads[3].buffer = modelResources.gpuBuffers().instanceScale();
        modres.uploads[4].buffer = modelResources.gpuBuffers().instanceClusterTracking();

        TransformHistory*   transform       = reinterpret_cast<TransformHistory*>(modres.uploads[0].ptr);
        LodBinding*         lodBinding      = reinterpret_cast<LodBinding*>(modres.uploads[1].ptr);
        InstanceMaterial*   material        = reinterpret_cast<InstanceMaterial*>(modres.uploads[2].ptr);
        VertexScale*        vertexScalePtr     = reinterpret_cast<VertexScale*>(modres.uploads[3].ptr);
        uint32_t*        clusterTrackingPtr = reinterpret_cast<uint32_t*>(modres.uploads[4].ptr);

        lodBinding->lodPointer = lodData.modelResource.gpuIndex;
        lodBinding->lodCount = static_cast<uint32_t>(lodCount);
        *transform = TransformHistory{};

        material->albedo = 0;
        material->metalness = 0;
        material->roughness = 0;
        material->normal = 0;
        material->ao = 0;
        material->materialSet = 0;
        material->roughnessStrength = 0.0f;
        material->metalnessStrength = 0.0f;
        material->occlusionStrength = 0.0f;
        material->scaleX = 0.0f;
        material->scaleY = 0.0f;
        material->padding = 0;
        material->color = {};

        *vertexScalePtr = vertexScale;

        // allocate data for cluster tracking
        auto clusterTracking = modelResources.gpuBuffers().allocateClusterTracking(clusterCount);
        *clusterTrackingPtr = clusterTracking.gpuIndex;

        modelResources.residency().makeResident(modres.uploads[0]);
        modelResources.residency().makeResident(modres.uploads[1]);
        modelResources.residency().makeResident(modres.uploads[2]);
        modelResources.residency().makeResident(modres.uploads[3]);
        modelResources.residency().makeResident(modres.uploads[4]);
        modelResources.residency().freeUpdateAllocation(modres.uploads[0]);
        modelResources.residency().freeUpdateAllocation(modres.uploads[1]);
        modelResources.residency().freeUpdateAllocation(modres.uploads[2]);
        modelResources.residency().freeUpdateAllocation(modres.uploads[3]);
        modelResources.residency().freeUpdateAllocation(modres.uploads[4]);
        modres.uploads.clear();
#endif

        ++instanceCount;

        //modelResources.printBufferStatus();
        return instance;
    }

	void SubMesh::freeInstance(Device& device, SubMeshInstance* instance, size_t& instanceCount)
    {
        auto removeBufferElement = [](CommandList& cmd, Buffer buffer, uint32_t elementToRemove, uint32_t lastElement)
        {
            if(elementToRemove != lastElement)
                cmd.copyBuffer(buffer, buffer, 1, lastElement, elementToRemove);
        };

        auto maxInstanceIndex = device.modelResources().gpuBuffers().maxInstanceIndexInUse();
        if (instance->instanceData.modelResource.gpuIndex <= maxInstanceIndex)
        {
            auto cmd = device.createCommandList("freeInstance, Remove element cmd");
            //removeBufferElement(cmd, device.modelResources().gpuBuffers().instanceTransform().buffer(), instance->instanceData.modelResource.gpuIndex, maxInstanceIndex);
            //removeBufferElement(cmd, device.modelResources().gpuBuffers().instanceLodBinding().buffer(), instance->instanceData.modelResource.gpuIndex, maxInstanceIndex);
            //removeBufferElement(cmd, device.modelResources().gpuBuffers().instanceMaterial().buffer(), instance->instanceData.modelResource.gpuIndex, maxInstanceIndex);
            //removeBufferElement(cmd, device.modelResources().gpuBuffers().instanceScale().buffer(), instance->instanceData.modelResource.gpuIndex, maxInstanceIndex);
            //removeBufferElement(cmd, device.modelResources().gpuBuffers().instanceClusterTracking().buffer(), instance->instanceData.modelResource.gpuIndex, maxInstanceIndex);

            device.corePipelines()->removeElement.cs.transform = device.modelResources().gpuBuffers().instanceTransformUAV();
            device.corePipelines()->removeElement.cs.lodBinding = device.modelResources().gpuBuffers().instanceLodBindingUAV();
            device.corePipelines()->removeElement.cs.materials = device.modelResources().gpuBuffers().instanceMaterialUAV();
            device.corePipelines()->removeElement.cs.scales = device.modelResources().gpuBuffers().instanceScaleUAV();
            device.corePipelines()->removeElement.cs.clusterTracking = device.modelResources().gpuBuffers().instanceClusterTrackingUAV();
            device.corePipelines()->removeElement.cs.elementToRemove.x = static_cast<uint32_t>(instance->instanceData.modelResource.gpuIndex);
            device.corePipelines()->removeElement.cs.lastElement.x = static_cast<uint32_t>(maxInstanceIndex);
            cmd.bindPipe(device.corePipelines()->removeElement);
            cmd.dispatch(1, 1, 1);

            device.submitBlocking(cmd);
        }

        --maxInstanceIndex;
        --instanceCount;
        device.modelResources().gpuBuffers().removeInstance(instance);
        device.modelResources().gpuBuffers().instanceDataAllocator().free(instance->instanceData.modelResource);
        device.modelResources().gpuBuffers().maxInstanceIndexInUse(maxInstanceIndex);

        /*auto maxInstanceIndex = modelResources.gpuBuffers().maxInstanceIndexInUse();
        if (instance->instanceData.modelResource.gpuIndex <= maxInstanceIndex)
        {
            engine::vector<Buffer> gpuInstanceBuffers = {
                modelResources.gpuBuffers().instanceTransform().buffer(),
                modelResources.gpuBuffers().instanceLodBinding().buffer(),
                modelResources.gpuBuffers().instanceMaterial().buffer(),
                modelResources.gpuBuffers().instanceScale().buffer() };

            engine::vector<int32_t> bufferElementSizes = {
                gpuInstanceBuffers[0].description().descriptor.elementSize,
                gpuInstanceBuffers[1].description().descriptor.elementSize,
                gpuInstanceBuffers[2].description().descriptor.elementSize,
                gpuInstanceBuffers[3].description().descriptor.elementSize
            };

            uint32_t allBytes = 0;
            for (auto&& bufElementSize : bufferElementSizes)
                allBytes += bufElementSize;

            auto upload = modelResources.residency().createUpdateAllocation(allBytes);

            {
                auto cmd = modelResources.residency().device().createCommandList("SubMesh::freeInstance", CommandListType::Direct);
                auto currentUploadBytePosition = 0;
                for (int i = 0; i < gpuInstanceBuffers.size(); ++i)
                {
                    cmd.copyBuffer(gpuInstanceBuffers[i], modelResources.residency().uploadBuffer(), 1, maxInstanceIndex, currentUploadBytePosition);
                    currentUploadBytePosition += bufferElementSizes[i];
                }
                currentUploadBytePosition = 0;
                for (int i = 0; i < gpuInstanceBuffers.size(); ++i)
                {
                    cmd.copyBuffer(modelResources.residency().uploadBuffer(), gpuInstanceBuffers[i], bufferElementSizes[i], currentUploadBytePosition, instance->instanceData.modelResource.gpuIndex);
                    currentUploadBytePosition += bufferElementSizes[i];
                }

                modelResources.residency().device().submitBlocking(cmd);
            }

            //modelResources.gpuBuffers().relocateInstance(maxInstanceIndex, instance->instanceData.modelResource.gpuIndex);
            modelResources.gpuBuffers().removeInstance(instance);

            modelResources.gpuBuffers().instanceDataAllocator().free(instance->instanceData.modelResource);
            --maxInstanceIndex;
            modelResources.gpuBuffers().maxInstanceIndexInUse(maxInstanceIndex);

            
        }
        else
        {
            modelResources.gpuBuffers().removeInstance(instance);
            modelResources.gpuBuffers().instanceDataAllocator().free(instance->instanceData.modelResource);
            --maxInstanceIndex;
            modelResources.gpuBuffers().maxInstanceIndexInUse(maxInstanceIndex);
        }
        
        --instanceCount;*/

        //modelResources.printBufferStatus();
    }

    size_t SubMesh::instanceCount() const
    {
        return m_instanceCount;
    }

    SubMesh::Count SubMesh::elementCount() const
    {
        Count count{ 0 };

        for (auto&& out : outputData)
        {
            if (out.vertex.size() > 0) ++count;
            if (out.normal.size() > 0) ++count;
            if (out.tangent.size() > 0) ++count;
            if (out.uv.size() > 0) count += static_cast<engine::SubMesh::Count>(out.uv.size());
            if (out.index.size() > 0) ++count;
            if (out.adjacency.size() > 0) ++count;
            if (out.color.size() > 0) count += static_cast<engine::SubMesh::Count>(out.color.size());
            
            if (out.clusterIndexStarts.size() > 0) ++count;
            if (out.clusterVertexStarts.size() > 0) ++count;
            if (out.clusterIndexCount.size() > 0) ++count;
            if (out.clusterBounds.size() > 0) ++count;
            if (out.clusterCones.size() > 0) ++count;
        }
        if (out_material.data().size() > 0) ++count;
        ++count; // bounding box
        ++count; // mesh scale
        return count;
    }

    void SubMesh::save(CompressedFile& file) const
    {
        Count elements = elementCount();
        file.write(reinterpret_cast<char*>(&elements), sizeof(Count));

        for (auto&& out : outputData)
        {
            if (out.vertex.size() > 0)    writeBlock<Vector2<uint32_t>>(file, MeshBlockType::Position, out.vertex);
            if (out.normal.size() > 0)      writeBlock<Vector2f>(file, MeshBlockType::Normal, out.normal);
            if (out.tangent.size() > 0)     writeBlock<Vector2f>(file, MeshBlockType::Tangent, out.tangent);
            for (auto&& u : out.uv)
            {
                if (u.size() > 0)          writeBlock<Vector2f>(file, MeshBlockType::Uv, u);
            }
            if (out.index.size() > 0)     writeBlock<uint16_t>(file, MeshBlockType::Indice, out.index);
            if (out.adjacency.size() > 0)     writeBlock<uint32_t>(file, MeshBlockType::AdjacencyData, out.adjacency);

            for (auto&& c : out.color)
            {
                if (c.size() > 0)          writeBlock<Vector4<unsigned char>>(file, MeshBlockType::Color, c);
            }

            if (out.clusterIndexStarts.size() > 0)           writeBlock<uint32_t>(file, MeshBlockType::ClusterIndexStart, out.clusterIndexStarts);
            if (out.clusterVertexStarts.size() > 0)           writeBlock<uint32_t>(file, MeshBlockType::ClusterVertexStart, out.clusterVertexStarts);
            if (out.clusterIndexCount.size() > 0)   writeBlock<uint32_t>(file, MeshBlockType::ClusterIndexCount, out.clusterIndexCount);
            if (out.clusterBounds.size() > 0)       writeBlock<BoundingBox>(file, MeshBlockType::ClusterBounds, out.clusterBounds);
            if (out.clusterCones.size() > 0)       writeBlock<Vector4f>(file, MeshBlockType::ClusterCones, out.clusterCones);
        }

        writeBlock<BoundingBox>(file, MeshBlockType::BoundingBox, boundingBox);

        auto materialData = out_material.data();
        if (materialData.size() > 0)
        {
            writeBlockHeader(file, { MeshBlockType::Material, materialData.size() });
            file.write(reinterpret_cast<char*>(materialData.data()), materialData.size());
        }

        writeBlock<VertexScale>(file, MeshBlockType::VertexScale, meshScale);
    }

}
