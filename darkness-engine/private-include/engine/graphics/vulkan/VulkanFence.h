#pragma once

#include "engine/graphics/FenceImplIf.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/Fence.h"
#include "containers/queue.h"

namespace engine
{
    namespace implementation
    {
        class DeviceImplIf;
        class DeviceImplVulkan;
        class FenceImplVulkan : public FenceImplIf
        {
        public:
            FenceImplVulkan(const DeviceImplIf* device, const char* name);
            ~FenceImplVulkan();

            FenceImplVulkan(const FenceImplVulkan&) = delete;
            FenceImplVulkan(FenceImplVulkan&&) = delete;
            FenceImplVulkan& operator=(const FenceImplVulkan&) = delete;
            FenceImplVulkan& operator=(FenceImplVulkan&&) = delete;

            VkSemaphore& native();
            const VkSemaphore& native() const;

            void increaseCPUValue() override;
            engine::FenceValue currentCPUValue() const override;
            engine::FenceValue currentGPUValue() const override;

            void blockUntilSignaled() override;
            void blockUntilSignaled(engine::FenceValue value) override;

            bool signaled() const override;
            bool signaled(engine::FenceValue value) const override;

            void reset() override;

        private:
            VkSemaphore m_fence;
            const DeviceImplVulkan* m_device;
            void* m_fenceEvent;
            FenceValue m_fenceValue;
        };
    }
}

