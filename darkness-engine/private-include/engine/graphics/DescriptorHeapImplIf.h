#pragma once

namespace engine
{
    namespace implementation
    {
        class DescriptorHeapImplIf
        {
        public:
            virtual ~DescriptorHeapImplIf() {};

            virtual void reset() = 0;
        };
    }
}
