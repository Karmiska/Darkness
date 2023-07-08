#include "engine/graphics/vulkan/VulkanGpuMarker.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanCommandList.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    namespace implementation
    {
        GpuMarkerImplVulkan::GpuMarkerImplVulkan(CommandList& cmd, const char* msg)
            : cmdList{ *static_cast<CommandListImplVulkan*>(cmd.native()) }
        {
            //VkDebugMarkerMarkerInfoEXT markerInfo = {};
            //markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            //
            //// Color to display this region with (if supported by debugger)
            //float color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
            //memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
            //
            //// Name of the region displayed by the debugging application
            //markerInfo.pMarkerName = msg;
            //CmdDebugMarkerBegin(cmdList.native(), &markerInfo);

            VkDebugUtilsLabelEXT label = {};
            label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label.pLabelName = msg;
            
            float color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
            memcpy(label.color, &color[0], sizeof(float) * 4);

            CmdBeginDebugUtilsLabelEXT(cmdList.native(), &label);
        }

        GpuMarkerImplVulkan::~GpuMarkerImplVulkan()
        {
            //CmdDebugMarkerEnd(cmdList.native());

            CmdEndDebugUtilsLabelEXT(cmdList.native());
        }
    }
}