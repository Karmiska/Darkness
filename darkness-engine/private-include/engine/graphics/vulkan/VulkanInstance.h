#pragma once

#include "containers/vector.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"

VkResult CreateDebugReportCallbackEXT(
    VkInstance instance, 
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugReportCallbackEXT* pCallback);

/*VkResult SetDebugUtilsObjectNameEXT(
    VkInstance instance,
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* pNameInfo);*/

extern PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
extern PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT;
extern PFN_vkQueueBeginDebugUtilsLabelEXT QueueBeginDebugUtilsLabelEXT;
extern PFN_vkQueueEndDebugUtilsLabelEXT QueueEndDebugUtilsLabelEXT;
extern PFN_vkQueueInsertDebugUtilsLabelEXT QueueInsertDebugUtilsLabelEXT;
extern PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
extern PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
extern PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;
extern PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
extern PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
extern PFN_vkSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT;

extern PFN_vkCmdBeginConditionalRenderingEXT CmdBeginConditionalRendering;
extern PFN_vkCmdEndConditionalRenderingEXT CmdEndConditionalRendering;


void DestroyDebugReportCallbackEXT(
    VkInstance instance, 
    VkDebugReportCallbackEXT callback, 
    const VkAllocationCallbacks* pAllocator);

extern PFN_vkCmdDrawIndirectCountKHR CmdDrawIndirectCountKHR;
extern PFN_vkCmdDrawIndexedIndirectCountKHR CmdDrawIndexedIndirectCountKHR;

extern PFN_vkGetSemaphoreCounterValueKHR GetSemaphoreCounterValueKHR;
extern PFN_vkWaitSemaphoresKHR WaitSemaphoresKHR;
extern PFN_vkSignalSemaphoreKHR SignalSemaphoreKHR;

class VulkanInstance
{
public:
    VulkanInstance();
    ~VulkanInstance();
    VkInstance& instance();
    const VkInstance& instance() const;
    engine::vector<const char*>& validationLayers();
private:
    VkInstance m_instance;
    VkDebugReportCallbackEXT m_callback;

    engine::vector<const char*> m_validationLayers;
    engine::vector<const char*> m_requiredExtensions;

    engine::vector<VkExtensionProperties> extensionProperties();
    
    bool checkValidationLayerSupport();
    void bindDebugInfo();
};
