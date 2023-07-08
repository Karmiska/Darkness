#pragma once

//struct ID3D12CommandAllocator;

namespace engine
{
    class Device;

    namespace implementation
    {
        class CommandAllocatorImpl
        {
        public:
            CommandAllocatorImpl(const Device& device);
            ~CommandAllocatorImpl();

            CommandAllocatorImpl(const CommandAllocatorImpl&) = delete;
            CommandAllocatorImpl(CommandAllocatorImpl&&) = delete;
            CommandAllocatorImpl& operator=(const CommandAllocatorImpl&) = delete;
            CommandAllocatorImpl& operator=(CommandAllocatorImpl&&) = delete;

            void reset();
            void* native() const;
        private:
            void* m_commandAllocator;
        };
    }
}

