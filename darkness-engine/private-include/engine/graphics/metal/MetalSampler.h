#pragma once

#include "containers/memory.h"

namespace engine
{
    struct SamplerDescription;
    class Device;
    class DescriptorHandle;
    
    namespace implementation
    {
        class SamplerImpl
        {
        public:
            SamplerImpl(const Device& device, const DescriptorHandle& heap, const SamplerDescription& desc);
            SamplerImpl(const Device& device, const SamplerDescription& desc);
            ~SamplerImpl();
            
            SamplerImpl(const SamplerImpl&) = delete;
            SamplerImpl(SamplerImpl&&) = delete;
            SamplerImpl& operator=(const SamplerImpl&) = delete;
            SamplerImpl& operator=(SamplerImpl&&) = delete;
                        
        private:
            const Device& m_device;
            engine::unique_ptr<DescriptorHandle> m_descriptorHandle;
        };
    }
}

