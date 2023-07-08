#include "engine/graphics/vulkan/VulkanInstance.h"
#include "tools/Debug.h"
#include "engine/Engine.h"
#include <Windows.h>

using namespace engine;

#define VALIDATION_LAYERS_ACTIVE

VkResult CreateDebugReportCallbackEXT(
    VkInstance instance, 
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugReportCallbackEXT* pCallback)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
                reinterpret_cast<void*>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT")));

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/*VkResult SetDebugUtilsObjectNameEXT(
    VkInstance instance,
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
    auto func = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
        reinterpret_cast<void*>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT")));

    if (func != nullptr) {
        return func(device, pNameInfo);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}*/

PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT;
PFN_vkQueueBeginDebugUtilsLabelEXT QueueBeginDebugUtilsLabelEXT;
PFN_vkQueueEndDebugUtilsLabelEXT QueueEndDebugUtilsLabelEXT;
PFN_vkQueueInsertDebugUtilsLabelEXT QueueInsertDebugUtilsLabelEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;
PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
PFN_vkSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT;

PFN_vkCmdDrawIndirectCountKHR CmdDrawIndirectCountKHR;
PFN_vkCmdDrawIndexedIndirectCountKHR CmdDrawIndexedIndirectCountKHR;

PFN_vkGetSemaphoreCounterValueKHR GetSemaphoreCounterValueKHR;
PFN_vkWaitSemaphoresKHR WaitSemaphoresKHR;
PFN_vkSignalSemaphoreKHR SignalSemaphoreKHR;

PFN_vkCmdBeginConditionalRenderingEXT CmdBeginConditionalRendering;
PFN_vkCmdEndConditionalRenderingEXT CmdEndConditionalRendering;

void DestroyDebugReportCallbackEXT(
    VkInstance instance, 
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                reinterpret_cast<void*>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT")));

    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

static VkBool32 debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT /*objType*/,
    uint64_t /*obj*/,
    size_t /*location*/,
    int32_t /*code*/,
    const char* /*layerPrefix*/,
    const char* msg,
    void* /*userData*/)
{
    if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) ||
        (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) ||
        (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) ||
        (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT))
    {
        LOG("%s", msg);
        return VK_TRUE;
    }
    else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        LOG("%s", msg);
        ASSERT(false);
        return VK_TRUE;
    }
    else
    {
        LOG("%s", msg);
        return VK_TRUE;
    }
}

VulkanInstance::VulkanInstance()
    : //m_instance{ vkDestroyInstance }
    //, m_callback( m_instance, DestroyDebugReportCallbackEXT )
     m_validationLayers{ 
        "VK_LAYER_KHRONOS_validation"
        /*"VK_LAYER_LUNARG_standard_validation",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_image",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_NV_optimus",
        "VK_LAYER_RENDERDOC_Capture"*/
    }
    , m_requiredExtensions{
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
#ifdef VALIDATION_LAYERS_ACTIVE
        ,VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
    }
{
#ifdef VALIDATION_LAYERS_ACTIVE
    if (!checkValidationLayerSupport())
    {
        ASSERT(false);
    }
#endif

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Darkness";
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pEngineName = "Darkness Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    
    // validation layers
#ifdef VALIDATION_LAYERS_ACTIVE
    createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
    createInfo.ppEnabledLayerNames = m_validationLayers.data();
#endif

    // extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = m_requiredExtensions.data();

    auto result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    ASSERT(result == VK_SUCCESS);

    engine::vector<VkExtensionProperties> properties = extensionProperties();

	CmdDrawIndirectCountKHR = reinterpret_cast<PFN_vkCmdDrawIndirectCountKHR>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCmdDrawIndirectCountKHR")));
	CmdDrawIndexedIndirectCountKHR = reinterpret_cast<PFN_vkCmdDrawIndexedIndirectCountKHR>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCmdDrawIndexedIndirectCountKHR")));

    GetSemaphoreCounterValueKHR = reinterpret_cast<PFN_vkGetSemaphoreCounterValueKHR>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkGetSemaphoreCounterValueKHR")));
    WaitSemaphoresKHR = reinterpret_cast<PFN_vkWaitSemaphoresKHR>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkWaitSemaphoresKHR")));
    SignalSemaphoreKHR = reinterpret_cast<PFN_vkSignalSemaphoreKHR>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkSignalSemaphoreKHR")));

    SetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectNameEXT")));
    SetDebugUtilsObjectTagEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectTagEXT")));
    QueueBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueBeginDebugUtilsLabelEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkQueueBeginDebugUtilsLabelEXT")));
    QueueEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueEndDebugUtilsLabelEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkQueueEndDebugUtilsLabelEXT")));
    QueueInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkQueueInsertDebugUtilsLabelEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkQueueInsertDebugUtilsLabelEXT")));
    CmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCmdBeginDebugUtilsLabelEXT")));
    CmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCmdEndDebugUtilsLabelEXT")));
    CmdInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCmdInsertDebugUtilsLabelEXT")));
    CreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT")));
    DestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT")));
    SubmitDebugUtilsMessageEXT = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkSubmitDebugUtilsMessageEXT")));

    CmdBeginConditionalRendering = reinterpret_cast<PFN_vkCmdBeginConditionalRenderingEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCmdBeginConditionalRenderingEXT")));
    CmdEndConditionalRendering = reinterpret_cast<PFN_vkCmdEndConditionalRenderingEXT>(reinterpret_cast<void*>(vkGetInstanceProcAddr(m_instance, "vkCmdEndConditionalRenderingEXT")));

#ifdef VALIDATION_LAYERS_ACTIVE
    bindDebugInfo();
#endif
}

VulkanInstance::~VulkanInstance()
{
    
}

void VulkanInstance::bindDebugInfo()
{
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugCallback;

    if (CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, &m_callback) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug callback!");
    }
}

engine::vector<VkExtensionProperties> VulkanInstance::extensionProperties()
{
    uint32_t extensionCount{0};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    engine::vector<VkExtensionProperties> properties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, properties.data());

    for (auto& ext : properties)
    {
        LOG("Extension poperty: %s\n", ext.extensionName);
    }

    return properties;
}

bool VulkanInstance::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    engine::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_validationLayers)
    {
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                return true;
            }
        }
    }

    return false;
}

VkInstance& VulkanInstance::instance()
{
    return m_instance;
}

const VkInstance& VulkanInstance::instance() const
{
    return m_instance;
}

engine::vector<const char*>& VulkanInstance::validationLayers()
{
    return m_validationLayers;
}

