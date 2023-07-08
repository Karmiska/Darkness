#ifdef GRAPHICS_API_DX12
#include "engine/graphics/dx12/DX12AfterMath.h"
#endif

#include "tools/Debug.h"

#include "containers/string.h"
#include "containers/vector.h"
#include <iostream>
#include <intrin.h>
#include "platform/Platform.h"

CustomDebugMessageHandler customDebugMessageHandler;

#ifdef AFTERMATH_ENABLED
#include "engine/graphics/dx12/DX12Conversions.h"
#include "engine/graphics/Format.h"
const char* afterMathResultToString(GFSDK_Aftermath_Result res)
{
    switch (res)
    {
        case GFSDK_Aftermath_Result_Success: return "GFSDK_Aftermath_Result_Success";
        case GFSDK_Aftermath_Result_Fail: return "GFSDK_Aftermath_Result_Fail";
        case GFSDK_Aftermath_Result_FAIL_VersionMismatch: return "GFSDK_Aftermath_Result_FAIL_VersionMismatch";
        case GFSDK_Aftermath_Result_FAIL_NotInitialized: return "GFSDK_Aftermath_Result_FAIL_NotInitialized";
        case GFSDK_Aftermath_Result_FAIL_InvalidAdapter: return "GFSDK_Aftermath_Result_FAIL_InvalidAdapter";
        case GFSDK_Aftermath_Result_FAIL_InvalidParameter: return "GFSDK_Aftermath_Result_FAIL_InvalidParameter";
        case GFSDK_Aftermath_Result_FAIL_Unknown: return "GFSDK_Aftermath_Result_FAIL_Unknown";
        case GFSDK_Aftermath_Result_FAIL_ApiError: return "GFSDK_Aftermath_Result_FAIL_ApiError";
        case GFSDK_Aftermath_Result_FAIL_NvApiIncompatible: return "GFSDK_Aftermath_Result_FAIL_NvApiIncompatible";
        case GFSDK_Aftermath_Result_FAIL_GettingContextDataWithNewCommandList: return "GFSDK_Aftermath_Result_FAIL_GettingContextDataWithNewCommandList";
        case GFSDK_Aftermath_Result_FAIL_AlreadyInitialized: return "GFSDK_Aftermath_Result_FAIL_AlreadyInitialized";
        case GFSDK_Aftermath_Result_FAIL_D3DDebugLayerNotCompatible: return "GFSDK_Aftermath_Result_FAIL_D3DDebugLayerNotCompatible";
        case GFSDK_Aftermath_Result_FAIL_DriverInitFailed: return "GFSDK_Aftermath_Result_FAIL_DriverInitFailed";
        case GFSDK_Aftermath_Result_FAIL_DriverVersionNotSupported: return "GFSDK_Aftermath_Result_FAIL_DriverVersionNotSupported";
        case GFSDK_Aftermath_Result_FAIL_OutOfMemory: return "GFSDK_Aftermath_Result_FAIL_OutOfMemory";
        case GFSDK_Aftermath_Result_FAIL_GetDataOnBundle: return "GFSDK_Aftermath_Result_FAIL_GetDataOnBundle";
        case GFSDK_Aftermath_Result_FAIL_GetDataOnDeferredContext: return "GFSDK_Aftermath_Result_FAIL_GetDataOnDeferredContext";
        case GFSDK_Aftermath_Result_FAIL_FeatureNotEnabled: return "GFSDK_Aftermath_Result_FAIL_FeatureNotEnabled";
        default: return "Unknown aftermath error";
    }
    return "";
}

const char* afterMathContextStatusToString(GFSDK_Aftermath_Context_Status status)
{
    switch (status)
    {
        case GFSDK_Aftermath_Context_Status_NotStarted: return "GFSDK_Aftermath_Context_Status_NotStarted";
        case GFSDK_Aftermath_Context_Status_Executing: return "GFSDK_Aftermath_Context_Status_Executing";
        case GFSDK_Aftermath_Context_Status_Finished: return "GFSDK_Aftermath_Context_Status_Finished";
        case GFSDK_Aftermath_Context_Status_Invalid: return "GFSDK_Aftermath_Context_Status_Invalid";
        default: return "Unknown aftermath context status";
    }
    return "";
}

const char* afterMathDeviceStatusToString(GFSDK_Aftermath_Device_Status status)
{
    switch (status)
    {
        case GFSDK_Aftermath_Device_Status_Active: return "GFSDK_Aftermath_Device_Status_Active";
        case GFSDK_Aftermath_Device_Status_Timeout: return "GFSDK_Aftermath_Device_Status_Timeout";
        case GFSDK_Aftermath_Device_Status_OutOfMemory: return "GFSDK_Aftermath_Device_Status_OutOfMemory";
        case GFSDK_Aftermath_Device_Status_PageFault: return "GFSDK_Aftermath_Device_Status_PageFault";
        case GFSDK_Aftermath_Device_Status_Unknown: return "GFSDK_Aftermath_Device_Status_Unknown";
        default: return "Unknown aftermath device status";
    }
    return "";
}

void printAfterMathResourceDescriptor(GFSDK_Aftermath_ResourceDescriptor desc)
{
    LOG("ResourceDescriptor:");
    LOG("size: %llu, width: %u, height: %u, depth: %u, mipLevels: %u, format: %s, bIsBufferHeap: %s, bIsStaticTextureHeap: %s, bIsRtvDsvTextureHeap: %s, bPlacedResource: %s, bWasDestroyed: %s",
        desc.size, desc.width, desc.height, desc.depth, desc.mipLevels, 
        engine::formatToString(engine::implementation::fromDXFormat(desc.format)).c_str(),
        desc.bIsBufferHeap ? "true" : "false",
        desc.bIsStaticTextureHeap ? "true" : "false",
        desc.bIsRtvDsvTextureHeap ? "true" : "false",
        desc.bPlacedResource ? "true" : "false",
        desc.bWasDestroyed ? "true" : "false");
}

void printAfterMathPageFaultInfo(GFSDK_Aftermath_PageFaultInformation& info)
{
    LOG("Page fault info:");
    LOG("faultingGpuVA: %llu", info.faultingGpuVA);
    LOG("bhasPageFaultOccured: %s", info.bhasPageFaultOccured ? "true" : "false");
    printAfterMathResourceDescriptor(info.resourceDesc);
}

void performAftermath()
{
    engine::vector<GFSDK_Aftermath_ContextData> contextData(afterMathContext.contextHandles().size());
    GFSDK_Aftermath_GetData(
        afterMathContext.contextHandles().size(),
        &afterMathContext.contextHandles()[0],
        &contextData[0]);

    LOG("Aftermath context count: %i", static_cast<int>(contextData.size()));
    for (auto& context : contextData)
    {
        engine::string marker;
        marker.assign(static_cast<const char*>(context.markerData), context.markerSize);
        
        LOG("Aftermath context marker: %s, status: %s, errorcode: %s",
            marker.c_str(),
            afterMathContextStatusToString(context.status),
            afterMathResultToString(context.getErrorCode()));
    }

    GFSDK_Aftermath_Device_Status deviceStatus;
    GFSDK_Aftermath_GetDeviceStatus(&deviceStatus);
    LOG("Aftermath device status: %s", afterMathDeviceStatusToString(deviceStatus));

    GFSDK_Aftermath_PageFaultInformation pageFaultInfo;
    auto res = GFSDK_Aftermath_GetPageFaultInformation(&pageFaultInfo);
    ASSERT(res == GFSDK_Aftermath_Result_Success, "Could not get page fault infromation from aftermath");
    printAfterMathPageFaultInfo(pageFaultInfo);
}
#endif


void DebugAssert(const char* condition, const char* location)
{
#if 1
    engine::string smsg{ condition };
    engine::string slocation{ location };

    engine::string message = "####################### ASSERT!! ########################\n";
    message += "#\n";
    message += "# condition: ";
    message += smsg;
    message += "\n";
    message += slocation;
    message += "Error: ASSERT\n";
    message += "#\n";
    message += "####################### ASSERT!! ########################\n";
    message += "\n";
    message += "\n";

#ifdef _WIN32
    OutputDebugStringA(message.data());
#endif
    std::cout << message.data() << std::flush;
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(message);

#ifdef AFTERMATH_ENABLED
    performAftermath();
#endif

    __debugbreak();

	std::terminate();
}

void DebugAssert(const char* condition, const char* location, const char* msg, ...)
{
#if 1
    engine::string smsg{ condition };
    engine::string slocation{ location };

    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s", message.data());

    engine::string assertMessage = buffer.data();

    engine::string assert_message = "####################### ASSERT!! ########################\n";
    assert_message += "#\n";
    assert_message += "# condition: ";
    assert_message += smsg;
    assert_message += "\n";
    assert_message += slocation;
    assert_message += "Error: ";
    assert_message += assertMessage;
    assert_message += "\n";
    assert_message += "#\n";
    assert_message += "####################### ASSERT!! ########################\n";
    assert_message += "\n";
    assert_message += "\n";

#ifdef _WIN32
    OutputDebugStringA(assert_message.data());
#endif
    std::cout << assert_message.data() << std::flush;
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(assert_message);

#ifdef AFTERMATH_ENABLED
    performAftermath();
#endif

    __debugbreak();

	std::terminate();
}

void Debug(const char* location, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s%s%s\n", location, "Log: ", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(message.data());

    std::cout << buffer.data() << std::flush;
}

void DebugInfo(const char* location, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s%s%s\n", location, "Info: ", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(buffer.data());

    std::cout << buffer.data() << std::flush;
}

void DebugWarning(const char* location, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s%s%s\n", location, "Warning: ", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(buffer.data());

    std::cout << buffer.data() << std::flush;
}

void DebugError(const char* location, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s%s%s\n", location, "Error: ", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(buffer.data());

    std::cout << buffer.data() << std::flush;
}

void DebugPure(const char* /*location*/, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s\n", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(buffer.data());

    std::cout << buffer.data() << std::flush;
}

void DebugPureInfo(const char* /*location*/, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s\n", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(buffer.data());

    std::cout << buffer.data() << std::flush;
}

void DebugPureWarning(const char* /*location*/, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s\n", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(buffer.data());

    std::cout << buffer.data() << std::flush;
}

void DebugPureError(const char* /*location*/, const char* msg, ...)
{
    engine::vector<char> message(32768);
    va_list arguments;
    va_start(arguments, msg);
    vsprintf_s(message.data(), 32768, msg, arguments);
    va_end(arguments);

    engine::vector<char> buffer(32768);
    sprintf_s(buffer.data(), 32768, "%s\n", message.data());

#ifdef _WIN32
    OutputDebugStringA(buffer.data());
#endif

    if (customDebugMessageHandler)
        customDebugMessageHandler(buffer.data());

    std::cout << buffer.data() << std::flush;
}
