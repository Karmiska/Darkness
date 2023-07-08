#include "components/MeshRendererComponent.h"
#include "tools/MeshTools.h"

namespace engine
{
    MeshBuffers convert(
        const engine::string& /*path*/,
        Device& device, 
        engine::shared_ptr<engine::SubMeshInstance>& source)
    {
        MeshBuffers buffers;

        buffers.modelAllocations = device.modelResources().addSubmesh(source);


        /*engine::string key = path + "_vert";
        buffers.vertices = device.createBufferSRV(
            tools::hash(
                reinterpret_cast<const uint8_t*>(key.c_str()),
                static_cast<unsigned int>(key.length())),
            BufferDescription()
            .usage(ResourceUsage::CpuToGpu)
            .format(Format::R32G32B32_FLOAT)
            .name("SubMesh Vertices")
            .setInitialData(BufferDescription::InitialData(source.position)));

        key = path + "_norm";
        buffers.normals = device.createBufferSRV(
            tools::hash(
                reinterpret_cast<const uint8_t*>(key.c_str()),
                static_cast<unsigned int>(key.length())),
            BufferDescription()
            .usage(ResourceUsage::CpuToGpu)
            .format(Format::R32G32B32_FLOAT)
            .name("SubMesh Normals")
            .setInitialData(BufferDescription::InitialData(source.normal)));*/

        
        /*key = path + "_indicesAdj";
        if(device.cachedDataExists<BufferIBV>(tools::hash(
            reinterpret_cast<const uint8_t*>(key.c_str()),
            static_cast<unsigned int>(key.length()))))
        {
            buffers.indicesAdjacency = device.createBufferIBV(
                tools::hash(
                    reinterpret_cast<const uint8_t*>(key.c_str()),
                    static_cast<unsigned int>(key.length())),
                BufferDescription()
                .usage(ResourceUsage::CpuToGpu)
                .name("SubMesh Indices Adjacency"));
        }
        else
        {
            auto adj = meshGenerateAdjacency(source.indices, source.position);
            buffers.indicesAdjacency = device.createBufferIBV(
                tools::hash(
                    reinterpret_cast<const uint8_t*>(key.c_str()),
                    static_cast<unsigned int>(key.length())),
                BufferDescription()
                .usage(ResourceUsage::CpuToGpu)
                .name("SubMesh Indices Adjacency")
                .setInitialData(BufferDescription::InitialData(adj)));
        }*/
        
        return buffers;
    }
}

