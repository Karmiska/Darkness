#pragma once

#include "engine/graphics/RootSignatureImplIf.h"
#include "VulkanHeaders.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/RootSignature.h"
#include "containers/memory.h"
#include "containers/vector.h"

namespace engine
{
    class Device;
    namespace implementation
    {
        class RootSignatureImplVulkan : public RootSignatureImplIf
        {
        public:
            RootSignatureImplVulkan(const Device& device, int rootParameterCount = 0, int staticSamplerCount = 0);
            
            void reset(int rootParameterCount, int staticSamplerCount) override;
            void initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility) override;
            void finalize(RootSignatureFlags flags = RootSignatureFlags::None) override;
            void enableNullDescriptors(bool texture, bool writeable) override;
            size_t rootParameterCount() const override;
            RootParameter& operator[](size_t index) override;
            const RootParameter& operator[](size_t index) const override;
            
            RootSignatureImplVulkan(const RootSignatureImplVulkan&) = delete;
            RootSignatureImplVulkan(RootSignatureImplVulkan&&) = delete;
            RootSignatureImplVulkan& operator=(const RootSignatureImplVulkan&) = delete;
            RootSignatureImplVulkan& operator=(RootSignatureImplVulkan&&) = delete;
            
            VkDescriptorSetLayout& layout();
            const VkDescriptorSetLayout& layout() const;

            void setNextVisibility(ShaderVisibility visibility) { m_visibility = visibility; };
        private:
            const Device& m_device;
            engine::shared_ptr<VkDescriptorPool> m_bindlessDescriptorHeap;

            engine::vector<RootParameter> m_parameters;
            engine::unique_ptr<StaticSamplerDescription[]> m_samplers;
            engine::shared_ptr<VkDescriptorSetLayout> m_descriptorSetLayout;
            
            int m_parameterCount;
            int m_samplerCount;
            size_t m_numInitializedStaticSamplers;
            
            uint32_t m_descriptorTableBitMap;
            uint32_t m_descriptorTableSize[16];
            unsigned int m_maxDescriptorCacheHandleCount;

            bool m_enableNullDescriptors;
            bool m_texture;
            bool m_writeable;
            ShaderVisibility m_visibility;
        };
    }
}

