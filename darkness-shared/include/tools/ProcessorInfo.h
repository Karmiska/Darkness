#pragma once

namespace engine
{
    struct ProcessorInformation
    {
        unsigned int logicalProcessorCount;
        unsigned int numaNodeCount;
        unsigned int processorCoreCount;
        unsigned int L1CacheCount;
        unsigned int L2CacheCount;
        unsigned int L3CacheCount;
        unsigned int processorPackageCount;
    };

    ProcessorInformation getProcessorInfo();
}
