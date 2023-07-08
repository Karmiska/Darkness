#include "engine/rendering/culling/IndexExpansion.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/CommandList.h"
#include "engine/rendering/BufferSettings.h"

namespace engine
{
    #include "shaders/core/shared_types/ClusterExecuteIndirect.hlsli"

    IndexExpansion::IndexExpansion(
        Device& device)
        : m_gpuBuffers{ device.modelResources().gpuBuffers() }
		, m_indexExpand{ device.createPipeline<shaders::ClustersToIndexExpand>() }
        , m_createArguments{ device.createPipeline<shaders::ClustersToIndexExpandCreateArguments>() }

        , m_expandDispatchArgs{ device.createBuffer(BufferDescription()
            .elementSize(sizeof(ClusterExecuteIndirectArgs))
            .elements(1000)
            .usage(ResourceUsage::GpuReadWrite)
            .structured(true)
			.indirectArgument(true)
            .name("ClusterLine execute indirect draw args")
        ) }
        , m_expandDispatchArgsUAV{ device.createBufferUAV(m_expandDispatchArgs) }

        , m_bufferMath{ device }
    {
        
        m_createArguments.cs.expandDispatchArgs = m_expandDispatchArgsUAV;
    }

    void IndexExpansion::expandIndexes(
        Device& device,
        CommandList& cmd,
        ClusterDataLine& input,
        IndexDataLine& indexAppendOutput,
        IndexDataLine& indexDrawOutput,
        DrawDataLine& drawOutput,
		uint32_t clusterSize)
    {
#if 0
        {
            CPU_MARKER(cmd.api(), "Create indirect cluster draw arguments");
            GPU_MARKER(cmd, "Create indirect cluster draw arguments");

            cmd.copyBuffer(indexAppendOutput.indexCount.buffer, indexDrawOutput.indexIndexUAV.resource().buffer(), 1);
            cmd.clearBuffer(drawOutput.clusterRendererExecuteIndirectCount.uav);

            // input
            m_indexExpand.cs.inputClusterCount = input.clusterCount.srv;

            // output
            m_indexExpand.cs.outputIndexCount = indexAppendOutput.indexCount.uav;

            // draw arguments output
            m_indexExpand.cs.clusterRendererExecuteIndirectArguments = drawOutput.clusterRendererExecuteIndirectArgumentsBufferUAV;
            m_indexExpand.cs.clusterRendererExecuteIndirectCountBuffer = drawOutput.clusterRendererExecuteIndirectCount.uav;

            // constants
			m_indexExpand.cs.clusterSize.x = clusterSize;

            cmd.bindPipe(m_indexExpand);
            cmd.dispatch(1, 1, 1);

            m_bufferMath.perform(
                cmd,
                indexAppendOutput.indexCount.srv,
                indexDrawOutput.indexIndexSRV,
                indexDrawOutput.indexCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

            cmd.copyBuffer(
                indexAppendOutput.indexCount.buffer,
                indexAppendOutput.indexIndexUAV.resource().buffer(), 1);
        }
#else
        CPU_MARKER(cmd.api(), "2. Expand indexes");
        GPU_MARKER(cmd, "2. Expand indexes");
        {
            CPU_MARKER(cmd.api(), "Create expand clusters to indexes dispatch args");
            GPU_MARKER(cmd, "Create expand clusters to indexes dispatch args");

            m_createArguments.cs.clusterCountBuffer = input.clusterCount.srv;
            cmd.bindPipe(m_createArguments);
            cmd.dispatch(1, 1, 1);
        }

        {
            CPU_MARKER(cmd.api(), "Expand clusters to indexes");
            GPU_MARKER(cmd, "Expand clusters to indexes");

            indexDrawOutput.indexBuffer = indexAppendOutput.indexBuffer;
            indexDrawOutput.indexBufferUAV = indexAppendOutput.indexBufferUAV;
            cmd.copyBuffer(indexAppendOutput.indexCount.buffer, indexDrawOutput.indexIndexUAV.resource().buffer(), 1);

            cmd.clearBuffer(drawOutput.clusterRendererExecuteIndirectCount.uav);

            // input
            m_indexExpand.cs.inputClusters = input.clustersSRV;
            m_indexExpand.cs.inputClusterCount = input.clusterCount.srv;
            m_indexExpand.cs.inputClustersIndex = input.clusterIndexSRV;

            m_indexExpand.cs.clusterData = device.modelResources().gpuBuffers().clusterBinding();
            m_indexExpand.cs.baseIndexes = device.modelResources().gpuBuffers().index();

            // output
            m_indexExpand.cs.outputIndexes = indexAppendOutput.indexBufferUAV;
            m_indexExpand.cs.outputIndexCount = indexAppendOutput.indexCount.uav;
            m_indexExpand.cs.outputIndexIndex = indexAppendOutput.indexIndexSRV;

            // draw arguments output
            m_indexExpand.cs.clusterRendererExecuteIndirectArguments = drawOutput.clusterRendererExecuteIndirectArgumentsBufferUAV;
            m_indexExpand.cs.clusterRendererExecuteIndirectCountBuffer = drawOutput.clusterRendererExecuteIndirectCount.uav;

            // constants
            m_indexExpand.cs.hack.x = 12582912u;

            cmd.bindPipe(m_indexExpand);
            cmd.dispatchIndirect(m_expandDispatchArgs, 0);

            m_bufferMath.perform(
                cmd,
                indexAppendOutput.indexCount.srv,
                indexDrawOutput.indexIndexSRV,
                indexDrawOutput.indexCount.uav,
                BufferMathOperation::Subtraction, 1, 0, 0, 0);

            cmd.copyBuffer(
                indexAppendOutput.indexCount.buffer,
                indexAppendOutput.indexIndexUAV.resource().buffer(), 1);
        }
#endif
    }
}
