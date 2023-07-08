#pragma once

#include "engine/graphics/RootParameterImplIf.h"
#include "VulkanHeaders.h"
#include "engine/graphics/CommonNoDep.h"
#include "tools/SmartPimpl.h"
#include "tools/Codegen.h"

namespace engine
{
    enum class DescriptorRangeType;
    class Device;
    
    namespace implementation
    {
        enum class DescriptorType;

        class RootParameterImplVulkan : public RootParameterImplIf
        {
        public:
            RootParameterImplVulkan();

            void binding(unsigned int index);
            unsigned int binding() const;

            void visibility(ShaderVisibility visibility);
            ShaderVisibility visibility() const;

            void descriptorType(DescriptorType type);
            DescriptorType descriptorType() const;
            
            void initAsConstants(unsigned int reg, unsigned int num32BitValues, ShaderVisibility visibility);
            void initAsCBV(unsigned int reg, ShaderVisibility visibility);
            void initAsSRV(unsigned int reg, ShaderVisibility visibility);
            void initAsUAV(unsigned int reg, ShaderVisibility visibility);
            void initAsDescriptorRange(DescriptorRangeType type, unsigned int reg, unsigned int count, ShaderVisibility visibility);
            void initAsDescriptorTable(unsigned int rangeCount, ShaderVisibility visibility);
            void setTableRange(unsigned int rangeIndex, DescriptorRangeType type, unsigned int reg, unsigned int count, unsigned int space = 0);

            const VkDescriptorSetLayoutBinding& layoutBinding() const;
            VkDescriptorSetLayoutBinding& layoutBinding();
        private:
            VkDescriptorSetLayoutBinding m_layoutBinding;
        };
    }
}

