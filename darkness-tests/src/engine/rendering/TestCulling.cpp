#include "GlobalTestFixture.h"
#include "engine/rendering/culling/FrustumCuller.h"
#include "engine/rendering/DepthPyramid.h"
#include "engine/primitives/BoundingBox.h"
#include "components/Transform.h"
#include "components/Camera.h"

TEST(TestDraw, FrustumInstanceCulling)
{
#if 0
    GlobalEnvironment& env = *envPtr;
    engine::ModelResources& resources = env.device().modelResources();
    engine::GpuBuffers& gpuBuffers = resources.gpuBuffers();
    engine::ResidencyManager& residency = resources.residency();

    engine::shared_ptr<Transform> cameraTransform = engine::make_shared<Transform>();
    Camera camera(cameraTransform);
    camera.position({ 0.0f, 0.0f, 10.0f });

    Vector2<int> resolution = { env.window().width(), env.window().height() };
    DepthPyramid depthPyramid(env.device(), resolution.x, resolution.y);

    // ================================================================================================
    // ================================================================================================
    // DATA UPLOAD
    // ================================================================================================
    // ================================================================================================
    // vertex data
    auto vertexCount = 576u;

    engine::vector<Vector2<uint32_t>>      srcVertex;
    engine::vector<Vector2f>               srcNormal;
    engine::vector<Vector2f>               srcTangent;
    engine::vector<Vector4<unsigned char>> srcColor;

    for (uint32_t i = 0; i < vertexCount; ++i)
    {
        srcVertex.emplace_back(Vector2<uint32_t>{ i, i });
        srcNormal.emplace_back(Vector2f{ static_cast<float>(i), static_cast<float>(i) });
        srcTangent.emplace_back(Vector2f{ static_cast<float>(i), static_cast<float>(i) });
        srcColor.emplace_back(Vector4<unsigned char>{ 
                static_cast<unsigned char>(i), 
                static_cast<unsigned char>(i), 
                static_cast<unsigned char>(i), 
                static_cast<unsigned char>(i) });
    }

    auto vertexData = gpuBuffers.vertexDataAllocator().allocate(vertexCount);
    auto vertexUpload = residency.createUpdateAllocation(vertexCount * sizeof(Vector2<uint32_t>));
    auto normalUpload = residency.createUpdateAllocation(vertexCount * sizeof(Vector2f));
    auto tangentUpload = residency.createUpdateAllocation(vertexCount * sizeof(Vector2f));
    auto colorUpload = residency.createUpdateAllocation(vertexCount * sizeof(Vector4<unsigned char>));

    vertexUpload.gpuIndex = vertexData.gpuIndex;
    vertexUpload.buffer = gpuBuffers.vertex();
    normalUpload.gpuIndex = vertexData.gpuIndex;
    normalUpload.buffer = gpuBuffers.normal();
    tangentUpload.gpuIndex = vertexData.gpuIndex;
    tangentUpload.buffer = gpuBuffers.tangent();
    colorUpload.gpuIndex = vertexData.gpuIndex;
    colorUpload.buffer = gpuBuffers.color();

    memcpy(vertexUpload.ptr,    &srcVertex[0],  srcVertex.size()    * sizeof(Vector2<uint32_t>));
    memcpy(normalUpload.ptr,    &srcNormal[0],  srcNormal.size()    * sizeof(Vector2f));
    memcpy(tangentUpload.ptr,   &srcTangent[0], srcTangent.size()   * sizeof(Vector2f));
    memcpy(colorUpload.ptr,     &srcColor[0],   srcColor.size()     * sizeof(Vector4<unsigned char>));

    residency.makeResident(vertexUpload);
    residency.makeResident(normalUpload);
    residency.makeResident(tangentUpload);
    residency.makeResident(colorUpload);

    residency.freeUpdateAllocation(vertexUpload);
    residency.freeUpdateAllocation(normalUpload);
    residency.freeUpdateAllocation(tangentUpload);
    residency.freeUpdateAllocation(colorUpload);

    // index data
    auto indexCount = vertexCount * 3;
    engine::vector<uint32_t> indexes(indexCount);
    for (uint32_t i = 0; i < indexCount; ++i)
        indexes[i] = i;

    auto indexData = gpuBuffers.indexAllocator().allocate(indexCount);
    auto indexUpload = residency.createUpdateAllocation(indexCount * sizeof(uint32_t));
    indexUpload.gpuIndex = indexData.gpuIndex;
    indexUpload.buffer = gpuBuffers.index();
    memcpy(indexUpload.ptr, &indexes[0], indexes.size() * sizeof(uint32_t));
    residency.makeResident(indexUpload);
    residency.freeUpdateAllocation(indexUpload);

    // uv data
    auto uvCount = vertexCount;
    engine::vector<Vector2f> uvs(uvCount);
    for (uint32_t i = 0; i < uvCount; ++i)
        uvs[i] = Vector2f{ static_cast<float>(i), static_cast<float>(i) };

    auto uvData = gpuBuffers.uvDataAllocator().allocate(uvCount);
    auto uvUpload = residency.createUpdateAllocation(uvCount * sizeof(Vector2f));
    uvUpload.gpuIndex = uvData.gpuIndex;
    uvUpload.buffer = gpuBuffers.uv();
    memcpy(uvUpload.ptr, &uvs[0], uvs.size() * sizeof(uint32_t));
    residency.makeResident(uvUpload);
    residency.freeUpdateAllocation(uvUpload);

    // adjacency data
    engine::vector<Vector2<uint32_t>>      adjVertex;
    for (uint32_t i = 0; i < vertexCount * 2; ++i)
    {
        adjVertex.emplace_back(Vector2<uint32_t>{ i, i });
    }
    auto adjvertexData = gpuBuffers.vertexDataAllocator().allocate(static_cast<uint32_t>(adjVertex.size()));
    auto adjvertexUpload = residency.createUpdateAllocation(static_cast<uint32_t>(adjVertex.size() * sizeof(Vector2<uint32_t>)));
    adjvertexUpload.gpuIndex = adjvertexData.gpuIndex;
    adjvertexUpload.indexBuffer = gpuBuffers.adjacency();
    memcpy(adjvertexUpload.ptr, &adjVertex[0], adjVertex.size() * sizeof(Vector2<uint32_t>));
    residency.makeResident(adjvertexUpload);
    residency.freeUpdateAllocation(adjvertexUpload);

    // per cluster data
    int clusterCount = roundUpToMultiple(vertexCount, 192) / 192;
    auto clusters = gpuBuffers.clusterDataAllocator().allocate(clusterCount);
    auto clusterDataUpload = residency.createUpdateAllocation(static_cast<uint32_t>(sizeof(ClusterData) * clusterCount));
    auto clusterBoundingBoxUpload = residency.createUpdateAllocation(static_cast<uint32_t>(sizeof(engine::BoundingBox) * clusterCount));
    auto clusterConeUpload = residency.createUpdateAllocation(static_cast<uint32_t>(sizeof(Vector4f) * clusterCount));
    clusterDataUpload.gpuIndex = clusters.gpuIndex;
    clusterDataUpload.buffer = gpuBuffers.clusterBinding();
    clusterBoundingBoxUpload.gpuIndex = clusters.gpuIndex;
    clusterBoundingBoxUpload.buffer = gpuBuffers.clusterBoundingBox();
    clusterConeUpload.gpuIndex = clusters.gpuIndex;
    clusterConeUpload.buffer = gpuBuffers.clusterCone();
    
    ClusterData* clusterData = reinterpret_cast<ClusterData*>(clusterDataUpload.ptr);
    engine::BoundingBox* clusterBoudingBox = reinterpret_cast<engine::BoundingBox*>(clusterBoundingBoxUpload.ptr);
    Vector4f* clusterCones = reinterpret_cast<Vector4f*>(clusterConeUpload.ptr);
    uint32_t indexPtr = indexData.gpuIndex;
    for (int i = 0; i < clusterCount; ++i)
    {
        clusterData[i].vertexPointer = vertexData.gpuIndex;
        clusterData[i].indexPointer = indexPtr;
        clusterData[i].indexCount = 192;
        indexPtr += 192;

        clusterBoudingBox[i] = { Vector3f{}, Vector3f{} };
        clusterCones[i] = Vector4f{};
    }
    residency.makeResident(clusterDataUpload);
    residency.makeResident(clusterBoundingBoxUpload);
    residency.makeResident(clusterConeUpload);
    residency.freeUpdateAllocation(clusterDataUpload);
    residency.freeUpdateAllocation(clusterBoundingBoxUpload);
    residency.freeUpdateAllocation(clusterConeUpload);

    // submesh data
    auto submesh = gpuBuffers.subMeshDataAllocator().allocate(1);
    auto subMeshAdjUpload = residency.createUpdateAllocation(sizeof(SubMeshAdjacency));
    auto subMeshDataUpload = residency.createUpdateAllocation(sizeof(SubMeshData));
    auto subMeshBBUpload = residency.createUpdateAllocation(sizeof(engine::BoundingBox));
    auto subMeshConeUpload = residency.createUpdateAllocation(sizeof(BoundingSphere));

    subMeshAdjUpload.gpuIndex =  submesh.gpuIndex;
    subMeshDataUpload.gpuIndex = submesh.gpuIndex;
    subMeshBBUpload.gpuIndex =   submesh.gpuIndex;
    subMeshConeUpload.gpuIndex = submesh.gpuIndex;

    subMeshAdjUpload.buffer =  gpuBuffers.subMeshAdjacency();
    subMeshDataUpload.buffer = gpuBuffers.subMeshData();
    subMeshBBUpload.buffer =   gpuBuffers.subMeshBoundingBox();
    subMeshConeUpload.buffer = gpuBuffers.subMeshBoundingSphere();

    SubMeshAdjacency*       sAdjacency = reinterpret_cast<SubMeshAdjacency*>(subMeshAdjUpload.ptr);
    SubMeshData*            sMeshData = reinterpret_cast<SubMeshData*>(subMeshDataUpload.ptr);
    engine::BoundingBox*    sbbData = reinterpret_cast<engine::BoundingBox*>(subMeshBBUpload.ptr);
    BoundingSphere*         sbsData = reinterpret_cast<BoundingSphere*>(subMeshConeUpload.ptr);

    sAdjacency->adjacencyPointer = adjvertexData.gpuIndex;
    sAdjacency->adjacencyCount = adjvertexData.elements;
    sAdjacency->baseVertexPointer = vertexData.gpuIndex;
    sMeshData->clusterCount = static_cast<uint>(clusterCount);
    sMeshData->clusterPointer = clusters.gpuIndex;
    *sbbData = {};
    *sbsData = Vector4f{};

    residency.makeResident(subMeshAdjUpload);
    residency.makeResident(subMeshDataUpload);
    residency.makeResident(subMeshBBUpload);
    residency.makeResident(subMeshConeUpload);
    residency.freeUpdateAllocation(subMeshAdjUpload);
    residency.freeUpdateAllocation(subMeshDataUpload);
    residency.freeUpdateAllocation(subMeshBBUpload);
    residency.freeUpdateAllocation(subMeshConeUpload);

    // lod binding data
    uint32_t lodCount = 1;
    auto lodBinding = gpuBuffers.lodAllocator().allocate(static_cast<uint32_t>(lodCount));
    auto lodBindingUpload = residency.createUpdateAllocation(static_cast<uint32_t>(sizeof(SubMeshUVLod) * lodCount));
    lodBindingUpload.gpuIndex = lodBinding.gpuIndex;
    lodBindingUpload.buffer = gpuBuffers.lod();
    SubMeshUVLod* lodBindingPtr = reinterpret_cast<SubMeshUVLod*>(lodBindingUpload.ptr);
    for (uint32_t i = 0; i < lodCount; ++i)
    {
        lodBindingPtr->submeshPointer = submesh.gpuIndex;
        lodBindingPtr->uvPointer = uvData.gpuIndex;
        ++lodBindingPtr;
    }
    residency.makeResident(lodBindingUpload);
    residency.freeUpdateAllocation(lodBindingUpload);
    // ================================================================================================
    // ================================================================================================
    // CREATE INSTANCE
    // ================================================================================================
    // ================================================================================================

    auto instance = gpuBuffers.instanceDataAllocator().allocate(1);
    uint64_t instanceGpuIndex = instance.gpuIndex;
    gpuBuffers.addInstance(reinterpret_cast<SubMeshInstance*>(instanceGpuIndex));
    gpuBuffers.maxInstanceIndexInUse(instance.gpuIndex);

    auto instanceTransformUpload = residency.createUpdateAllocation(sizeof(TransformHistory));
    auto instanceLodBindingUpload = residency.createUpdateAllocation(sizeof(LodBinding));
    auto instanceMaterialUpload = residency.createUpdateAllocation(sizeof(InstanceMaterial));

    instanceTransformUpload.gpuIndex = instance.gpuIndex;
    instanceLodBindingUpload.gpuIndex = instance.gpuIndex;
    instanceMaterialUpload.gpuIndex = instance.gpuIndex;

    instanceTransformUpload.buffer =    gpuBuffers.instanceTransform();
    instanceLodBindingUpload.buffer =   gpuBuffers.instanceLodBinding();
    instanceMaterialUpload.buffer =     gpuBuffers.instanceMaterial();

    TransformHistory*   transform = reinterpret_cast<TransformHistory*>(instanceTransformUpload.ptr);
    LodBinding*         instanceLodBinding = reinterpret_cast<LodBinding*>(instanceLodBindingUpload.ptr);
    InstanceMaterial*   material = reinterpret_cast<InstanceMaterial*>(instanceMaterialUpload.ptr);

    *transform = TransformHistory{};
    instanceLodBinding->lodPointer = lodBinding.gpuIndex;
    instanceLodBinding->lodCount = static_cast<uint32_t>(lodCount);

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

    residency.makeResident(instanceTransformUpload);
    residency.makeResident(instanceLodBindingUpload);
    residency.makeResident(instanceMaterialUpload);
    residency.freeUpdateAllocation(instanceTransformUpload);
    residency.freeUpdateAllocation(instanceLodBindingUpload);
    residency.freeUpdateAllocation(instanceMaterialUpload);
    
    // hack
    resources.increaseInstanceCount();
#endif

#if 0
    do
    {
        auto cmd = env.device().createCommandList("FrustumInstanceCulling test cmdList");

        ClusterDataLine outputClusters;

        FrustumInstanceCullingArguments args;
        args.cmd = &cmd;
        args.camera = &camera;
        args.modelResources = &env.device().modelResources();
        args.virtualResolution = resolution;
        args.depthPyramid = &depthPyramid;
        args.output = &outputClusters;
        
        FrustumInstanceCullingOutput output(env.device());
        
        FrustumCuller frustumCuller(env.device());
        frustumCuller.instanceCull(args, output);

        env.submit(cmd);
        env.present();

    } while (env.canContinue(true));
    env.device().waitForIdle();
#endif
}
