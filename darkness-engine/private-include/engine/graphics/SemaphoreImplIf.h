#pragma once

namespace engine
{
    namespace implementation
    {
        class SemaphoreImplIf
        {
        public:
            virtual ~SemaphoreImplIf() {};
            virtual void reset() = 0;
            virtual bool signaled() const = 0;
        };
    }
}