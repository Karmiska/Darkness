#pragma once

#include "engine/graphics/SemaphoreImplIf.h"
#include "containers/memory.h"
#include "engine/graphics/Fence.h"

namespace engine
{
    class Device;
    enum class GraphicsApi;
    namespace implementation
    {
        class CommandListImpl;
    }

    class Semaphore
    {
    public:
        void reset();

        engine::FenceValue currentGPUValue() const;

        implementation::SemaphoreImplIf* native() { return m_impl.get(); }
        const implementation::SemaphoreImplIf* native() const { return m_impl.get(); }
    private:
        friend class Device;
        friend class CommandList;
        friend class implementation::CommandListImpl;
        Semaphore(const Device& device, GraphicsApi api);

        engine::unique_ptr<implementation::SemaphoreImplIf> m_impl;
    };
}
