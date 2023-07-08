#include "engine/rendering/Picker.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    Picker::Picker(Device& device)
        : m_device{ device }
        , m_pickPipeline{ device.createPipeline<shaders::Pick>() }
        , m_pickUAV{ device.createBufferUAV(BufferDescription()
            .elements(1)
            .format(Format::R32_UINT)
            .usage(ResourceUsage::GpuReadWrite)
            .name("Pick buffer")
        ) }
        , m_pickReadBack{ device.createBufferSRV(BufferDescription()
            .elements(1)
            .elementSize(sizeof(unsigned int))
            .format(Format::R32_UINT)
            .usage(ResourceUsage::GpuToCpu)
            .name("Pick buffer readback")
        ) }
    {}

    void Picker::pick(
        GBuffer* gBuffer,
        CommandList& cmd,
        uint32_t mouseX,
        uint32_t mouseY)
    {

        {
            CPU_MARKER(cmd.api(), "Pick objects");
            GPU_MARKER(cmd, "Pick objects");
            m_pickPipeline.cs.mousePosition.x = mouseX;
            m_pickPipeline.cs.mousePosition.y = mouseY;
            m_pickPipeline.cs.instanceId = gBuffer->srv(GBufferType::InstanceId);
            m_pickPipeline.cs.output = m_pickUAV;
            cmd.bindPipe(m_pickPipeline);
            cmd.dispatch(1, 1, 1);
        }

        {
            CPU_MARKER(cmd.api(), "Pick buffer update");
            GPU_MARKER(cmd, "Pick buffer update");
            cmd.copyBuffer(m_pickUAV.resource().buffer(), m_pickReadBack.resource().buffer(), 1);
        }
    }

    uint32_t Picker::selectedInstanceId(Device& device)
    {
        unsigned int res = 0;
        uint32_t* bufferPtr = reinterpret_cast<uint32_t*>(m_pickReadBack.resource().buffer().map(device));
        res = *bufferPtr;
        m_pickReadBack.resource().buffer().unmap(device);
        return res;
    }
}
