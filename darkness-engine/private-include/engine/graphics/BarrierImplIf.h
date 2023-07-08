#pragma once

namespace engine
{
    enum class ResourceState;

    namespace implementation
    {
        class BarrierImplIf
        {
        public:
            virtual ~BarrierImplIf() {};

            virtual void update(
                ResourceState before,
                ResourceState after) = 0;
        };
    }
}

