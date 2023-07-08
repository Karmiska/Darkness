#include "engine/graphics/metal/MetalDescriptorHeap.h"
#include "engine/graphics/metal/MetalDescriptorHandle.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/metal/MetalDevice.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/DescriptorHeap.h"
#include "engine/graphics/DescriptorHandle.h"
#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        DescriptorHeapImpl::DescriptorHeapImpl(
                                               const Device& device,
                                               const std::vector<DescriptorType>& type,
                                               DescriptorHeapFlags /*flags*/,
                                               int maxNumDescriptors)
        : m_device{ device }
        , m_type{ type }
        , m_handlesAvailable{ maxNumDescriptors }
        , m_heap{ nullptr }
        {
            /*D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
            // Initialize the render target view heap description for the two back buffers.
            memset(&renderTargetViewHeapDesc, 0, sizeof(renderTargetViewHeapDesc));

            // Set the number of descriptors to two for our two back buffers.  
            // Also set the heap tyupe to render target views.
            renderTargetViewHeapDesc.NumDescriptors = maxNumDescriptors;
            renderTargetViewHeapDesc.Type = dxDescriptorType(type);
            renderTargetViewHeapDesc.Flags = dxDescriptorHeapFlags(type);

            // Create the render target view heap for the back buffers.
            ASSERT(SUCCEEDED(DeviceImplGet::impl(device).device()->CreateDescriptorHeap(
                &renderTargetViewHeapDesc, 
                __uuidof(ID3D12DescriptorHeap), 
                (void**)&m_heap)));*/
        }
        
        DescriptorHandle DescriptorHeapImpl::allocate(
                                                      const RootSignature& signature,
                                                      const Buffer& buffer,
                                                      const TextureSRV& view,
                                                      const Sampler& sampler,
                                                      int count)
        {
            ASSERT(count < m_handlesAvailable);
            
            /*DescriptorHandle handle = m_currentHandle;
            m_currentHandle += count;
            m_handlesAvailable -= count;
            return handle;*/
            return {};
        }

        DescriptorType DescriptorHeapImpl::type() const
        {
            //return m_type;
            return {};
        }

        DescriptorHandle DescriptorHeapImpl::cpuDescriptorHandleForHeapStart()
        {
            /*D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
            unsigned int incrementSize = DeviceImplGet::impl(m_device).device()->GetDescriptorHandleIncrementSize(dxDescriptorType(m_type));
            return DescriptorHandle(tools::make_unique_impl<DescriptorHandleImpl>(renderTargetViewHandle, incrementSize));*/
            
            /*METAL_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
            return DescriptorHandle(tools::make_unique_impl<DescriptorHandleImpl>(renderTargetViewHandle, 0));*/
            return {};
        }
    }
}
