#include "engine/graphics/vulkan/VulkanCpuMarker.h"

namespace engine
{
    namespace implementation
    {
        CpuMarkerImplVulkan::CpuMarkerImplVulkan(const char* /*msg*/)
        {
            //PIXBeginEvent(0, msg);
        }

        CpuMarkerImplVulkan::~CpuMarkerImplVulkan()
        {
            //PIXEndEvent();
        }
    }
}
