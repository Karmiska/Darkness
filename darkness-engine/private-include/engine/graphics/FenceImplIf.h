#pragma once

namespace engine
{
    using FenceValue = unsigned long long;

    namespace implementation
    {
        class FenceImplIf
        {
        public:
            virtual ~FenceImplIf() {};

            virtual void increaseCPUValue() = 0;
            virtual FenceValue currentCPUValue() const = 0;
            virtual FenceValue currentGPUValue() const = 0;

            virtual void blockUntilSignaled() = 0;
            virtual void blockUntilSignaled(FenceValue value) = 0;

            virtual bool signaled() const = 0;
            virtual bool signaled(FenceValue value) const = 0;

            virtual void reset() = 0;
        };
    }
}
