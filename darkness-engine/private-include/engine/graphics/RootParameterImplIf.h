#pragma once

#include <cstdint>

namespace engine
{
    using ShaderVisibility = std::uint32_t;
    enum class DescriptorRangeType;

    namespace implementation
    {
        class RootParameterImplIf
        {
        public:
            virtual ~RootParameterImplIf() {};

            virtual void binding(unsigned int index) = 0;
            virtual unsigned int binding() const = 0;

            virtual void visibility(ShaderVisibility visibility) = 0;
            virtual ShaderVisibility visibility() const = 0;

            virtual void initAsConstants(unsigned int reg, unsigned int num32BitValues, ShaderVisibility visibility) = 0;
            virtual void initAsCBV(unsigned int reg, ShaderVisibility visibility) = 0;
            virtual void initAsSRV(unsigned int reg, ShaderVisibility visibility) = 0;
            virtual void initAsUAV(unsigned int reg, ShaderVisibility visibility) = 0;
            virtual void initAsDescriptorRange(DescriptorRangeType type, unsigned int reg, unsigned int count, ShaderVisibility visibility) = 0;
            virtual void initAsDescriptorTable(unsigned int rangeCount, ShaderVisibility visibility) = 0;
            virtual void setTableRange(unsigned int rangeIndex, DescriptorRangeType type, unsigned int reg, unsigned int count, unsigned int space) = 0;
        };
    }
}