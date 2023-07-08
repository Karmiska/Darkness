#include "engine/rendering/BufferSettings.h"

namespace engine
{
    // these settings are for huge scenes
#if 0
    // ~40 MB
    uint32_t MaxIndexCount = 180000000;

    // ~80 MB
    uint32_t MaxUvCount = 55000000;

    // ~600 MB
    // single vertex takes 60 bytes
    // it includes vertex, normal, tangent, uv, color
    // TODO: will make this a lot smaller
    uint32_t MaxVertexCount = MaxIndexCount / 2;

    uint32_t ClusterMaxSize = 3 * 64;

    // ~2.6 MB
    // MaxClusterCount ~52083 clusters
    // single cluster takes 52 bytes
    // it includes cluster indexPtr, indexCount, vertexPointer, bb, bs
    uint32_t MaxClusterCount = MaxIndexCount / ClusterMaxSize;
    uint32_t MaxClusterInstanceCount = (10ull * 1024ull * 1024ull);

    // ~45.7 MB
    // single sub model takes 48 bytes
    uint32_t MaxSubModels = 500000;
    uint32_t MaxLods = 10;

    // ~244 MB
    // single instance takes 256 bytes
    // it includes submesh binding, transforms, material
    uint32_t MaxInstances = 500000;

    uint32_t UploadBufferSizeBytes = 1024u * 1024u * 1024u;
    uint32_t ResidencyUploadBufferSizeBytes = 1024u * 1024u * 1024u;

    int ClusterTrackingSize = 100000000;
#elif 0
// ~40 MB
    uint32_t MaxIndexCount = 150000000;

    // ~80 MB
    uint32_t MaxUvCount = 25000000;

    // ~600 MB
    // single vertex takes 60 bytes
    // it includes vertex, normal, tangent, uv, color
    // TODO: will make this a lot smaller
    uint32_t MaxVertexCount = MaxIndexCount / 2;

    uint32_t ClusterMaxSize = 3 * 64;

    // ~2.6 MB
    // MaxClusterCount ~52083 clusters
    // single cluster takes 52 bytes
    // it includes cluster indexPtr, indexCount, vertexPointer, bb, bs
    uint32_t MaxClusterCount = MaxIndexCount / ClusterMaxSize;
    uint32_t MaxClusterInstanceCount = (10ull * 1024ull * 1024ull);

    // ~45.7 MB
    // single sub model takes 48 bytes
    uint32_t MaxSubModels = 500000;
    uint32_t MaxLods = 10;

    // ~244 MB
    // single instance takes 256 bytes
    // it includes submesh binding, transforms, material
    uint32_t MaxInstances = 500000;

    uint32_t UploadBufferSizeBytes = 1024u * 1024u * 100u;
    uint32_t ResidencyUploadBufferSizeBytes = 1024u * 1024u * 100u;

    int ClusterTrackingSize = 100000000;
#elif 0
    // ~40 MB
    uint32_t MaxIndexCount = 90000000;

    // ~80 MB
    uint32_t MaxUvCount = 17000000;

    // ~600 MB
    // single vertex takes 60 bytes
    // it includes vertex, normal, tangent, uv, color
    // TODO: will make this a lot smaller
    uint32_t MaxVertexCount = MaxIndexCount / 2;

    uint32_t ClusterMaxSize = 3 * 64;

    // ~2.6 MB
    // MaxClusterCount ~52083 clusters
    // single cluster takes 52 bytes
    // it includes cluster indexPtr, indexCount, vertexPointer, bb, bs
    uint32_t MaxClusterCount = MaxIndexCount / ClusterMaxSize;
    uint32_t MaxClusterInstanceCount = (10ull * 1024ull * 1024ull);

    // ~45.7 MB
    // single sub model takes 48 bytes
    uint32_t MaxSubModels = 500000;
    uint32_t MaxLods = 10;

    // ~244 MB
    // single instance takes 256 bytes
    // it includes submesh binding, transforms, material
    uint32_t MaxInstances = 500000;

    uint32_t UploadBufferSizeBytes = 1024u * 1024u * 100u;
    uint32_t ResidencyUploadBufferSizeBytes = 1024u * 1024u * 100u;

    int ClusterTrackingSize = 100000000;
#elif 0
    // ~40 MB
    uint32_t MaxIndexCount = 20000000;

    // ~80 MB
    uint32_t MaxUvCount = 20000000;

    // ~600 MB
    // single vertex takes 60 bytes
    // it includes vertex, normal, tangent, uv, color
    // TODO: will make this a lot smaller
    uint32_t MaxVertexCount = MaxIndexCount / 2;

    uint32_t ClusterMaxSize = 3 * 64;

    // ~2.6 MB
    // MaxClusterCount ~52083 clusters
    // single cluster takes 52 bytes
    // it includes cluster indexPtr, indexCount, vertexPointer, bb, bs
    uint32_t MaxClusterCount = MaxIndexCount / ClusterMaxSize;
    uint32_t MaxClusterInstanceCount = (10ull * 1024ull * 1024ull);

    // ~45.7 MB
    // single sub model takes 48 bytes
    uint32_t MaxSubModels = 500000;
    uint32_t MaxLods = 10;

    // ~244 MB
    // single instance takes 256 bytes
    // it includes submesh binding, transforms, material
    uint32_t MaxInstances = 500000;

    uint32_t UploadBufferSizeBytes = 1024u * 1024u * 100u;
    uint32_t ResidencyUploadBufferSizeBytes = 1024u * 1024u * 100u;

    int ClusterTrackingSize = 100000000;
#elif 1
    // ~40 MB
    uint32_t MaxIndexCount = 0;

    // ~80 MB
    uint32_t MaxUvCount = MaxIndexCount;

    // ~600 MB
    // single vertex takes 60 bytes
    // it includes vertex, normal, tangent, uv, color
    // TODO: will make this a lot smaller
    uint32_t MaxVertexCount = MaxIndexCount;

    uint32_t ClusterMaxSize = 3 * 64;

    // ~2.6 MB
    // MaxClusterCount ~52083 clusters
    // single cluster takes 52 bytes
    // it includes cluster indexPtr, indexCount, vertexPointer, bb, bs
    uint32_t MaxClusterCount = 0;
    uint32_t MaxClusterInstanceCount = 1024ull * 2048ull;

    // ~45.7 MB
    // single sub model takes 48 bytes
    uint32_t MaxSubModels = 0;
    uint32_t MaxLods = 0;

    // ~244 MB
    // single instance takes 256 bytes
    // it includes submesh binding, transforms, material
    uint32_t MaxInstances = 15000;

    uint32_t UploadBufferSizeBytes = 1024u * 1024u * 5u;
    uint32_t ResidencyUploadBufferSizeBytes = 1024u * 1024u * 50u;

#elif 1
    // ~40 MB
    uint32_t MaxIndexCount = 200000;

    // ~80 MB
    uint32_t MaxUvCount = MaxIndexCount * 2;

    // ~600 MB
    // single vertex takes 60 bytes
    // it includes vertex, normal, tangent, uv, color
    // TODO: will make this a lot smaller
    uint32_t MaxVertexCount = MaxIndexCount / 2;

    uint32_t ClusterMaxSize = 3 * 64;

    // ~2.6 MB
    // MaxClusterCount ~52083 clusters
    // single cluster takes 52 bytes
    // it includes cluster indexPtr, indexCount, vertexPointer, bb, bs
    uint32_t MaxClusterCount = MaxIndexCount / ClusterMaxSize;
    uint32_t MaxClusterInstanceCount = 1024ull * 512ull;

    // ~45.7 MB
    // single sub model takes 48 bytes
    uint32_t MaxSubModels = 500;
    uint32_t MaxLods = 10;

    // ~244 MB
    // single instance takes 256 bytes
    // it includes submesh binding, transforms, material
    uint32_t MaxInstances = 500;

    uint32_t UploadBufferSizeBytes = 1024u * 1024u * 100u;
    uint32_t ResidencyUploadBufferSizeBytes = 1024u * 1024u * 100u;

    int ClusterTrackingSize = 100000000;
#else
    // ~40 MB
    uint32_t MaxIndexCount = 200000;

    // ~80 MB
    uint32_t MaxUvCount = MaxIndexCount * 2;

    // ~600 MB
    // single vertex takes 60 bytes
    // it includes vertex, normal, tangent, uv, color
    // TODO: will make this a lot smaller
    uint32_t MaxVertexCount = MaxIndexCount / 2;

    uint32_t ClusterMaxSize = 3 * 64;

    // ~2.6 MB
    // MaxClusterCount ~52083 clusters
    // single cluster takes 52 bytes
    // it includes cluster indexPtr, indexCount, vertexPointer, bb, bs
    uint32_t MaxClusterCount = MaxIndexCount / ClusterMaxSize;
    uint32_t MaxClusterInstanceCount = 10000u;

    // ~45.7 MB
    // single sub model takes 48 bytes
    uint32_t MaxSubModels = 50;
    uint32_t MaxLods = 10;

    // ~244 MB
    // single instance takes 256 bytes
    // it includes submesh binding, transforms, material
    uint32_t MaxInstances = 50;

    uint32_t UploadBufferSizeBytes = 10 * 1024u * 1024u;
    uint32_t ResidencyUploadBufferSizeBytes = 10 * 1024u * 1024u;

    int ClusterTrackingSize = 10000;
#endif
}
