#pragma once

#include "engine/graphics/DescriptorHeapImplIf.h"
#include "containers/vector.h"
#include "VulkanHeaders.h"

namespace engine
{
    class RootSignature;
    class Buffer;
    class Sampler;
    class BufferView;
    class TextureSRV;
    enum class DescriptorHeapFlags;
    namespace shaders
    {
        class PipelineConfiguration;
    }

    namespace implementation
    {
        class DeviceImplVulkan;
        class DescriptorHandleImplVulkan;
        enum class DescriptorType;

        class DescriptorHeapImplVulkan : public DescriptorHeapImplIf
        {
        public:
            DescriptorHeapImplVulkan(const DeviceImplVulkan& device);

            //DescriptorType type() const;
            engine::shared_ptr<DescriptorHandleImplVulkan> allocate(
                const RootSignature& signature,
                const shaders::PipelineConfiguration& pipelineConfig
                /*, 
                const Buffer& buffer, 
                const TextureSRV& view,
                const Sampler& sampler,
                int count*/
            );

            VkDescriptorPool& native();
            void reset() {};
        private:
            const DeviceImplVulkan& m_device;

            engine::shared_ptr<VkDescriptorPool> m_heap;
        };
    }
}
