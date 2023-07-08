#pragma once

#include "engine/graphics/DescriptorHandle.h"
#include "containers/vector.h"

struct MetalDescriptorHeap;
enum class DescriptorHeapFlags;

namespace engine
{
    class Device;
    class DescriptorHandle;
    class RootSignature;
    class Buffer;
    class Sampler;
    class BufferView;
    class TextureSRV;
    enum class DescriptorType;
    enum class DescriptorHeapFlags;
    namespace implementation
    {
        class DescriptorHeapImpl
        {
        public:
            DescriptorHeapImpl(
                               const Device& device,
                               const engine::vector<DescriptorType>& type,
                               DescriptorHeapFlags flags,
                               int maxNumDescriptors = 1);
            
            DescriptorType type() const;
            DescriptorHandle allocate(
                                      const RootSignature& signature,
                                      const Buffer& buffer,
                                      const TextureSRV& view,
                                      const Sampler& sampler,
                                      int count);
            DescriptorHandle cpuDescriptorHandleForHeapStart();
        private:
            const Device& m_device;
            const engine::vector<DescriptorType>& m_type;
            int m_handlesAvailable;
            
            MetalDescriptorHeap* m_heap;
  
            /*DescriptorHandle m_currentHandle;
            
            const Device& m_device;
            MetalDescriptorHeap* m_heap;
            DescriptorType m_type;
            int m_handlesAvailable;*/
        };
    }
}
