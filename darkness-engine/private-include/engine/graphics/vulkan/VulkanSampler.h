#pragma once

#include "engine/graphics/SamplerImplIf.h"
#include "VulkanHeaders.h"

static const char* SamplerDebugNames[] = {
            "Sampler_0", "Sampler_1", "Sampler_2", "Sampler_3", "Sampler_4", "Sampler_5", "Sampler_6", "Sampler_7", "Sampler_8", "Sampler_9",
            "Sampler_10", "Sampler_11", "Sampler_12", "Sampler_13", "Sampler_14", "Sampler_15", "Sampler_16", "Sampler_17", "Sampler_18", "Sampler_19",
            "Sampler_20", "Sampler_21", "Sampler_22", "Sampler_23", "Sampler_24", "Sampler_25", "Sampler_26", "Sampler_27", "Sampler_28", "Sampler_29",
            "Sampler_30", "Sampler_31", "Sampler_32", "Sampler_33", "Sampler_34", "Sampler_35", "Sampler_36", "Sampler_37", "Sampler_38", "Sampler_39",
            "Sampler_40", "Sampler_41", "Sampler_42", "Sampler_43", "Sampler_44", "Sampler_45", "Sampler_46", "Sampler_47", "Sampler_48", "Sampler_49",
            "Sampler_50", "Sampler_51", "Sampler_52", "Sampler_53", "Sampler_54", "Sampler_55", "Sampler_56", "Sampler_57", "Sampler_58", "Sampler_59",
            "Sampler_60", "Sampler_61", "Sampler_62", "Sampler_63", "Sampler_64", "Sampler_65", "Sampler_66", "Sampler_67", "Sampler_68", "Sampler_69",
            "Sampler_70", "Sampler_71", "Sampler_72", "Sampler_73", "Sampler_74", "Sampler_75", "Sampler_76", "Sampler_77", "Sampler_78", "Sampler_79",
            "Sampler_80", "Sampler_81", "Sampler_82", "Sampler_83", "Sampler_84", "Sampler_85", "Sampler_86", "Sampler_87", "Sampler_88", "Sampler_89",
            "Sampler_90", "Sampler_91", "Sampler_92", "Sampler_93", "Sampler_94", "Sampler_95", "Sampler_96", "Sampler_97", "Sampler_98", "Sampler_99"
};

extern int SamplerCount;

namespace engine
{
    struct SamplerDescription;
    class Device;
    class DescriptorHandle;
    
    namespace implementation
    {
        class SamplerImplVulkan : public SamplerImplIf
        {
        public:
            SamplerImplVulkan(
                const Device& device, 
                const SamplerDescription& desc);

            const VkSampler& native() const;
            VkSampler& native();
        private:
            const Device& m_device;
            engine::shared_ptr<VkSampler> m_sampler{ nullptr };
        };
    }
}

