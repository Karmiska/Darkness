#pragma once

struct MetalFence{};

namespace engine
{
    namespace implementation
    {
        class DeviceImpl;
        class FenceImpl
        {
        public:
            FenceImpl(const DeviceImpl& device);
            ~FenceImpl();
            
            FenceImpl(const FenceImpl&) = delete;
            FenceImpl(FenceImpl&&) = delete;
            FenceImpl& operator=(const FenceImpl&) = delete;
            FenceImpl& operator=(FenceImpl&&) = delete;
            
            MetalFence& native();
            const MetalFence& native() const;
            
            void reset();
            bool signaled() const;
            void blockUntilSignaled() const;
        private:
            const DeviceImpl& m_device;
            MetalFence m_fence;
        };
    }
}

