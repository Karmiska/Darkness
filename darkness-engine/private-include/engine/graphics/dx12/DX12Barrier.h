#pragma once

#include "containers/memory.h"
#include "engine/graphics/BarrierImplIf.h"

struct D3D12_RESOURCE_BARRIER;

namespace engine
{
    class CommandList;
    class Buffer;
    class TextureRTV;
    class Semaphore;
    enum class ResourceBarrierFlags;
    enum class ResourceState;

    namespace implementation
    {
        class BarrierImplDX12 : public BarrierImplIf
        {
        public:
            BarrierImplDX12(
                const CommandList& commandList,
                ResourceBarrierFlags flags,
                const TextureRTV& resource,
                ResourceState before,
                ResourceState after,
                unsigned int subResource,
                const Semaphore& waitSemaphore,
                const Semaphore& signalSemaphore);
            ~BarrierImplDX12();

            BarrierImplDX12(const BarrierImplDX12&) = delete;
            BarrierImplDX12(BarrierImplDX12&&) = delete;
            BarrierImplDX12& operator=(const BarrierImplDX12&) = delete;
            BarrierImplDX12& operator=(BarrierImplDX12&&) = delete;

            void update(
                ResourceState before,
                ResourceState after) override;

            D3D12_RESOURCE_BARRIER& native();
        private:
            engine::unique_ptr<D3D12_RESOURCE_BARRIER> m_barrier;
            const CommandList& m_commandList;
        };
    }
}

