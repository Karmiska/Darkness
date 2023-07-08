#pragma once

#include "containers/memory.h"
#include "engine/graphics/BarrierImplIf.h"
#include "engine/graphics/CommonNoDep.h"

namespace engine
{
    class CommandList;
    class Buffer;
    class TextureRTV;
    class Semaphore;

    enum class ResourceBarrierFlags
    {
        None,
        BeginOnly,
        EndOnly
    };

    const char* resourceStateToString(ResourceState state);

    static const unsigned int BarrierAllSubresources = 0xffffffff;

    class Barrier
    {
    public:
        void update(
            ResourceState before,
            ResourceState after);

        implementation::BarrierImplIf* native() { return m_impl.get(); }
    private:
        friend class CommandList;
        Barrier(
            const CommandList& commandList,
            ResourceBarrierFlags flags,
            const TextureRTV& resource,
            ResourceState before,
            ResourceState after,
            unsigned int subResource,
            const Semaphore& waitSemaphore,
            const Semaphore& signalSemaphore,
            GraphicsApi api
        );

        engine::unique_ptr<implementation::BarrierImplIf> m_impl;
    };
}
