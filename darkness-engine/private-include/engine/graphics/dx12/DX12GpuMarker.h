#pragma once

#include "engine/graphics/GpuMarkerImplIf.h"
#include "containers/string.h"

namespace engine
{
    class Device;
    class CommandList;

    namespace implementation
    {
        class CommandListImplDX12;

        class GpuMarkerImplDX12 : public GpuMarkerImplIf
        {
        public:
            GpuMarkerImplDX12(CommandList& cmd, const char* msg);
            ~GpuMarkerImplDX12();

        private:
            CommandListImplDX12* cmdList;
            uint32_t m_queryId;
        };
    }
}