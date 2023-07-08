#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "containers/vector.h"
#include "engine/primitives/Vector3.h"

namespace engine
{
    namespace implementation
    {
        struct VulkanQueue
        {
            int familyIndex;
            int available;
            VkQueueFlags flags;
            bool presentSupport;
            engine::vector<float> priority;
            engine::vector<bool> taken;
            engine::Vector3<size_t> minImageTransferGranularity;
        };
    }
}
