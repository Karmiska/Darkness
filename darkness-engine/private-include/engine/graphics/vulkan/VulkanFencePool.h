#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "containers/vector.h"
#include <functional>

namespace engine
{
    namespace implementation
    {
        using CreateFence = std::function<VkSemaphore()>;
        using ResetFence = std::function<void(VkSemaphore)>;
        using DestroyFence = std::function<void(VkSemaphore)>;

        class FencePool
        {
        public:
            FencePool(
                CreateFence createFence,
                ResetFence resetFence,
                DestroyFence destroyFence);
            FencePool(FencePool&& pool);
            FencePool& operator=(FencePool&& pool);

            FencePool(const FencePool&) = delete;
            FencePool& operator=(const FencePool&) = delete;
            
            ~FencePool();

            VkSemaphore acquire();
            void release(VkSemaphore fence);

        private:
            CreateFence m_createFence;
            ResetFence m_resetFence;
            DestroyFence m_destroyFence;

            engine::vector<VkSemaphore> m_free;
        };
    }
}