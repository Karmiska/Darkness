#pragma once

#include <vulkan/vulkan.h>
#include "containers/memory.h"
#include <functional>

namespace engine
{
    namespace implementation
    {
        constexpr const uint64_t VulkanInfinity = ~0ull;

        template <typename T>
        engine::shared_ptr<T> vulkanPtr(
            std::function<void(VkDevice, const VkAllocationCallbacks*)> deleter,
            const VkAllocationCallbacks* cb = nullptr)
        {
            engine::shared_ptr<T> result = engine::shared_ptr<T>(new T(), [=](const T* ptr) {
                if (*ptr != VK_NULL_HANDLE)
                    deleter(*ptr, cb);
                delete ptr;
            });
            *result = VK_NULL_HANDLE;
            return result;
        }

        template <typename T>
        engine::shared_ptr<T> vulkanPtr(
            const VkDevice& device, 
            std::function<void(VkDevice, T, const VkAllocationCallbacks*)> deleter, 
            T item, 
            const VkAllocationCallbacks* cb = nullptr)
        {
            engine::shared_ptr<T> result = engine::shared_ptr<T>(new T(), [=](const T* ptr) {
                if(*ptr != VK_NULL_HANDLE)
                    deleter(device, *ptr, cb);
                delete ptr;
            });
            *result = item;
            return result;
        }

        template <typename T>
        engine::shared_ptr<T> vulkanPtr(
            const VkInstance& instance,
            std::function<void(VkInstance, T, const VkAllocationCallbacks*)> deleter,
            const VkAllocationCallbacks* cb = nullptr)
        {
            engine::shared_ptr<T> result = engine::shared_ptr<T>(new T(), [=](const T* ptr) {
                if (*ptr != VK_NULL_HANDLE)
                    deleter(instance, *ptr, cb);
                delete ptr;
            });
            *result = VK_NULL_HANDLE;
            return result;
        }

        template <typename T>
        engine::shared_ptr<T> vulkanPtr(
            const VkDevice& device, 
            std::function<void(VkDevice, T, const VkAllocationCallbacks*)> deleter, 
            const VkAllocationCallbacks* cb = nullptr)
        {
            engine::shared_ptr<T> result = engine::shared_ptr<T>(new T(), [=](const T* ptr) {
                if (*ptr != VK_NULL_HANDLE)
                    deleter(device, *ptr, cb);
                delete ptr;
            });
            *result = VK_NULL_HANDLE;
            return result;
        }
    }
}
