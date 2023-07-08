#include "engine/graphics/vulkan/VulkanFencePool.h"

namespace engine
{
    namespace implementation
    {
        FencePool::FencePool(
            CreateFence createFence,
            ResetFence resetFence,
            DestroyFence destroyFence)
            : m_createFence{ createFence }
            , m_resetFence{ resetFence }
            , m_destroyFence{ destroyFence }
        {}

        FencePool::FencePool(FencePool&& pool)
        {
            std::swap(pool.m_free, m_free);
        }

        FencePool& FencePool::operator=(FencePool&& pool)
        {
            std::swap(pool.m_free, m_free);
            return *this;
        }

        FencePool::~FencePool()
        {
            for (auto&& fence : m_free)
            {
                m_destroyFence(fence);
            }
            m_free.clear();
        }

        VkSemaphore FencePool::acquire()
        {
            VkSemaphore fence;
            if (m_free.size() > 0)
            {
                fence = m_free.back();
                m_free.pop_back();
            }
            else
            {
                fence = m_createFence();
            }
            return fence;
        }

        void FencePool::release(VkSemaphore fence)
        {
            m_resetFence(fence);
            m_free.emplace_back(std::move(fence));
        }

    }
}