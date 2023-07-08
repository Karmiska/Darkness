#pragma once

#include "engine/graphics/GpuMarkerImplIf.h"
#include "containers/string.h"

namespace engine
{
    class Device;
    class CommandList;

    namespace implementation
    {
        class CommandListImplVulkan;

        class GpuMarkerImplVulkan : public GpuMarkerImplIf
        {
        public:
            GpuMarkerImplVulkan(CommandList& cmd, const char* msg);
            ~GpuMarkerImplVulkan();

        private:
            CommandListImplVulkan& cmdList;
            uint32_t m_queryId;
        };
    }
}