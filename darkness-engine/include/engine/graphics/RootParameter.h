#pragma once

#include "engine/graphics/RootParameterImplIf.h"
#include "tools/Codegen.h"
#include "engine/graphics/CommonNoDep.h"
#include "containers/memory.h"

namespace engine
{
    enum class RootParameterType
    {
        DescriptorTable,
        Constants32bit,
        CBV, // ConstantBufferView
        SRV, // ShaderResourceView,
        UAV  // UnorderedAccessView
    };

    enum class DescriptorRangeType
    {
        SRV,
        UAV,
        CBV,
        Sampler
    };

    class Device;

    class RootParameter
    {
    public:
        RootParameter(const RootParameter&) = default;
        RootParameter(RootParameter&&) = default;
        RootParameter& operator=(const RootParameter&) = default;
        RootParameter& operator=(RootParameter&&) = default;

        RootParameter();
        RootParameter(GraphicsApi api);

        void binding(unsigned int index);
        unsigned int binding() const;

        void visibility(ShaderVisibility visibility);
        ShaderVisibility visibility() const;

        void initAsConstants(unsigned int reg, unsigned int num32BitValues, ShaderVisibility visibility);
        void initAsCBV(unsigned int reg, ShaderVisibility visibility);
        void initAsSRV(unsigned int reg, ShaderVisibility visibility);
        void initAsUAV(unsigned int reg, ShaderVisibility visibility);
        void initAsDescriptorRange(DescriptorRangeType type, unsigned int reg, unsigned int count, ShaderVisibility visibility);
        void initAsDescriptorTable(unsigned int rangeCount, ShaderVisibility visibility);
        void setTableRange(unsigned int rangeIndex, DescriptorRangeType type, unsigned int reg, unsigned int count, unsigned int space = 0);

        implementation::RootParameterImplIf* native() { return m_impl.get(); }

    private:
        engine::unique_ptr<implementation::RootParameterImplIf> m_impl;
    };
}
