#include "engine/rendering/RenderOutline.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/CommonNoDep.h"
#include "tools/Debug.h"
#include "tools/Measure.h"
#include "tools/MeshTools.h"
#include "shaders/core/shared_types/VertexScale.hlsli"
#include <climits>

using namespace tools;

namespace engine
{
    RenderOutline::RenderOutline(
        Device& device,
        Vector2<int> virtualResolution)
        : m_outlinePipeline{ device.createPipeline<shaders::Outline>() }
        , m_createOutlineIndirectIndexedDrawArgs{ device.createPipeline<shaders::CreateOutlineIndirectIndexedDrawArgs>() }
        , m_virtualResolution{ virtualResolution }
        , m_depthSampler{ device.createSampler(SamplerDescription().filter(Filter::Point)) }
        , m_indexedDrawArguments{
            device.createBuffer(BufferDescription()
                .elementSize(sizeof(DrawIndexIndirectArgs))
                .elements(1)
                .usage(ResourceUsage::GpuReadWrite)
                .structured(true)
				.indirectArgument(true)
                .name("Occlusion culling dispatch args (DispatchIndirectArgs)")
            ) }
        , m_indexedDrawArgumentsUAV{
            device.createBufferUAV(m_indexedDrawArguments) }
    {
        m_outlinePipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList, true);
        m_outlinePipeline.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
        m_outlinePipeline.setDepthStencilState(DepthStencilDescription().depthEnable(false));
        m_outlinePipeline.ps.depth_sampler = m_depthSampler;
    }

    void RenderOutline::render(
        Device& device,
        TextureRTV currentRenderTarget,
        TextureSRV depthBuffer,
        Camera& camera,
        ModelResources& modelResources,
        CommandList& cmd,
        FlatSceneNode& model)
    {
        return;
        auto instance = model.mesh->meshBuffer().modelAllocations->subMeshInstance->instanceData.modelResource.gpuIndex;
        m_createOutlineIndirectIndexedDrawArgs.cs.instanceId.x = static_cast<uint32_t>(instance);
        m_createOutlineIndirectIndexedDrawArgs.cs.lodBinding = modelResources.gpuBuffers().lod();
        m_createOutlineIndirectIndexedDrawArgs.cs.instanceLodBinding = modelResources.gpuBuffers().instanceLodBinding();
        m_createOutlineIndirectIndexedDrawArgs.cs.subMeshAdjacency = modelResources.gpuBuffers().subMeshAdjacency();
        m_createOutlineIndirectIndexedDrawArgs.cs.indexedDrawArguments = m_indexedDrawArgumentsUAV;
        cmd.bindPipe(m_createOutlineIndirectIndexedDrawArgs);
        cmd.dispatch(1, 1, 1);

        // draw outline
        auto viewMatrix = camera.viewMatrix();
        auto cameraProjectionMatrix = camera.projectionMatrix(m_virtualResolution);
        auto jitterMatrix = camera.jitterMatrix(device.frameNumber(), m_virtualResolution);

        cmd.setRenderTargets({ currentRenderTarget });
        m_outlinePipeline.ps.color = Float4{ 0.0f, 1.0f, 0.0f, 1.0f };

        const auto& transformMatrix = model.transform;
        
        m_outlinePipeline.gs.modelMatrix = fromMatrix(transformMatrix);
        m_outlinePipeline.gs.modelViewMatrix = fromMatrix(viewMatrix * transformMatrix);
        m_outlinePipeline.gs.jitterProjectionMatrix = fromMatrix(jitterMatrix * cameraProjectionMatrix);

        m_outlinePipeline.gs.jitterModelViewProjectionMatrix = fromMatrix(jitterMatrix * (cameraProjectionMatrix * (viewMatrix * transformMatrix)));
        m_outlinePipeline.gs.cameraWorldSpacePosition = camera.position();
        m_outlinePipeline.gs.lineThickness = 2.0f;
        m_outlinePipeline.gs.texelSize = Float2{ 
            1.0f / static_cast<float>(camera.width()),
            1.0f / static_cast<float>(camera.height()) };

        m_outlinePipeline.ps.depth = depthBuffer;
        m_outlinePipeline.ps.inverseSize = Vector2f(
            1.0f / static_cast<float>(camera.width()),
            1.0f / static_cast<float>(camera.height()));

        m_outlinePipeline.vs.vertices = modelResources.gpuBuffers().vertex();
        m_outlinePipeline.vs.normals = modelResources.gpuBuffers().normal();
        m_outlinePipeline.vs.scales = modelResources.gpuBuffers().instanceScale();
        m_outlinePipeline.vs.instanceId.x = static_cast<uint32_t>(instance);
        cmd.bindPipe(m_outlinePipeline);
        cmd.drawIndexedIndirect(modelResources.gpuBuffers().adjacency(), m_indexedDrawArguments, 0);
    }

}
