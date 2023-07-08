#pragma once

namespace engine
{
    enum class CommandListType;

    namespace implementation
    {
        class CommandAllocatorImplIf
        {
        public:
            virtual ~CommandAllocatorImplIf() {};

            virtual void reset() = 0;
            virtual CommandListType type() const = 0;
        };
    }
}
