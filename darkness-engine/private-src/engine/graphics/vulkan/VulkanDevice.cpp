#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanSwapChain.h"
#include "engine/graphics/vulkan/VulkanCommandList.h"
#include "engine/graphics/vulkan/VulkanDescriptorHandle.h"
#include "engine/graphics/vulkan/VulkanCommandAllocator.h"

#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/GpuMarkerStorage.h"

#include "platform/Platform.h"
#ifdef _WIN32
#include "platform/window/windows/WindowsWindow.h"
#endif
#include "tools/Debug.h"
#include "containers/vector.h"
#include <algorithm>
#include <set>
#include "tools/image/Image.h"

using namespace platform::implementation;
using namespace engine;
using namespace tools;

namespace engine
{
    namespace implementation
    {
        void createWindowSurface(
            VkInstance instance,
            const VkWin32SurfaceCreateInfoKHR& createInfo,
            VkSurfaceKHR* surface)
        {
            auto createWin32SurfaceKHR = 
                reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(
                reinterpret_cast<void*>(vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR")));

            ASSERT(createWin32SurfaceKHR != nullptr);
            ASSERT(createWin32SurfaceKHR(instance, &createInfo, nullptr, surface) == VK_SUCCESS);
        }

        //VulkanInstance DeviceImplVulkan::m_instance;

        DeviceImplVulkan::DeviceImplVulkan(engine::shared_ptr<platform::Window> window)
            : m_mutex{}
            , m_physicalDevice{ VK_NULL_HANDLE }
            , m_uploadFence{ nullptr }
            , m_deviceQueue{ nullptr }
            , m_device{ vulkanPtr<VkDevice>( vkDestroyDevice) }
            , m_surface{ vulkanPtr<VkSurfaceKHR>(m_instance.instance(), vkDestroySurfaceKHR ) }
            , m_window{ window }
            , m_requiredExtensions{ 
				VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
				VK_KHR_MAINTENANCE1_EXTENSION_NAME,
				VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
                VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
                //VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
                VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME } //, VK_KHR_DISPLAY_EXTENSION_NAME }
            , m_allocator{ nullptr }
            
            , m_fencePool{ 
                [&]()->VkSemaphore
                {
                    VkSemaphoreTypeCreateInfo timelineCreateInfo;
                    timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                    timelineCreateInfo.pNext = NULL;
                    timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
                    timelineCreateInfo.initialValue = 0;

                    VkSemaphoreCreateInfo createInfo;
                    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                    createInfo.pNext = &timelineCreateInfo;
                    createInfo.flags = 0;

                    VkSemaphore timelineSemaphore;
                    auto result = vkCreateSemaphore(*m_device, &createInfo, NULL, &timelineSemaphore);
                    ASSERT(result == VK_SUCCESS);

                    return timelineSemaphore;
                },
                [&](VkSemaphore /*fence*/)
                {
                    LOG("Vulkan reset fence not implemented");
                    //vkResetSemaphore(*m_device, 1, &fence);
                },
                [&](VkSemaphore fence)
                {
                    vkDestroySemaphore(*m_device, fence, nullptr);
                }
            }
            , m_descriptorHeap{ nullptr }
            , m_currentFenceValue{ 0 }
        {
            createSurface();

            uint32_t deviceCount{ 0 };
            vkEnumeratePhysicalDevices(m_instance.instance(), &deviceCount, nullptr);
            ASSERT(deviceCount > 0);

            vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(m_instance.instance(), &deviceCount, devices.data());
            for (auto& device : devices)
            {
                if (isSuitableDevice(device))
                {
                    m_physicalDevice = device;
                    break;
                }
            }

            ASSERT(m_physicalDevice != VK_NULL_HANDLE);

            createLogicalDevice();

            m_allocator = engine::make_shared<CommandAllocatorImplVulkan>(*this, CommandListType::Direct);

            m_descriptorHeap = engine::make_unique<DescriptorHeapImplVulkan>(*this);

            m_uploadBuffer = BufferOwner(
                static_pointer_cast<BufferImplIf>(engine::make_shared<BufferImplVulkan>(
                    *this,
                    BufferDescription()
                    .usage(ResourceUsage::Upload)
                    .elementSize(1)
                    .elements(UploadBufferSizeBytes)
                    .name("UploadBuffer"))),
                [&](engine::shared_ptr<BufferImplIf> im)
                {
                    LOG("Cursious if this just works. GPU should be empty by now");
                    //m_returnedBuffers.push(ReturnedResourceBuffer{ im, m_submitFence.currentCPUValue() });
                });

            Buffer temp(m_uploadBuffer.resource());
            auto ptr = temp.m_impl->map(this);

            m_uploadAllocator = RingBuffer(
                ByteRange{
                    static_cast<uint8_t*>(ptr),
                    static_cast<uint8_t*>(ptr) + UploadBufferSizeBytes });

#if 0
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, VK_FORMAT_BC7_UNORM_BLOCK, &formatProperties);

            //formatProperties.linearTilingFeatures
            auto printFeatureSupport = [&](VkFormatFeatureFlags flags)
            {
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT");
                if (flags & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) LOG("VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT");
                if (flags & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT) LOG("VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT");
                if (flags & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) LOG("VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT");
                if (flags & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) LOG("VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT");
                if (flags & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT) LOG("VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT");
                if (flags & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) LOG("VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT");
                if (flags & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) LOG("VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT");
                if (flags & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) LOG("VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT");
                if (flags & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) LOG("VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT");
                if (flags & VK_FORMAT_FEATURE_BLIT_SRC_BIT) LOG("VK_FORMAT_FEATURE_BLIT_SRC_BIT");
                if (flags & VK_FORMAT_FEATURE_BLIT_DST_BIT) LOG("VK_FORMAT_FEATURE_BLIT_DST_BIT");
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT");
                if (flags & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) LOG("VK_FORMAT_FEATURE_TRANSFER_SRC_BIT");
                if (flags & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) LOG("VK_FORMAT_FEATURE_TRANSFER_DST_BIT");
                if (flags & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT) LOG("VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT");
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT");
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT");
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT");
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT");
                if (flags & VK_FORMAT_FEATURE_DISJOINT_BIT) LOG("VK_FORMAT_FEATURE_DISJOINT_BIT");
                if (flags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT) LOG("VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT");
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG");
                if (flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT) LOG("VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT");
                if (flags & VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT) LOG("VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT");
                if (flags & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) LOG("VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR");
                if (flags & VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR) LOG("VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR");
                if (flags & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT_KHR) LOG("VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT_KHR");
            };

            LOG("bufferFeatures");
            printFeatureSupport(formatProperties.bufferFeatures);

            LOG("linearTilingFeatures");
            printFeatureSupport(formatProperties.linearTilingFeatures);

            LOG("optimalTilingFeatures");
            printFeatureSupport(formatProperties.optimalTilingFeatures);
#endif
        }

        void DeviceImplVulkan::createFences(Device& device)
        {
            m_uploadFence = engine::make_shared<Fence>(device.createFence("Upload fence"));
            m_deviceQueue = &device.queue();
        }

        engine::shared_ptr<CommandAllocatorImplVulkan> DeviceImplVulkan::allocator() const
        {
            return m_allocator;
        }

        void DeviceImplVulkan::processUploads(engine::FenceValue value, bool force)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (force)
            {
                for (auto&& upload : m_uploadAllocations)
                {
                    m_uploadAllocator.free(upload.ptr);
                }
                m_uploadAllocations.clear();
                return;
            }
            auto upload = m_uploadAllocations.begin();
            while (upload != m_uploadAllocations.end())
            {
                if ((*upload).value <= value)
                {
                    m_uploadAllocator.free((*upload).ptr);
                    upload = m_uploadAllocations.erase(upload);
                }
                else
                    ++upload;
            }
        }

        void DeviceImplVulkan::setCurrentFenceValue(CommandListType type, engine::FenceValue value)
        {
            if (type == CommandListType::Direct)
                m_currentFenceValue = value;
            else if (type == CommandListType::Copy)
                m_currentCopyFenceValue = value;
        }

        void DeviceImplVulkan::uploadRawBuffer(CommandListImplIf* commandList, Buffer buffer, const ByteRange& data, size_t startBytes)
        {
            //auto uploadData = m_uploadAllocator.allocate(data.sizeBytes(), 4);
            auto uploadData = m_uploadAllocator.allocate(data.sizeBytes(), 4);

            if (!uploadData.ptr)
            {
                m_uploadFence->increaseCPUValue();
                m_deviceQueue->signal(*m_uploadFence, m_uploadFence->currentCPUValue());
                m_uploadFence->blockUntilSignaled();

                processUploads(0, true);
                m_uploadAllocator.reset();
                uploadData = m_uploadAllocator.allocate(data.sizeBytes(), 4);
                if (!uploadData.ptr)
                {
                    ASSERT(false, "Ran out of memory");
                }
            }

            memcpy(uploadData.ptr, reinterpret_cast<uint8_t*>(data.start), data.sizeBytes());
            commandList->transition(buffer, ResourceState::CopyDest);
            commandList->applyBarriers();

            VkBufferCopy bufferCopy;
            bufferCopy.dstOffset = startBytes;
            bufferCopy.srcOffset = m_uploadAllocator.offset(uploadData.ptr);
            bufferCopy.size = data.sizeBytes();

            vkCmdCopyBuffer(
                static_cast<CommandListImplVulkan*>(commandList)->native(),
                static_cast<BufferImplVulkan*>(m_uploadBuffer.resource().m_impl)->native(),
                static_cast<BufferImplVulkan*>(buffer.m_impl)->native(), 1, &bufferCopy);
        }

        void DeviceImplVulkan::uploadBuffer(CommandList& commandList, BufferSRV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(*static_cast<CommandListImplVulkan*>(commandList.native()), buffer, data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandListImplVulkan& commandList, BufferSRV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandList& commandList, BufferUAV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(*static_cast<CommandListImplVulkan*>(commandList.native()), buffer, data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandListImplVulkan& commandList, BufferUAV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandList& commandList, BufferCBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(*static_cast<CommandListImplVulkan*>(commandList.native()), buffer, data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandListImplVulkan& commandList, BufferCBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandList& commandList, BufferIBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(*static_cast<CommandListImplVulkan*>(commandList.native()), buffer, data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandListImplVulkan& commandList, BufferIBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandList& commandList, BufferVBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(*static_cast<CommandListImplVulkan*>(commandList.native()), buffer, data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandListImplVulkan& commandList, BufferVBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBuffer(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplVulkan::uploadBuffer(CommandListImplVulkan& commandList, Buffer buffer, const tools::ByteRange& data, size_t startElement)
        {
            //auto uploadData = m_uploadAllocator.allocate(data.sizeBytes(), 4);
            auto uploadData = m_uploadAllocator.allocate(data.sizeBytes(), 4);

            if (!uploadData.ptr)
            {
                m_uploadFence->increaseCPUValue();
                m_deviceQueue->signal(*m_uploadFence, m_uploadFence->currentCPUValue());
                m_uploadFence->blockUntilSignaled();

                processUploads(0, true);
                m_uploadAllocator.reset();
                uploadData = m_uploadAllocator.allocate(data.sizeBytes(), 4);
                if (!uploadData.ptr)
                {
                    ASSERT(false, "Ran out of memory");
                }
            }

            memcpy(uploadData.ptr, reinterpret_cast<uint8_t*>(data.start), data.sizeBytes());
            commandList.transition(buffer, ResourceState::CopyDest);
            commandList.applyBarriers();

            VkBufferCopy bufferCopy;
            bufferCopy.dstOffset = startElement * buffer.description().elementSize;
            bufferCopy.srcOffset = m_uploadAllocator.offset(uploadData.ptr);
            bufferCopy.size = data.sizeBytes();

            vkCmdCopyBuffer(
                commandList.native(), 
                static_cast<BufferImplVulkan*>(m_uploadBuffer.resource().m_impl)->native(), 
                static_cast<BufferImplVulkan*>(buffer.m_impl)->native(), 1, &bufferCopy);
        }

        engine::shared_ptr<TextureImplIf> DeviceImplVulkan::createTexture(const Device& device, Queue& queue, const TextureDescription& desc)
        {
            auto descmod = desc;
            if (isBlockCompressedFormat(descmod.descriptor.format))
            {
                if (descmod.descriptor.width < 4)
                    descmod.descriptor.width = 4;
                if (descmod.descriptor.height < 4)
                    descmod.descriptor.height = 4;
            }

            if (descmod.initialData.data.size() > 0)
            {
                auto gpuDesc = descmod;
                auto gpuTexture = createTexture(device, queue, gpuDesc
                    .usage(ResourceUsage::GpuRead)
                    .setInitialData(TextureDescription::InitialData()));
				TextureOwner tex(gpuTexture,
					[&](engine::shared_ptr<TextureImplIf> im)
					{
						device.m_returnedTextures.push(Device::ReturnedResourceTexture{ im, device.m_submitFence.currentCPUValue() });
					});
                auto cmd = device.createCommandList("DeviceImplVulkan::createTexture");

                size_t dataIndex = 0;
                for (size_t slice = 0; slice < desc.descriptor.arraySlices; ++slice)
                {
                    size_t width = desc.descriptor.width;
                    size_t height = desc.descriptor.height;

                    for (size_t mip = 0; mip < desc.descriptor.mipLevels; ++mip)
                    {
                        auto info = surfaceInformation(desc.descriptor.format, width, height);
						//auto uploadData = m_uploadAllocator.allocate(info.numBytes, formatBytes(desc.descriptor.format));
                        auto uploadData = m_uploadAllocator.allocate(info.numBytes, formatBytes(desc.descriptor.format));

                        if (!uploadData.ptr)
                        {
                            m_uploadFence->increaseCPUValue();
                            m_deviceQueue->signal(*m_uploadFence, m_uploadFence->currentCPUValue());
                            m_uploadFence->blockUntilSignaled();

                            processUploads(0, true);
                            m_uploadAllocator.reset();
                            uploadData = m_uploadAllocator.allocate(info.numBytes, formatBytes(desc.descriptor.format));
                            if (!uploadData.ptr)
                            {
                                ASSERT(false, "Ran out of memory");
                            }
                        }
                        {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            m_uploadAllocations.emplace_back(UploadAllocation{ uploadData, m_currentFenceValue });
                        }

						// memcpy the buffer data to upload buffer
						const uint8_t* srcData = &descmod.initialData.data[dataIndex];
						for (int i = 0; i < static_cast<int>(info.numRows); ++i)
						{
							//UINT8* pScan = static_cast<uint8_t*>(uploadData) + (i * info.rowBytes);
                            UINT8* pScan = static_cast<uint8_t*>(uploadData.ptr) + (i * info.rowBytes);
							memcpy(pScan, srcData, info.rowBytes);
							srcData += info.rowBytes;
						}
						dataIndex += info.numBytes;

						// copy from upload buffer to image
						VkBufferImageCopy region = {};
						region.bufferOffset = m_uploadAllocator.offset(uploadData.ptr);
						region.bufferRowLength = 0;
						region.bufferImageHeight = 0;
						region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						region.imageSubresource.mipLevel = static_cast<uint32_t>(mip);
						region.imageSubresource.baseArrayLayer = static_cast<uint32_t>(slice);
						region.imageSubresource.layerCount = 1;

						region.imageOffset = { 0, 0, 0 };
						region.imageExtent = {
							static_cast<uint32_t>(width),
							static_cast<uint32_t>(height),
							static_cast<uint32_t>(1)
						};

						cmd.transition(tex, ResourceState::CopyDest, { static_cast<uint32_t>(mip), 1, static_cast<uint32_t>(slice), 1 });
						static_cast<CommandListImplVulkan*>(cmd.native())->applyBarriers();
						vkCmdCopyBufferToImage(
                            static_cast<CommandListImplVulkan*>(cmd.native())->native(),
							static_cast<BufferImplVulkan*>(m_uploadBuffer.resource().m_impl)->native(),
							static_cast<TextureImplVulkan*>(gpuTexture.get())->native(),
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							1,
							&region
						);

                        width /= 2;
                        height /= 2;
                        /*if (isBlockCompressedFormat(desc.descriptor.format))
                        {
                            if (width < 4)
                                width = 4;
                            if (height < 4)
                                height = 4;
                        }
                        else
                        {
                            if (width < 1)
                                width = 1;
                            if (height < 1)
                                height = 1;
                        }*/
                    }
                }
				const_cast<Device*>(&device)->submitBlocking(cmd);
                return gpuTexture;
            }
            else
                return engine::make_shared<TextureImplVulkan>(*this, desc);
        }

        engine::shared_ptr<CommandAllocatorImplIf> DeviceImplVulkan::createCommandAllocator(CommandListType type, const char* /*name*/)
        {
            if (type == CommandListType::Direct || type == CommandListType::Compute)
            {
                if (m_commandAllocatorsDirect.size() > 0)
                {
                    auto res = m_commandAllocatorsDirect.front();
                    m_commandAllocatorsDirect.pop();
                    m_inUseCommandAllocatorsDirect.emplace_back(res);
                    return res;
                }

                m_inUseCommandAllocatorsDirect.emplace_back(engine::make_shared<CommandAllocatorImplVulkan>(*this, type));
                return m_inUseCommandAllocatorsDirect.back();
            }
            else if (type == CommandListType::Copy)
            {
                if (m_commandAllocatorsCopy.size() > 0)
                {
                    auto res = m_commandAllocatorsCopy.front();
                    m_commandAllocatorsCopy.pop();
                    m_inUseCommandAllocatorsCopy.emplace_back(res);
                    return res;
                }

                m_inUseCommandAllocatorsCopy.emplace_back(engine::make_shared<CommandAllocatorImplVulkan>(*this, type));
                return m_inUseCommandAllocatorsCopy.back();
            }
            else
                ASSERT(false, "Not implemented");
            return {};
        }

        void DeviceImplVulkan::nullResources(engine::shared_ptr<NullResources> nullResources)
        {
            m_nullResources = nullResources;
            if (!nullResources)
                m_grabBuffer = {};
        }

        NullResources& DeviceImplVulkan::nullResources()
        {
            return *m_nullResources;
        }

        void DeviceImplVulkan::freeCommandAllocator(engine::shared_ptr<CommandAllocatorImplIf> allocator)
        {
            if (allocator->type() == CommandListType::Direct || allocator->type() == CommandListType::Compute)
            {
                m_inUseCommandAllocatorsDirect.erase(std::find(m_inUseCommandAllocatorsDirect.begin(), m_inUseCommandAllocatorsDirect.end(), allocator));
                m_returnedCommandAllocatorsDirect.emplace_back(static_pointer_cast<CommandAllocatorImplVulkan>(allocator));
            }
            else if (allocator->type() == CommandListType::Copy)
            {
                m_inUseCommandAllocatorsCopy.erase(std::find(m_inUseCommandAllocatorsCopy.begin(), m_inUseCommandAllocatorsCopy.end(), allocator));
                m_returnedCommandAllocatorsCopy.emplace_back(static_pointer_cast<CommandAllocatorImplVulkan>(allocator));
            }
            else
                ASSERT(false, "Not implemented");
        }

        void DeviceImplVulkan::createSurface()
        {
            VkWin32SurfaceCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            createInfo.hwnd = platform::implementation::WindowImplGet::impl(*m_window.get()).handle();
            createInfo.hinstance = GetModuleHandle(nullptr);
            createWindowSurface(m_instance.instance(), createInfo, m_surface.get());
        }

        bool DeviceImplVulkan::isSuitableDevice(const VkPhysicalDevice& device) const
        {
            if (!checkExtensionSupport(device))
                return false;

            SwapChainImplVulkan::SwapChainDetails swapChainDetails = SwapChainImplVulkan::getDetails(device, *m_surface);
            if (swapChainDetails.formats.empty() || swapChainDetails.presentModes.empty())
                return false;

            auto deviceQueues = enumerateQueues(device);
            for (auto queue : deviceQueues)
            {
                if ((queue.flags & VK_QUEUE_GRAPHICS_BIT) && queue.presentSupport)
                    return true;
            }
            return false;
        }

        bool DeviceImplVulkan::checkExtensionSupport(const VkPhysicalDevice& device) const
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<engine::string> requiredExtensionSet(m_requiredExtensions.begin(), m_requiredExtensions.end());

            for (const auto& extension : availableExtensions) {
                LOG("Device extension: %s", extension.extensionName);
                requiredExtensionSet.erase(extension.extensionName);
            }

            return requiredExtensionSet.empty();
        }

        const engine::vector<VulkanQueue>& DeviceImplVulkan::deviceQueues() const
        {
            return queues;
        }

        void DeviceImplVulkan::createLogicalDevice()
        {
            queues = enumerateQueues(m_physicalDevice);

            vector<VkDeviceQueueCreateInfo> createInfos;
            for (const auto& queue : queues)
            {
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(queue.familyIndex);
                queueCreateInfo.queueCount = static_cast<uint32_t>(queue.available);
                queueCreateInfo.pQueuePriorities = queue.priority.data();
                createInfos.emplace_back(queueCreateInfo);
            }
            
            VkPhysicalDeviceFeatures deviceFeatures = {};
			deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
			deviceFeatures.multiDrawIndirect = VK_TRUE;
			deviceFeatures.textureCompressionBC = VK_TRUE;
            deviceFeatures.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
            deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
            deviceFeatures.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
            deviceFeatures.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
            deviceFeatures.depthClamp = VK_TRUE;
            deviceFeatures.depthBiasClamp = VK_TRUE;
            deviceFeatures.robustBufferAccess = VK_TRUE;

            VkPhysicalDeviceVulkan12Features vk12Features = {};
            vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            vk12Features.timelineSemaphore = VK_TRUE;
            vk12Features.descriptorIndexing = VK_TRUE;
            vk12Features.drawIndirectCount = VK_TRUE;
            vk12Features.runtimeDescriptorArray = VK_TRUE;
            vk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            vk12Features.imagelessFramebuffer = VK_TRUE;
            vk12Features.descriptorBindingPartiallyBound = VK_TRUE;
            vk12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;

            VkPhysicalDeviceRobustness2FeaturesEXT robust = {};
            robust.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
            robust.nullDescriptor = VK_TRUE;
            robust.robustBufferAccess2 = VK_TRUE;
            robust.robustImageAccess2 = VK_TRUE;
            vk12Features.pNext = &robust;

            //descriptorBindingPartiallyBound

            /*VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline = {};
            timeline.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
            timeline.timelineSemaphore = VK_TRUE;
            timeline.pNext = &vk12Features;*/

			/*VkPhysicalDeviceDescriptorIndexingFeaturesEXT ext = {};
			ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
			ext.runtimeDescriptorArray = VK_TRUE;
            ext.pNext = &timeline;*/

            VkDeviceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = createInfos.data();
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(createInfos.size());
            createInfo.pEnabledFeatures = &deviceFeatures;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensions.size());
            createInfo.ppEnabledExtensionNames = m_requiredExtensions.data();
            createInfo.enabledLayerCount = 0;
#ifndef NDEBUG
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_instance.validationLayers().size());
            createInfo.ppEnabledLayerNames = m_instance.validationLayers().data();
#endif
			createInfo.pNext = &vk12Features;

            auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, m_device.get());
            ASSERT(result == VK_SUCCESS);
        }

        vector<VulkanQueue> DeviceImplVulkan::enumerateQueues(const VkPhysicalDevice& device) const
        {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            engine::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            vector<VulkanQueue> result;
            for (int i = 0; i < static_cast<int>(queueFamilies.size()); ++i)
            {
                engine::vector<float> priorities(queueFamilies[static_cast<std::size_t>(i)].queueCount);
                float decreasePriority = 1.0f / static_cast<float>(priorities.size());
                float currentPriority = 1.0f;
                for (auto& priority : priorities)
                {
                    priority = currentPriority;
                    currentPriority -= decreasePriority;
                }


                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(i), *m_surface, &presentSupport);

                VkQueueFamilyProperties& que = queueFamilies[static_cast<std::size_t>(i)];
                result.emplace_back(VulkanQueue{
                    i,
                    static_cast<int>(que.queueCount),
                    que.queueFlags,
                    presentSupport == VK_TRUE,
                    priorities,
                    vector<bool>(priorities.size(), false),
                    engine::Vector3<size_t>{ 
                      static_cast<size_t>(que.minImageTransferGranularity.width),
                      static_cast<size_t>(que.minImageTransferGranularity.height),
                      static_cast<size_t>(que.minImageTransferGranularity.depth) }
                    });
            }
            return result;
        }

        QueueInfo DeviceImplVulkan::createQueue(CommandListType type) const
        {
            //for (auto& queue : queues)
            for (auto q = queues.rbegin(); q != queues.rend(); ++q)
            {
                auto& queue = *q;
                if((queue.flags & vulkanCommandListType(type)) &&
                    queue.available > 0)
                {
                    --queue.available;
                    for (int i = 0; i < static_cast<int>(queue.taken.size()); ++i)
                    {
                        if (!queue.taken[static_cast<std::size_t>(i)])
                        {
                            queue.taken[static_cast<std::size_t>(i)] = true;
                            return QueueInfo{ queue.familyIndex, i, queue.minImageTransferGranularity };
                        }
                    }
                    ASSERT(false);
                }
            }
            ASSERT(false);
            return QueueInfo{ InvalidFamilyIndex };
        }

        void DeviceImplVulkan::destroyQueue(QueueInfo commandQueue) const
        {
            for (auto& queue : queues)
            {
                if (queue.familyIndex == commandQueue.queueFamilyIndex)
                {
                    ++queue.available;
                    queue.taken[static_cast<std::size_t>(commandQueue.queueIndex)] = false;
                    return;
                }
            }
            ASSERT(false);
        }

        const VkPhysicalDevice DeviceImplVulkan::physicalDevice() const
        {
            return m_physicalDevice;
        }

        const VkDevice& DeviceImplVulkan::device() const
        {
            return *m_device;
        }

        const VkSurfaceKHR& DeviceImplVulkan::surface() const
        {
            return *m_surface;
        }

        const platform::Window& DeviceImplVulkan::window() const
        {
            return *m_window;
        }

        void DeviceImplVulkan::window(engine::shared_ptr<platform::Window> window)
        {
            m_window = window;
        }

        int DeviceImplVulkan::width() const
        {
            return WindowImplGet::impl(*m_window).width();
        }

        int DeviceImplVulkan::height() const
        {
            return WindowImplGet::impl(*m_window).height();
        }

        void DeviceImplVulkan::waitForIdle()
        {
            vkDeviceWaitIdle(*m_device);
        }

        DescriptorHeapImplVulkan& DeviceImplVulkan::descriptorHeap()
        {
            return *m_descriptorHeap;
        }

        engine::unique_ptr<GpuMarkerContainer> DeviceImplVulkan::getMarkerContainer()
        {
            return {};
        };

        void DeviceImplVulkan::returnMarkerContainer(engine::unique_ptr<GpuMarkerContainer>&& /*container*/)
        {
            ASSERT(false, "DeviceImplVulkan::returnMarkerContainer not implemented");
        };
        
        TextureBufferCopyDesc DeviceImplVulkan::getTextureBufferCopyDesc(size_t width, size_t height, Format format)
        {
            TextureBufferCopyDesc result;
            result.elementSize = engine::formatBytes(format);
            result.elements = width * height;
            result.bufferSize = result.elementSize * result.elements;
            result.width = width;
            result.height = height;
            result.pitch = width;
            result.pitchBytes = result.pitch * formatBytes(format);
            result.format = format;
            result.zeroUp = false;
            return result;
        }

        CpuTexture DeviceImplVulkan::grabTexture(Device& device, TextureSRV texture)
        {
            auto elementSize = engine::formatBytes(texture.format());
            auto elements = texture.width() * texture.height();
            auto bufferSize = elements * elementSize;

            if (!m_grabBuffer.resource().valid() ||
                (m_grabBuffer.resource().desc().elements * m_grabBuffer.resource().desc().elementSize) != bufferSize)
            {
                m_grabBuffer = device.createBufferSRV(engine::BufferDescription()
                    .format(texture.format())
                    .elementSize(elementSize)
                    .elements(elements)
                    .name("Frame capture")
                    .usage(engine::ResourceUsage::GpuToCpu)
                );
            }

            CommandList texcmdb = device.createCommandList("DeviceImplDX12::createTexture");
            texcmdb.copyTexture(texture, m_grabBuffer.resource());
            device.submitBlocking(texcmdb);


            CpuTexture result;
            result.width = texture.width();
            result.height = texture.height();
            result.pitch = texture.width();
            result.pitchBytes = result.pitch * formatBytes(texture.format());
            result.format = texture.format();
            result.zeroUp = false;
            result.data = engine::make_shared<vector<uint8_t>>();

            auto ptr = m_grabBuffer.resource().buffer().map(device);
            result.data->resize(bufferSize);
            memcpy(result.data->data(), ptr, bufferSize);
            m_grabBuffer.resource().buffer().unmap(device);

            return result;

            //auto grabBuffer = device.createBufferSRV(engine::BufferDescription()
            //    .format(texture.format())
            //    .elementSize(engine::formatBytes(texture.format()))
            //    .elements(texture.width() * texture.height())
            //    .name("Frame capture")
            //    .usage(engine::ResourceUsage::GpuToCpu)
            //);
            //
            //CommandList texcmdb = device.createCommandList("DeviceImplDX12::createTexture");
            //texcmdb.copyTexture(texture, grabBuffer.resource());
            //device.submitBlocking(texcmdb);
            //
            //
            //CpuTexture result;
            //result.width = texture.width();
            //result.height = texture.height();
            //result.pitch = texture.width();
            //result.pitchBytes = result.pitch * formatBytes(texture.format());
            //result.format = texture.format();
            //result.zeroUp = false;
            //
            //auto bufferSize = grabBuffer.resource().desc().elements * grabBuffer.resource().desc().elementSize;
            //auto ptr = grabBuffer.resource().buffer().map(device);
            //result.data.resize(bufferSize);
            //memcpy(result.data.data(), ptr, bufferSize);
            //grabBuffer.resource().buffer().unmap(device);
            //
            //return result;
        }

        TextureSRVOwner DeviceImplVulkan::loadTexture(Device& device, const CpuTexture& texture)
        {
            auto tex = texture.tightPack();
            if (!texture.zeroUp)
                return device.createTextureSRV(TextureDescription()
                    .name("Device loadTexture srv")
                    .width(static_cast<uint32_t>(tex.width))
                    .height(static_cast<uint32_t>(tex.height))
                    .format(tex.format)
                    .arraySlices(static_cast<uint32_t>(1))
                    .mipLevels(1)
                    .setInitialData(TextureDescription::InitialData(
                        *tex.data,
                        static_cast<uint32_t>(tex.pitch),
                        static_cast<uint32_t>(tex.pitch * tex.height))));
            else
            {
                auto flipped = tex.flipYAxis();
                return device.createTextureSRV(TextureDescription()
                    .name("Device loadTexture srv")
                    .width(static_cast<uint32_t>(flipped.width))
                    .height(static_cast<uint32_t>(flipped.height))
                    .format(flipped.format)
                    .arraySlices(static_cast<uint32_t>(1))
                    .mipLevels(1)
                    .setInitialData(TextureDescription::InitialData(
                        *flipped.data,
                        static_cast<uint32_t>(flipped.pitch),
                        static_cast<uint32_t>(flipped.pitch * flipped.height))));
            }

            //if (!texture.zeroUp)
            //    return device.createTextureSRV(TextureDescription()
            //        .name("color")
            //        .width(static_cast<uint32_t>(texture.width))
            //        .height(static_cast<uint32_t>(texture.height))
            //        .format(texture.format)
            //        .arraySlices(static_cast<uint32_t>(1))
            //        .mipLevels(1)
            //        .setInitialData(TextureDescription::InitialData(
            //            texture.data,
            //            static_cast<uint32_t>(texture.pitch),
            //            static_cast<uint32_t>(texture.pitch * texture.height))));
            //else
            //{
            //    auto flipped = texture.flipYAxis();
            //    return device.createTextureSRV(TextureDescription()
            //        .name("color")
            //        .width(static_cast<uint32_t>(flipped.width))
            //        .height(static_cast<uint32_t>(flipped.height))
            //        .format(flipped.format)
            //        .arraySlices(static_cast<uint32_t>(1))
            //        .mipLevels(1)
            //        .setInitialData(TextureDescription::InitialData(
            //            flipped.data,
            //            static_cast<uint32_t>(flipped.pitch),
            //            static_cast<uint32_t>(flipped.pitch * flipped.height))));
            //}
        }

        void DeviceImplVulkan::copyTexture(Device& device, const CpuTexture& tex, TextureSRV dsttex)
        {
            auto texture = tex.tightPack();
            auto width = texture.width;
            auto height = texture.height;
            auto mip = 0;
            auto slice = 0;

            auto cmd = device.createCommandList("DeviceImplVulkan::createTexture");

            TextureDescription desc;
            desc
                .arraySlices(1)
                .depth(1)
                .dimension(ResourceDimension::Texture2D)
                .format(texture.format)
                .width(texture.width)
                .height(texture.height)
                .mipLevels(1)
                .name("copy tex cpu gpu")
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 0.0f })
                .optimizedDepthClearValue(0)
                .optimizedStencilClearValue(0)
                .samples(1)
                .usage(ResourceUsage::Upload);


            auto info = surfaceInformation(desc.descriptor.format, width, height);
            //auto uploadData = m_uploadAllocator.allocate(info.numBytes, formatBytes(desc.descriptor.format));
            auto uploadData = m_uploadAllocator.allocate(info.numBytes, formatBytes(desc.descriptor.format));

            if (!uploadData.ptr)
            {
                m_uploadFence->increaseCPUValue();
                m_deviceQueue->signal(*m_uploadFence, m_uploadFence->currentCPUValue());
                m_uploadFence->blockUntilSignaled();

                processUploads(0, true);
                m_uploadAllocator.reset();
                uploadData = m_uploadAllocator.allocate(info.numBytes, formatBytes(desc.descriptor.format));
                if (!uploadData.ptr)
                {
                    ASSERT(false, "Ran out of memory");
                }
            }
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_uploadAllocations.emplace_back(UploadAllocation{ uploadData, m_currentFenceValue });
            }

            // memcpy the buffer data to upload buffer
            const uint8_t* srcData = texture.data->data();
            for (int i = 0; i < static_cast<int>(info.numRows); ++i)
            {
                //UINT8* pScan = static_cast<uint8_t*>(uploadData) + (i * info.rowBytes);
                UINT8* pScan = static_cast<uint8_t*>(uploadData.ptr) + (i * info.rowBytes);
                memcpy(pScan, srcData, info.rowBytes);
                srcData += info.rowBytes;
            }

            // copy from upload buffer to image
            VkBufferImageCopy region = {};
            region.bufferOffset = m_uploadAllocator.offset(uploadData.ptr);
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mip;
            region.imageSubresource.baseArrayLayer = slice;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                static_cast<uint32_t>(1)
            };

            cmd.transition(dsttex.texture(), ResourceState::CopyDest, { static_cast<uint32_t>(mip), 1, static_cast<uint32_t>(slice), 1 });
            static_cast<CommandListImplVulkan*>(cmd.native())->applyBarriers();
            vkCmdCopyBufferToImage(
                static_cast<CommandListImplVulkan*>(cmd.native())->native(),
                static_cast<BufferImplVulkan*>(m_uploadBuffer.resource().m_impl)->native(),
                static_cast<TextureImplVulkan*>(dsttex.texture().m_impl)->native(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );

            const_cast<Device*>(&device)->submitBlocking(cmd);
        }

    }
}
