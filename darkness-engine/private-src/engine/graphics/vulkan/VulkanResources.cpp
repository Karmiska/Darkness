#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Common.h"

#include "tools/Debug.h"

// memcpy
#include <cstring>

namespace engine
{
    namespace implementation
    {
        uint32_t findMemoryType(const DeviceImplVulkan& device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(device.physicalDevice(), &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }
            ASSERT(false);
            return 0;
        }

        void updateDescFromInitialData(const BufferDescription& origdesc, BufferDescription::Descriptor& desc)
        {
            auto elements = origdesc.descriptor.elements;
            auto elementSize = origdesc.descriptor.elementSize;
            if (elements == InvalidElementsValue)
            {
                if (origdesc.initialData)
                {
                    elements = origdesc.initialData.elements;
                }
                else ASSERT(false);
            }
            if (elementSize == InvalidElementSizeValue)
            {
                if (origdesc.initialData)
                {
                    elementSize = origdesc.initialData.elementSize;
                }
                if (elementSize == InvalidElementSizeValue)
                {
                    elementSize = static_cast<int32_t>(formatBytes(origdesc.descriptor.format));
                }
                if (elementSize == InvalidElementSizeValue)
                    ASSERT(false);
            }
            desc.elements = elements;
            desc.elementSize = elementSize;
        }

        /*VkBufferUsageFlags vulkanBufferUsageFlagsFromUsage(ResourceUsage usage)
        {
            switch (usage)
            {
            case ResourceUsage::GpuRead: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            case ResourceUsage::GpuReadWrite: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
            can't mix buffer and image flags
            case ResourceUsage::GpuRenderTargetRead: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            case ResourceUsage::GpuRenderTargetReadWrite: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            case ResourceUsage::DepthStencil: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
            case ResourceUsage::GpuToCpu: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
            case ResourceUsage::Upload: return VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            default: return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
            }
        }*/

        VkMemoryPropertyFlags vulkanMemoryPropertyFlagsFromUsage(ResourceUsage usage)
        {
            switch (usage)
            {
            case ResourceUsage::GpuRead: return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case ResourceUsage::GpuReadWrite: return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case ResourceUsage::GpuRenderTargetRead: return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case ResourceUsage::GpuRenderTargetReadWrite: return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case ResourceUsage::DepthStencil: return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case ResourceUsage::GpuToCpu: return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            case ResourceUsage::Upload: return VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            default: return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            }
        }

        BufferImplVulkan::BufferImplVulkan(const DeviceImplVulkan& device, const BufferDescription& desc)
            : m_description( desc.descriptor )
            , m_buffer{ vulkanPtr<VkBuffer>(device.device(), vkDestroyBuffer) }
            , m_memory{ vulkanPtr<VkDeviceMemory>(device.device(), vkFreeMemory) }
            , m_state{ getResourceStateFromUsage(m_description.usage) }
        {
            updateDescFromInitialData(desc, m_description);

            const VkDevice& dev = device.device();

            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = static_cast<VkDeviceSize>(m_description.elements * m_description.elementSize);
            
			bufferInfo.usage = 0;

            // generic access bits
            if (desc.descriptor.usage == ResourceUsage::GpuRead)
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            if (desc.descriptor.usage == ResourceUsage::GpuReadWrite)
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            if (desc.descriptor.usage == ResourceUsage::Upload)
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            if (desc.descriptor.usage == ResourceUsage::GpuToCpu)
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            // Structured SRV
            if (((desc.descriptor.usage == ResourceUsage::GpuRead) ||
                (desc.descriptor.usage == ResourceUsage::GpuToCpu) ||
                (desc.descriptor.usage == ResourceUsage::Upload)) &&
                desc.descriptor.structured)
                bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

            // Type SRV
            if (((desc.descriptor.usage == ResourceUsage::GpuRead) ||
                (desc.descriptor.usage == ResourceUsage::GpuToCpu)) &&
                !desc.descriptor.structured)
                bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

            // Structured UAV
            if ((desc.descriptor.usage == ResourceUsage::GpuReadWrite) && desc.descriptor.structured)
                bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            // Type UAV
            if ((desc.descriptor.usage == ResourceUsage::GpuReadWrite) && !desc.descriptor.structured)
                bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

            // is index buffer
            if (desc.descriptor.indexBuffer)
                bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            // is vertex buffer
            if (desc.descriptor.vertexBuffer)
                bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

            // is indirect argument
            if (desc.descriptor.indirectArgument)
                bufferInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            auto result = vkCreateBuffer(dev, &bufferInfo, nullptr, m_buffer.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_buffer.get());
            debInfo.pObjectName = desc.descriptor.name;
            debInfo.objectType = VK_OBJECT_TYPE_BUFFER;
            result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
            ASSERT(result == VK_SUCCESS);

            VkMemoryRequirements memReq;
            vkGetBufferMemoryRequirements(dev, *m_buffer, &memReq);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReq.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                device,
                memReq.memoryTypeBits,
                vulkanMemoryPropertyFlagsFromUsage(m_description.usage));

            result = vkAllocateMemory(dev, &allocInfo, nullptr, m_memory.get());
            ASSERT(result == VK_SUCCESS);

            vkBindBufferMemory(dev, *m_buffer, *m_memory, 0);

            //LOG("Buffer created: %p, %s", *m_buffer, m_description.name);
        }

        BufferImplVulkan::~BufferImplVulkan()
        {
            //LOG("Buffer destroyed: %p, %s", *m_buffer, m_description.name);
        }

        void* BufferImplVulkan::map(const DeviceImplIf* device)
        {
            void* data;
            vkMapMemory(static_cast<const DeviceImplVulkan*>(device)->device(), (*m_memory.get()), 0, m_description.elements * m_description.elementSize, 0, &data);
            return data;
        }

        void BufferImplVulkan::unmap(const DeviceImplIf* device)
        {
            vkUnmapMemory(static_cast<const DeviceImplVulkan*>(device)->device(), *m_memory.get());
        }

        const BufferDescription::Descriptor& BufferImplVulkan::description() const
        {
            return m_description;
        }

        ResourceState BufferImplVulkan::state() const
        {
            return m_state;
        }

        void BufferImplVulkan::state(ResourceState state)
        {
            m_state = state;
        }

        VkBuffer& BufferImplVulkan::native()
        {
            return *m_buffer;
        }

        const VkBuffer& BufferImplVulkan::native() const
        {
            return *m_buffer;
        }

        /*BufferViewImpl::BufferViewImpl(
            const Device& device, 
            const Buffer& buffer, 
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkBufferView>(device.device(), vkDestroyBufferView) }
            , m_memory{ BufferImplGet::impl(buffer)->m_memory }
        {
            const VkDevice& dev = device.device();

            auto elements = m_description.elements != InvalidElementsValue ? m_description.elements : buffer.description().descriptor.elements;
            auto elementSize = m_description.elementSize != InvalidElementSizeValue ? m_description.elementSize : buffer.description().descriptor.elementSize;

            m_description.elements = elements;
            m_description.elementSize = elementSize;

            VkBufferViewCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            bufferInfo.buffer = BufferImplGet::impl(buffer)->native();
            bufferInfo.flags = 0;// vulkanBufferUsageFlags(desc.descriptor.usage) | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            bufferInfo.format = vulkanFormat(desc.descriptor.format);
            bufferInfo.offset = desc.descriptor.firstElement * elementSize;
            bufferInfo.range = elements * elementSize;

            auto result = vkCreateBufferView(dev, &bufferInfo, nullptr, m_view.get());
            ASSERT(result == VK_SUCCESS);
        }

        void* BufferViewImpl::map(const Device& device)
        {
            void* data;
            vkMapMemory(device.device(), (*m_memory.get()), 0, m_description.elements * m_description.elementSize, 0, &data);
            return data;
        }

        void BufferViewImpl::unmap(const Device& device)
        {
            vkUnmapMemory(device.device(), *m_memory.get());
        }

        const BufferDescription::Descriptor& BufferViewImpl::description() const
        {
            return m_description;
        }

        VkBufferView& BufferViewImpl::native()
        {
            return *m_view;
        }

        const VkBufferView& BufferViewImpl::native() const
        {
            return *m_view;
        }*/

        //------------------------
        BufferSRVImplVulkan::BufferSRVImplVulkan(
            const DeviceImplVulkan& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkBufferView>(device.device(), vkDestroyBufferView) }
            , m_memory{ static_cast<BufferImplVulkan*>(buffer.m_impl)->m_memory }
            , m_buffer{ buffer }
            , m_uniqueId{ GlobalUniqueHandleId++ }
        {
            const VkDevice& dev = device.device();

            size_t elements = m_description.elements != InvalidElementsValue ? m_description.elements : buffer.description().elements;
            size_t elementSize = m_description.elementSize != InvalidElementSizeValue ? m_description.elementSize : buffer.description().elementSize;

            m_description.elements = elements;
            m_description.elementSize = elementSize;

			if (!desc.descriptor.structured)
			{
				VkBufferViewCreateInfo bufferInfo = {};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				bufferInfo.buffer = static_cast<BufferImplVulkan*>(buffer.m_impl)->native();
				bufferInfo.flags = 0;
				bufferInfo.format = vulkanFormat(desc.descriptor.format);

				if (bufferInfo.format == VK_FORMAT_UNDEFINED)
					bufferInfo.format = vulkanFormat(buffer.description().format);

				if ((bufferInfo.format == VK_FORMAT_UNDEFINED) && (m_description.usage == ResourceUsage::DepthStencil))
					bufferInfo.format = m_description.elementSize == 4 ? VK_FORMAT_R32_UINT : VK_FORMAT_R16_UINT;

				//if (desc.descriptor.structured)
				//	bufferInfo.format = VK_FORMAT_R8_UNORM;

				bufferInfo.offset = desc.descriptor.firstElement * elementSize;
				bufferInfo.range = elements * elementSize;

				auto result = vkCreateBufferView(dev, &bufferInfo, nullptr, m_view.get());
				ASSERT(result == VK_SUCCESS);

                VkDebugUtilsObjectNameInfoEXT debInfo = {};
                debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
                debInfo.pObjectName = desc.descriptor.name;
                debInfo.objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
                result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
                ASSERT(result == VK_SUCCESS);

                //LOG("Buffer SRV created: %p, %s", *m_view, m_description.name);
			}
        }

        BufferSRVImplVulkan::~BufferSRVImplVulkan()
        {
            //LOG("Buffer SRV destroyed: %p, %s", *m_view, m_description.name);
        }

        const BufferDescription::Descriptor& BufferSRVImplVulkan::description() const
        {
            return m_description;
        }

        VkBufferView& BufferSRVImplVulkan::native()
        {
            return *m_view;
        }

        const VkBufferView& BufferSRVImplVulkan::native() const
        {
            return *m_view;
        }

        Buffer BufferSRVImplVulkan::buffer() const
        {
            return m_buffer;
        }

        BufferUAVImplVulkan::BufferUAVImplVulkan(
            const DeviceImplVulkan& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkBufferView>(device.device(), vkDestroyBufferView) }
            , m_memory{ static_cast<BufferImplVulkan*>(buffer.m_impl)->m_memory }
            , m_buffer{ buffer }
            , m_counterBuffer{ desc.descriptor.append ? vulkanPtr<VkBuffer>(device.device(), vkDestroyBuffer) : nullptr }
            , m_counterMemory{ desc.descriptor.append ? vulkanPtr<VkDeviceMemory>(device.device(), vkFreeMemory) : nullptr }
            , m_uniqueId{ GlobalUniqueHandleId++ }
        {
            const VkDevice& dev = device.device();

            size_t elements = m_description.elements != InvalidElementsValue ? m_description.elements : buffer.description().elements;
            size_t elementSize = m_description.elementSize != InvalidElementSizeValue ? m_description.elementSize : buffer.description().elementSize;

            m_description.elements = elements;
            m_description.elementSize = elementSize;

			if (!desc.descriptor.structured)
			{
				VkBufferViewCreateInfo bufferInfo = {};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				bufferInfo.buffer = static_cast<BufferImplVulkan*>(buffer.m_impl)->native();
				bufferInfo.flags = 0;// vulkanBufferUsageFlags(desc.descriptor.usage) | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
				bufferInfo.format = vulkanFormat(buffer.description().format);// vulkanFormat(desc.descriptor.format);
				bufferInfo.offset = desc.descriptor.firstElement * elementSize;
				bufferInfo.range = elements * elementSize;

				auto result = vkCreateBufferView(dev, &bufferInfo, nullptr, m_view.get());
				ASSERT(result == VK_SUCCESS);

                VkDebugUtilsObjectNameInfoEXT debInfo = {};
                debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
                debInfo.pObjectName = desc.descriptor.name;
                debInfo.objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
                result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
                ASSERT(result == VK_SUCCESS);
			}
            if (desc.descriptor.append)
            {
                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = static_cast<VkDeviceSize>(sizeof(uint32_t));
                bufferInfo.usage = 0;
                bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                auto result = vkCreateBuffer(dev, &bufferInfo, nullptr, m_counterBuffer.get());
                ASSERT(result == VK_SUCCESS);

                VkDebugUtilsObjectNameInfoEXT debInfo = {};
                debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_counterBuffer.get());
                debInfo.pObjectName = desc.descriptor.name;
                debInfo.objectType = VK_OBJECT_TYPE_BUFFER;
                result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
                ASSERT(result == VK_SUCCESS);

                VkMemoryRequirements memReq;
                vkGetBufferMemoryRequirements(dev, *m_counterBuffer, &memReq);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memReq.size;
                allocInfo.memoryTypeIndex = findMemoryType(
                    device,
                    memReq.memoryTypeBits,
                    vulkanMemoryPropertyFlagsFromUsage(m_description.usage));

                result = vkAllocateMemory(dev, &allocInfo, nullptr, m_counterMemory.get());
                ASSERT(result == VK_SUCCESS);

                vkBindBufferMemory(dev, *m_counterBuffer, *m_counterMemory, 0);
            }
        }

        const BufferDescription::Descriptor& BufferUAVImplVulkan::description() const
        {
            return m_description;
        }

        VkBufferView& BufferUAVImplVulkan::native()
        {
            return *m_view;
        }

        const VkBufferView& BufferUAVImplVulkan::native() const
        {
            return *m_view;
        }

        Buffer BufferUAVImplVulkan::buffer() const
        {
            return m_buffer;
        }

        uint64_t BufferUAVImplVulkan::uniqueId() const
        {
            return m_uniqueId;
        }

        size_t BufferUAVImplVulkan::structureCounterOffsetBytes() const
        {
            return 0ull;
        }

        BufferIBVImplVulkan::BufferIBVImplVulkan(
            const DeviceImplVulkan& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkBufferView>(device.device(), vkDestroyBufferView) }
            , m_memory{ static_cast<BufferImplVulkan*>(buffer.m_impl)->m_memory }
            , m_buffer{ buffer }
        {
            const VkDevice& dev = device.device();

            size_t elements = m_description.elements != InvalidElementsValue ? m_description.elements : buffer.description().elements;
            size_t elementSize = m_description.elementSize != InvalidElementSizeValue ? m_description.elementSize : buffer.description().elementSize;

            m_description.elements = elements;
            m_description.elementSize = elementSize;

			if (!desc.descriptor.structured)
			{
				VkBufferViewCreateInfo bufferInfo = {};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				bufferInfo.buffer = static_cast<BufferImplVulkan*>(buffer.m_impl)->native();
				bufferInfo.flags = 0;
				bufferInfo.format = vulkanFormat(desc.descriptor.format);
				if (bufferInfo.format == VK_FORMAT_UNDEFINED)
					bufferInfo.format = vulkanFormat(buffer.description().format);
				if (bufferInfo.format == VK_FORMAT_UNDEFINED)
					bufferInfo.format = m_description.elementSize == 4 ? VK_FORMAT_R32_UINT : VK_FORMAT_R16_UINT;

				bufferInfo.offset = desc.descriptor.firstElement * elementSize;
				bufferInfo.range = elements * elementSize;

				auto result = vkCreateBufferView(dev, &bufferInfo, nullptr, m_view.get());
				ASSERT(result == VK_SUCCESS);

                VkDebugUtilsObjectNameInfoEXT debInfo = {};
                debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
                debInfo.pObjectName = desc.descriptor.name;
                debInfo.objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
                result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
                ASSERT(result == VK_SUCCESS);
			}
        }

        const BufferDescription::Descriptor& BufferIBVImplVulkan::description() const
        {
            return m_description;
        }

        VkBufferView& BufferIBVImplVulkan::native()
        {
            return *m_view;
        }

        const VkBufferView& BufferIBVImplVulkan::native() const
        {
            return *m_view;
        }

        Buffer BufferIBVImplVulkan::buffer() const
        {
            return m_buffer;
        }

        BufferCBVImplVulkan::BufferCBVImplVulkan(
            const DeviceImplVulkan& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkBufferView>(device.device(), vkDestroyBufferView) }
            , m_memory{ static_cast<BufferImplVulkan*>(buffer.m_impl)->m_memory }
            , m_buffer{ buffer }
        {
            const VkDevice& dev = device.device();

            size_t elements = m_description.elements != InvalidElementsValue ? m_description.elements : buffer.description().elements;
            size_t elementSize = m_description.elementSize != InvalidElementSizeValue ? m_description.elementSize : buffer.description().elementSize;

            m_description.elements = elements;
            m_description.elementSize = elementSize;

			if (!desc.descriptor.structured)
			{
				VkBufferViewCreateInfo bufferInfo = {};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				bufferInfo.buffer = static_cast<BufferImplVulkan*>(buffer.m_impl)->native();
				bufferInfo.flags = 0;// vulkanBufferUsageFlags(desc.descriptor.usage) | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
				bufferInfo.format = vulkanFormat(desc.descriptor.format);
				bufferInfo.offset = desc.descriptor.firstElement * elementSize;
				bufferInfo.range = elements * elementSize;

				auto result = vkCreateBufferView(dev, &bufferInfo, nullptr, m_view.get());
				ASSERT(result == VK_SUCCESS);

                VkDebugUtilsObjectNameInfoEXT debInfo = {};
                debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
                debInfo.pObjectName = desc.descriptor.name;
                debInfo.objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
                result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
                ASSERT(result == VK_SUCCESS);
			}
        }

        const BufferDescription::Descriptor& BufferCBVImplVulkan::description() const
        {
            return m_description;
        }

        VkBufferView& BufferCBVImplVulkan::native()
        {
            return *m_view;
        }

        const VkBufferView& BufferCBVImplVulkan::native() const
        {
            return *m_view;
        }

        Buffer BufferCBVImplVulkan::buffer() const
        {
            return m_buffer;
        }

        BufferVBVImplVulkan::BufferVBVImplVulkan(
            const DeviceImplVulkan& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkBufferView>(device.device(), vkDestroyBufferView) }
            , m_memory{ static_cast<BufferImplVulkan*>(buffer.m_impl)->m_memory }
            , m_buffer{ buffer }
        {
            const VkDevice& dev = device.device();

            size_t elements = m_description.elements != InvalidElementsValue ? m_description.elements : buffer.description().elements;
            size_t elementSize = m_description.elementSize != InvalidElementSizeValue ? m_description.elementSize : buffer.description().elementSize;

            m_description.elements = elements;
            m_description.elementSize = elementSize;

			if (!desc.descriptor.structured)
			{
				VkBufferViewCreateInfo bufferInfo = {};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
				bufferInfo.buffer = static_cast<BufferImplVulkan*>(buffer.m_impl)->native();
				bufferInfo.flags = 0;// vulkanBufferUsageFlags(desc.descriptor.usage) | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
				bufferInfo.format = vulkanFormat(desc.descriptor.format);
				bufferInfo.offset = desc.descriptor.firstElement * elementSize;
				bufferInfo.range = elements * elementSize;

				auto result = vkCreateBufferView(dev, &bufferInfo, nullptr, m_view.get());
				ASSERT(result == VK_SUCCESS);

                VkDebugUtilsObjectNameInfoEXT debInfo = {};
                debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
                debInfo.pObjectName = desc.descriptor.name;
                debInfo.objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
                result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
                ASSERT(result == VK_SUCCESS);
			}
        }

        const BufferDescription::Descriptor& BufferVBVImplVulkan::description() const
        {
            return m_description;
        }

        VkBufferView& BufferVBVImplVulkan::native()
        {
            return *m_view;
        }

        const VkBufferView& BufferVBVImplVulkan::native() const
        {
            return *m_view;
        }

        Buffer BufferVBVImplVulkan::buffer() const
        {
            return m_buffer;
        }

        BindlessBufferSRVImplVulkan::BindlessBufferSRVImplVulkan(const DeviceImplVulkan& /*device*/)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_change{ true }
        {}

        bool BindlessBufferSRVImplVulkan::operator==(const BindlessBufferSRVImplVulkan& buff) const
        {
            return m_resourceId == buff.m_resourceId;
        }

        uint32_t BindlessBufferSRVImplVulkan::push(BufferSRVOwner buffer)
        {
            auto res = m_buffers.size();
            m_buffers.emplace_back(buffer);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessBufferSRVImplVulkan::size() const
        {
            return m_buffers.size();
        }

        BufferSRV BindlessBufferSRVImplVulkan::get(size_t index)
        {
            return m_buffers[index];
        }

        uint64_t BindlessBufferSRVImplVulkan::resourceId() const
        {
            return m_resourceId;
        }

        void BindlessBufferSRVImplVulkan::updateDescriptors(DeviceImplIf* /*device*/)
        {
            LOG("BindlessBufferSRVImplVulkan::updateDescriptors not implemented");
        }

        bool BindlessBufferSRVImplVulkan::change() const
        {
            return m_change;
        }

        void BindlessBufferSRVImplVulkan::change(bool value)
        {
            m_change = value;
        }

        BindlessBufferUAVImplVulkan::BindlessBufferUAVImplVulkan(const DeviceImplVulkan& /*device*/)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_change{ true }
        {}

        bool BindlessBufferUAVImplVulkan::operator==(const BindlessBufferUAVImplVulkan& buff) const
        {
            return m_resourceId == buff.m_resourceId;
        }

        uint32_t BindlessBufferUAVImplVulkan::push(BufferUAVOwner buffer)
        {
            auto res = m_buffers.size();
            m_buffers.emplace_back(buffer);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessBufferUAVImplVulkan::size() const
        {
            return m_buffers.size();
        }

        BufferUAV BindlessBufferUAVImplVulkan::get(size_t index)
        {
            return m_buffers[index];
        }

        uint64_t BindlessBufferUAVImplVulkan::resourceId() const
        {
            return m_resourceId;
        }

        void BindlessBufferUAVImplVulkan::updateDescriptors(DeviceImplIf* /*device*/)
        {
            LOG("BindlessBufferUAVImplVulkan::updateDescriptors not implemented");
        }

        bool BindlessBufferUAVImplVulkan::change() const
        {
            return m_change;
        }

        void BindlessBufferUAVImplVulkan::change(bool value)
        {
            m_change = value;
        }

        RaytracingAccelerationStructureImplVulkan::RaytracingAccelerationStructureImplVulkan(
            const Device& /*device*/,
            BufferSRV /*vertexBuffer*/,
            BufferIBV /*indexBuffer*/,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_state{ ResourceState::Common }
        {
            LOG("BindlessBufferUAVImplVulkan::updateDescriptors not implemented");
        }

        RaytracingAccelerationStructureImplVulkan::~RaytracingAccelerationStructureImplVulkan()
        {

        }

        const BufferDescription::Descriptor& RaytracingAccelerationStructureImplVulkan::description() const
        {
            return m_description;
        }

        ResourceState RaytracingAccelerationStructureImplVulkan::state() const
        {
            return m_state;
        }

        void RaytracingAccelerationStructureImplVulkan::state(ResourceState _state)
        {
            m_state = _state;
        }

        bool RaytracingAccelerationStructureImplVulkan::operator==(const RaytracingAccelerationStructureImplVulkan& buff) const
        {
            return
                (m_bottomLevel.resource() == buff.m_bottomLevel.resource()) &&
                (m_topLevel.resource() == buff.m_topLevel.resource());
        }

        uint64_t RaytracingAccelerationStructureImplVulkan::resourceId() const
        {
            return 0;
        }

        VkImageTiling vulkanImageTilingFromUsage(ResourceUsage usage)
        {
            switch (usage)
            {
            case ResourceUsage::GpuToCpu: return VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
            case ResourceUsage::GpuRead: return VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
            case ResourceUsage::GpuRenderTargetRead: return VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
            case ResourceUsage::GpuRenderTargetReadWrite: return VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
            case ResourceUsage::DepthStencil: return VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
            case ResourceUsage::GpuReadWrite: return VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
            case ResourceUsage::Upload: return VkImageTiling::VK_IMAGE_TILING_LINEAR;
            default: return VkImageTiling::VK_IMAGE_TILING_LINEAR;
            }
        }

        TextureImplVulkan::TextureImplVulkan(const DeviceImplVulkan& device, const TextureDescription& desc)
            : m_description( desc.descriptor )
            , m_image{ vulkanPtr<VkImage>(device.device(), vkDestroyImage) }
            , m_memory{ vulkanPtr<VkDeviceMemory>(device.device(), vkFreeMemory) }
            //, m_state{ getResourceStateFromUsage(m_description.usage) }
        {
			for (int slice = 0; slice < static_cast<int>(m_description.arraySlices); ++slice)
			{
				for (int mip = 0; mip < static_cast<int>(m_description.mipLevels); ++mip)
				{
					m_state.emplace_back(ResourceState::Undefined);
				}
			}

            const VkDevice& dev = device.device();

            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = vulkanDimension(m_description.dimension);
            imageInfo.extent.width = static_cast<uint32_t>(m_description.width);
            imageInfo.extent.height = static_cast<uint32_t>(m_description.height);
            imageInfo.extent.depth = static_cast<uint32_t>(m_description.depth);
            imageInfo.mipLevels = static_cast<uint32_t>(m_description.mipLevels);
            imageInfo.arrayLayers = static_cast<uint32_t>(m_description.arraySlices);

            imageInfo.format = vulkanFormat(m_description.format);

			if (desc.descriptor.usage == ResourceUsage::DepthStencil)
			{
				if (desc.descriptor.format == Format::D32_FLOAT)
					imageInfo.format = vulkanFormat(Format::R32_TYPELESS);
				if (desc.descriptor.format == Format::D16_UNORM)
					imageInfo.format = vulkanFormat(Format::R16_UNORM);
			}


            imageInfo.tiling = vulkanImageTilingFromUsage(m_description.usage);
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			
			if (m_description.usage == ResourceUsage::GpuRenderTargetRead)
				imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			
			if (m_description.usage == ResourceUsage::GpuRenderTargetReadWrite)
				imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            if (m_description.usage == ResourceUsage::GpuRenderTargetReadWrite && m_description.samples == 1)
                imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

			if (m_description.usage == ResourceUsage::DepthStencil)
				imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            if (m_description.usage == ResourceUsage::GpuReadWrite)
                imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = vulkanSamples(static_cast<uint32_t>(m_description.samples));
            imageInfo.flags = (desc.descriptor.dimension == ResourceDimension::TextureCube) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

            VkImageFormatProperties imageFormatProperties = {};
            auto result = vkGetPhysicalDeviceImageFormatProperties(device.physicalDevice(), imageInfo.format, imageInfo.imageType, imageInfo.tiling, imageInfo.usage, imageInfo.flags, &imageFormatProperties);
            if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
            {
                // need to tweak something
                //imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                //imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
                imageInfo.format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                result = vkGetPhysicalDeviceImageFormatProperties(
                    device.physicalDevice(), 
                    imageInfo.format, 
                    imageInfo.imageType, 
                    imageInfo.tiling, 
                    imageInfo.usage, 
                    imageInfo.flags, 
                    &imageFormatProperties);
            }
            ASSERT(result == VK_SUCCESS);

            result = vkCreateImage(device.device(), &imageInfo, nullptr, m_image.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_image.get());
            debInfo.pObjectName = desc.descriptor.name;
            debInfo.objectType = VK_OBJECT_TYPE_IMAGE;
            result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
            ASSERT(result == VK_SUCCESS);

            VkMemoryRequirements memReq;
            vkGetImageMemoryRequirements(dev, *m_image, &memReq);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReq.size;
            allocInfo.memoryTypeIndex = findMemoryType(
                device,
                memReq.memoryTypeBits,
                vulkanMemoryPropertyFlagsFromUsage(desc.descriptor.usage));

            result = vkAllocateMemory(device.device(), &allocInfo, nullptr, m_memory.get());
            ASSERT(result == VK_SUCCESS);

            result = vkBindImageMemory(device.device(), *m_image, *m_memory, 0);
            ASSERT(result == VK_SUCCESS);

            /*if (desc.initialData)
            {
                ASSERT(allocInfo.allocationSize == desc.initialData.data.size());
                void* data;
                result = vkMapMemory(
                    device.device(), 
                    *m_memory, 
                    0,
                    desc.initialData.data.size(),
                    0, 
                    &data);
                ASSERT(result == VK_SUCCESS);

                std::memcpy(data, reinterpret_cast<uint8_t*>(desc.initialData.data.start), desc.initialData.data.sizeBytes());
                vkUnmapMemory(device.device(), *m_memory.get());
            }*/
        }

        TextureImplVulkan::TextureImplVulkan(
            engine::shared_ptr<VkImage> image,
            const TextureDescription& desc)
            : m_description(desc.descriptor)
            , m_image{ image }
            , m_memory{ nullptr }
        {
			for (int slice = 0; slice < static_cast<int>(m_description.arraySlices); ++slice)
			{
				for (int mip = 0; mip < static_cast<int>(m_description.mipLevels); ++mip)
				{
					m_state.emplace_back(ResourceState::Undefined);
				}
			}
        }

        void* TextureImplVulkan::map(const DeviceImplIf* /*device*/)
        {
            void* data = nullptr;
            LOG_WARNING("TODO: TextureImplVulkan::map not implemented");
            //vkMapMemory(device.device(), (*m_memory.get()), 0, m_description.elements * m_description.elementSize, 0, &data);
            return data;
        }

        void TextureImplVulkan::unmap(const DeviceImplIf* /*device*/)
        {
            LOG_WARNING("TODO: TextureImplVulkan::unmap not implemented");
            //vkUnmapMemory(device.device(), *m_memory.get());
        }

        const TextureDescription::Descriptor& TextureImplVulkan::description() const
        {
            return m_description;
        }

        VkImage& TextureImplVulkan::native()
        {
            return *m_image;
        }

        const VkImage& TextureImplVulkan::native() const
        {
            return *m_image;
        }

        ResourceState TextureImplVulkan::state(int slice, int mip) const
        {
            return m_state[mip + (slice * m_description.mipLevels)];
        }

        void TextureImplVulkan::state(int slice, int mip, ResourceState state)
        {
            m_state[mip + (slice * m_description.mipLevels)] = state;
        }

        TextureSRVImplVulkan::TextureSRVImplVulkan(
            const DeviceImplVulkan& device,
            const Texture& texture, 
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkImageView>(device.device(), vkDestroyImageView) }
            , m_texture{ texture }
            , m_subResources{ subResources }
            , m_uniqueId{ GlobalUniqueHandleId++ }
        {
            const VkDevice& dev = device.device();

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = static_cast<TextureImplVulkan*>(texture.m_impl)->native();
            viewInfo.flags = 0; // vulkanBufferUsageFlags(desc.descriptor.usage) | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            viewInfo.format = vulkanFormat(desc.descriptor.format);
			if (desc.descriptor.usage == ResourceUsage::DepthStencil)
			{
				if((desc.descriptor.format == Format::D32_FLOAT) || (desc.descriptor.format == Format::R32_FLOAT))
					viewInfo.format = vulkanFormat(Format::R32_TYPELESS);
				if (desc.descriptor.format == Format::D16_UNORM)
					viewInfo.format = vulkanFormat(Format::R16_UNORM);
			}
            viewInfo.subresourceRange = vulkanSubResource(texture, subResources);
            viewInfo.viewType = vulkanViewType(m_description.dimension);

            auto result = vkCreateImageView(dev, &viewInfo, nullptr, m_view.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
            debInfo.pObjectName = desc.descriptor.name;
            debInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
            ASSERT(result == VK_SUCCESS);
        }

        const TextureDescription::Descriptor& TextureSRVImplVulkan::description() const
        {
            return m_description;
        }

        Texture TextureSRVImplVulkan::texture() const
        {
            return m_texture;
        }

        Format TextureSRVImplVulkan::format() const
        {
            return m_description.format;
        }

        size_t TextureSRVImplVulkan::width() const
        {
            return m_description.width;
        }

        size_t TextureSRVImplVulkan::height() const
        {
            return m_description.height;
        }

        size_t TextureSRVImplVulkan::depth() const
        {
            return m_description.depth;
        }

        ResourceDimension TextureSRVImplVulkan::dimension() const
        {
            return m_description.dimension;
        }

        VkImageView& TextureSRVImplVulkan::native()
        {
            return *m_view;
        }

        const VkImageView& TextureSRVImplVulkan::native() const
        {
            return *m_view;
        }

        TextureUAVImplVulkan::TextureUAVImplVulkan(
            const DeviceImplVulkan& device,
            const Texture& texture,
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkImageView>(device.device(), vkDestroyImageView) }
            , m_texture{ texture }
            , m_subResources{ subResources }
            , m_uniqueId{ GlobalUniqueHandleId++ }
        {
            const VkDevice& dev = device.device();

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = static_cast<TextureImplVulkan*>(texture.m_impl)->native();
            viewInfo.flags = 0;
            viewInfo.format = vulkanFormat(desc.descriptor.format);
            viewInfo.subresourceRange = vulkanSubResource(texture, subResources);
            viewInfo.viewType = vulkanViewType(m_description.dimension);

            auto result = vkCreateImageView(dev, &viewInfo, nullptr, m_view.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
            debInfo.pObjectName = desc.descriptor.name;
            debInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
            ASSERT(result == VK_SUCCESS);
        }

        const TextureDescription::Descriptor& TextureUAVImplVulkan::description() const
        {
            return m_description;
        }

        Texture TextureUAVImplVulkan::texture() const
        {
            return m_texture;
        }

        Format TextureUAVImplVulkan::format() const
        {
            return m_description.format;
        }

        size_t TextureUAVImplVulkan::width() const
        {
            return m_description.width;
        }

        size_t TextureUAVImplVulkan::height() const
        {
            return m_description.height;
        }

        size_t TextureUAVImplVulkan::depth() const
        {
            return m_description.depth;
        }

        ResourceDimension TextureUAVImplVulkan::dimension() const
        {
            return m_description.dimension;
        }

        VkImageView& TextureUAVImplVulkan::native()
        {
            return *m_view;
        }

        const VkImageView& TextureUAVImplVulkan::native() const
        {
            return *m_view;
        }

        TextureDSVImplVulkan::TextureDSVImplVulkan(
            const DeviceImplVulkan& device,
            const Texture& texture,
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkImageView>(device.device(), vkDestroyImageView) }
            , m_texture{ texture }
            , m_subResources{ subResources }
        {
            const VkDevice& dev = device.device();

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = static_cast<TextureImplVulkan*>(texture.m_impl)->native();
            viewInfo.flags = 0;// vulkanBufferUsageFlags(desc.descriptor.usage) | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            viewInfo.format = vulkanFormat(desc.descriptor.format);
            viewInfo.subresourceRange = vulkanSubResource(texture, subResources);
            viewInfo.viewType = vulkanViewType(m_description.dimension);

            auto result = vkCreateImageView(dev, &viewInfo, nullptr, m_view.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
            debInfo.pObjectName = desc.descriptor.name;
            debInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
            ASSERT(result == VK_SUCCESS);
        }

        const TextureDescription::Descriptor& TextureDSVImplVulkan::description() const
        {
            return m_description;
        }

        Texture TextureDSVImplVulkan::texture() const
        {
            return m_texture;
        }

        Format TextureDSVImplVulkan::format() const
        {
            return m_description.format;
        }

        size_t TextureDSVImplVulkan::width() const
        {
            return m_description.width;
        }

        size_t TextureDSVImplVulkan::height() const
        {
            return m_description.height;
        }

        VkImageView& TextureDSVImplVulkan::native()
        {
            return *m_view;
        }

        const VkImageView& TextureDSVImplVulkan::native() const
        {
            return *m_view;
        }

        TextureRTVImplVulkan::TextureRTVImplVulkan(
            const DeviceImplVulkan& device,
            const Texture& texture,
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_view{ vulkanPtr<VkImageView>(device.device(), vkDestroyImageView) }
            , m_texture{ texture }
            , m_subResources{ subResources }
        {
            const VkDevice& dev = device.device();

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = static_cast<TextureImplVulkan*>(texture.m_impl)->native();
            viewInfo.flags = 0;// vulkanBufferUsageFlags(desc.descriptor.usage) | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            viewInfo.format = vulkanFormat(desc.descriptor.format);
            viewInfo.subresourceRange = vulkanSubResource(texture, subResources);
            viewInfo.viewType = vulkanViewType(m_description.dimension);

            auto result = vkCreateImageView(dev, &viewInfo, nullptr, m_view.get());
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_view.get());
            debInfo.pObjectName = desc.descriptor.name;
            debInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
            result = SetDebugUtilsObjectNameEXT(device.device(), &debInfo);
            ASSERT(result == VK_SUCCESS);
        }

        const TextureDescription::Descriptor& TextureRTVImplVulkan::description() const
        {
            return m_description;
        }

        Texture TextureRTVImplVulkan::texture() const
        {
            return m_texture;
        }

        Format TextureRTVImplVulkan::format() const
        {
            return m_description.format;
        }

        size_t TextureRTVImplVulkan::width() const
        {
            return m_description.width;
        }

        size_t TextureRTVImplVulkan::height() const
        {
            return m_description.height;
        }

        VkImageView& TextureRTVImplVulkan::native()
        {
            return *m_view;
        }

        const VkImageView& TextureRTVImplVulkan::native() const
        {
            return *m_view;
        }

        BindlessTextureSRVImplVulkan::BindlessTextureSRVImplVulkan(const DeviceImplVulkan& /*device*/)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_change{ true }
        {
        }

        bool BindlessTextureSRVImplVulkan::operator==(const BindlessTextureSRVImplVulkan& tex) const
        {
            return m_resourceId == tex.m_resourceId;
        }

        uint32_t BindlessTextureSRVImplVulkan::push(TextureSRVOwner texture)
        {
            //bool found = false;
            //for (int i = 0; i < m_textures.size(); ++i)
            //{
            //    if (m_textures[i].resource().resourceId() == texture.resource().resourceId())
            //    {
            //        return i;
            //    }
            //}

            auto res = m_textures.size();
            m_textures.emplace_back(texture);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessTextureSRVImplVulkan::size() const
        {
            return m_textures.size();
        }

        TextureSRV BindlessTextureSRVImplVulkan::get(size_t index)
        {
            return m_textures[index];
        }

        uint64_t BindlessTextureSRVImplVulkan::resourceId() const
        {
            return m_resourceId;
        }

        void BindlessTextureSRVImplVulkan::updateDescriptors(DeviceImplIf* /*device*/)
        {}

        bool BindlessTextureSRVImplVulkan::change() const
        {
            return m_change;
        }

        void BindlessTextureSRVImplVulkan::change(bool value)
        {
            m_change = value;
        }

        BindlessTextureUAVImplVulkan::BindlessTextureUAVImplVulkan(const DeviceImplVulkan& /*device*/)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_change{ true }
        {
        }

        bool BindlessTextureUAVImplVulkan::operator==(const BindlessTextureUAVImplVulkan& tex) const
        {
            return m_resourceId == tex.m_resourceId;
        }

        uint32_t BindlessTextureUAVImplVulkan::push(TextureUAVOwner texture)
        {
            auto res = m_textures.size();
            m_textures.emplace_back(texture);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessTextureUAVImplVulkan::size() const
        {
            return m_textures.size();
        }

        TextureUAV BindlessTextureUAVImplVulkan::get(size_t index)
        {
            return m_textures[index];
        }

        uint64_t BindlessTextureUAVImplVulkan::resourceId() const
        {
            return m_resourceId;
        }

        void BindlessTextureUAVImplVulkan::updateDescriptors(DeviceImplIf* /*device*/)
        {}

        bool BindlessTextureUAVImplVulkan::change() const
        {
            return m_change;
        }

        void BindlessTextureUAVImplVulkan::change(bool value)
        {
            m_change = value;
        }
    }
}
