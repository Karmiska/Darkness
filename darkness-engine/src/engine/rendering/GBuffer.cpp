#include "engine/rendering/GBuffer.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    GBuffer::GBuffer(
        Device& device,
        int width,
        int height)
        : m_device{ device }
        , m_desc{ TextureDescription()
            .width(width)
            .height(height)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .dimension(ResourceDimension::Texture2D)
            .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 0.0f })
        }

        , m_normalRTV{ device.createTextureRTV(TextureDescription(m_desc)
            .format(Format::R16G16_FLOAT)
            .name("GBuffer normal")) }
        , m_normalSRV{ device.createTextureSRV(m_normalRTV) }

        , m_uvRTV{ device.createTextureRTV(TextureDescription(m_desc)
            .format(Format::R16G16_UINT)
            .name("GBuffer uv buffer")) }
        , m_uvSRV{ device.createTextureSRV(m_uvRTV) }

        , m_motionRTV{ device.createTextureRTV(TextureDescription(m_desc)
            .format(Format::R16G16_FLOAT)
            .name("GBuffer motion buffer")) }
        , m_motionSRV{ device.createTextureSRV(m_motionRTV) }

        , m_instanceIdRTV{ device.createTextureRTV(TextureDescription(m_desc)
            .format(Format::R32_UINT)
            .name("GBuffer object id")) }
        , m_instanceIdSRV{ device.createTextureSRV(m_instanceIdRTV) }

    {}

    TextureRTV GBuffer::rtv(GBufferType type)
    {
        switch (type)
        {
        case GBufferType::Normal: return m_normalRTV;
        case GBufferType::Uv: return m_uvRTV;
        case GBufferType::Motion: return m_motionRTV;
        case GBufferType::InstanceId: return m_instanceIdRTV;
        }
        return m_normalRTV;
    }

    TextureSRV GBuffer::srv(GBufferType type)
    {
        switch (type)
        {
        case GBufferType::Normal: return m_normalSRV;
        case GBufferType::Uv: return m_uvSRV;
        case GBufferType::Motion: return m_motionSRV;
        case GBufferType::InstanceId: return m_instanceIdSRV;
        }
        return m_normalSRV;
    }

    void GBuffer::clear(CommandList& cmd)
    {
        GPU_MARKER(cmd, "Clear GBuffer");
        cmd.clearRenderTargetView(m_normalRTV, { 0.0f, 0.0f, 0.0f, 0.0f });
        cmd.clearRenderTargetView(m_uvRTV, { 0.0f, 0.0f, 0.0f, 0.0f });
        cmd.clearRenderTargetView(m_motionRTV, { 0.0f, 0.0f, 0.0f, 0.0f });
        cmd.clearRenderTargetView(m_instanceIdRTV, { 0.0f, 0.0f, 0.0f, 0.0f });
    }
}
