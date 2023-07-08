#include "tools/ProcessorInfo.h"
#include "platform/Platform.h"
#include "tools/Debug.h"
#include <Sysinfoapi.h>

namespace engine
{
    typedef BOOL(WINAPI *LPFN_GLPI)(
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
        PDWORD);

    DWORD CountSetBits(ULONG_PTR bitMask)
    {
        DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
        DWORD bitSetCount = 0;
        ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
        DWORD i;

        for (i = 0; i <= LSHIFT; ++i)
        {
            bitSetCount += ((bitMask & bitTest) ? 1 : 0);
            bitTest /= 2;
        }

        return bitSetCount;
    }

    ProcessorInformation getProcessorInfo()
    {
        ProcessorInformation result;

        LPFN_GLPI glpi;
        BOOL done = FALSE;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
        DWORD returnLength = 0;
        DWORD logicalProcessorCount = 0;
        DWORD numaNodeCount = 0;
        DWORD processorCoreCount = 0;
        DWORD processorL1CacheCount = 0;
        DWORD processorL2CacheCount = 0;
        DWORD processorL3CacheCount = 0;
        DWORD processorPackageCount = 0;
        DWORD byteOffset = 0;
        PCACHE_DESCRIPTOR Cache;

        glpi = (LPFN_GLPI)GetProcAddress(
            GetModuleHandle(TEXT("kernel32")),
            "GetLogicalProcessorInformation");
        if (NULL == glpi)
        {
            LOG_ERROR("GetLogicalProcessorInformation is not supported.");
            return {};
        }

        while (!done)
        {
            DWORD rc = glpi(buffer, &returnLength);

            if (FALSE == rc)
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (buffer)
                        free(buffer);

                    buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                        returnLength);

                    if (NULL == buffer)
                    {
                        LOG_ERROR("Error: Allocation failure");
                        return {};
                    }
                }
                else
                {
                    LOG_ERROR("Error %d", GetLastError());
                    return {};
                }
            }
            else
            {
                done = TRUE;
            }
        }

        ptr = buffer;

        while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
        {
            switch (ptr->Relationship)
            {
            case RelationNumaNode:
                // Non-NUMA systems report a single record of this type.
                numaNodeCount++;
                break;

            case RelationProcessorCore:
                processorCoreCount++;

                // A hyperthreaded core supplies more than one logical processor.
                logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
                break;

            case RelationCache:
                // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
                Cache = &ptr->Cache;
                if (Cache->Level == 1)
                {
                    processorL1CacheCount++;
                }
                else if (Cache->Level == 2)
                {
                    processorL2CacheCount++;
                }
                else if (Cache->Level == 3)
                {
                    processorL3CacheCount++;
                }
                break;

            case RelationProcessorPackage:
                // Logical processors share a physical package.
                processorPackageCount++;
                break;

            default:
                LOG_ERROR("Error: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.");
                break;
            }
            byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            ptr++;
        }

        /*_tprintf(TEXT("\nGetLogicalProcessorInformation results:\n"));
        _tprintf(TEXT("Number of NUMA nodes: %d\n"),
            numaNodeCount);
        _tprintf(TEXT("Number of physical processor packages: %d\n"),
            processorPackageCount);
        _tprintf(TEXT("Number of processor cores: %d\n"),
            processorCoreCount);
        _tprintf(TEXT("Number of logical processors: %d\n"),
            logicalProcessorCount);
        _tprintf(TEXT("Number of processor L1/L2/L3 caches: %d/%d/%d\n"),
            processorL1CacheCount,
            processorL2CacheCount,
            processorL3CacheCount);*/

        free(buffer);

        result.logicalProcessorCount = logicalProcessorCount;
        result.numaNodeCount = numaNodeCount;
        result.processorCoreCount = processorCoreCount;
        result.L1CacheCount = processorL1CacheCount;
        result.L2CacheCount = processorL2CacheCount;
        result.L3CacheCount = processorL3CacheCount;
        result.processorPackageCount = processorPackageCount;

        return result;
    }
}
