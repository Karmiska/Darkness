#pragma once

namespace engine
{
    namespace implementation
    {
        class CpuMarkerImplVulkan
        {
        public:
            CpuMarkerImplVulkan(const char* msg);
            ~CpuMarkerImplVulkan();
        };
    }
}