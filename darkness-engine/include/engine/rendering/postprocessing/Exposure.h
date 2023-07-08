#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/ResourceOwners.h"

namespace engine
{
    class CommandList;
    class TextureSRV;
    class Exposure
    {
    public:
        Exposure(Device& device);

        BufferSRV exposureSRV();
        BufferUAV exposureUAV();

    private:
        Device& m_device;

        BufferUAVOwner m_exposureUAV;
        BufferSRVOwner m_exposureSRV;
    };
}
