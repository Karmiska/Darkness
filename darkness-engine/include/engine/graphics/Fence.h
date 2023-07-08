#pragma once

#include "engine/graphics/FenceImplIf.h"
#include "containers/memory.h"

namespace engine
{
    namespace implementation
    {
        class DeviceImpl;
    }
    enum class GraphicsApi;
    class Device;

    class Fence
    {
    public:
        void increaseCPUValue();
        FenceValue currentCPUValue() const;
        FenceValue currentGPUValue() const;

        void blockUntilSignaled();
        void blockUntilSignaled(FenceValue value);

        bool signaled() const;
        bool signaled(FenceValue value) const;

        void reset();

        implementation::FenceImplIf* native() { return m_impl.get(); };
        const implementation::FenceImplIf* native() const { return m_impl.get(); };
    private:
        friend class Device;
        friend class implementation::DeviceImpl;
        Fence(const Device& device, const char* name, GraphicsApi api);

        engine::unique_ptr<implementation::FenceImplIf> m_impl;
    };
}
