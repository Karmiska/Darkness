#pragma once

#include "engine/graphics/ResourceOwners.h"

namespace engine
{
    class Device;
    class CommandList;
    enum class GBufferType
    {
        Normal,
        Uv,
        Motion,
        InstanceId
    };

    class GBuffer
    {
    public:
        GBuffer(
            Device& device,
            int width,
            int height);

        TextureRTV rtv(GBufferType type);
        TextureSRV srv(GBufferType type);

        void clear(CommandList& cmd);
    private:
        Device& m_device;
        TextureDescription m_desc;

        TextureRTVOwner m_normalRTV;
        TextureSRVOwner m_normalSRV;

        TextureRTVOwner m_uvRTV;
        TextureSRVOwner m_uvSRV;

        TextureRTVOwner m_motionRTV;
        TextureSRVOwner m_motionSRV;

        TextureRTVOwner m_instanceIdRTV;
        TextureSRVOwner m_instanceIdSRV;
    };
}