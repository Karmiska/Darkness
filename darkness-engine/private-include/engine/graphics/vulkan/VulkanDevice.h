#pragma once

#include "engine/graphics/DeviceImplIf.h"
#include "engine/graphics/vulkan/VulkanInstance.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanCommon.h"
#include "engine/graphics/vulkan/VulkanFencePool.h"
#include "engine/graphics/vulkan/VulkanDescriptorHeap.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/ResourceOwners.h"
#include "tools/RingBuffer.h"
#include "tools/ByteRange.h"
#include "containers/memory.h"
#include <map>
#include "containers/string.h"
#include <queue>
#include <mutex>
//struct ID3D12Device;

namespace platform
{
    class Window;
}

namespace engine
{
    class Buffer;
    class BufferSRV;
    class BufferUAV;
    class BufferIBV;
    class BufferCBV;
    class BufferVBV;
    class Texture;
    class Queue;
    class Device;
    class CommandList;
    class DescriptorHandle;
    struct NullResources;
    enum class Format;
    struct TextureDescription;

    namespace implementation
    {
        class CommandAllocatorImplVulkan;
        class CommandListImplVulkan;
        class TextureImplVulkan;
        class FenceImplVulkan;

        struct QueueInfo
        {
            int queueFamilyIndex;
            int queueIndex;
            engine::Vector3<size_t> minImageTransferGranularity;
        };
        static const int InvalidFamilyIndex = -1;

        constexpr uint32_t UploadBufferSizeBytes = 1024u * 1024u * 1000u;

        class DeviceImplVulkan : public DeviceImplIf
        {
        public:
            DeviceImplVulkan(engine::shared_ptr<platform::Window> window);

            void createFences(Device& device);

            DeviceImplVulkan(const DeviceImplVulkan&) = delete;
            DeviceImplVulkan(DeviceImplVulkan&&) = delete;
            DeviceImplVulkan& operator=(const DeviceImplVulkan&) = delete;
            DeviceImplVulkan& operator=(DeviceImplVulkan&&) = delete;

            void nullResources(engine::shared_ptr<NullResources> nullResources) override;
            NullResources& nullResources() override;

            void recycleUploadBuffers(uint32_t /*frameNumber*/) { LOG("Not implemented recycleUploadBuffers"); };

            const VkPhysicalDevice physicalDevice() const;
            const VkDevice& device() const;
            const VkSurfaceKHR& surface() const;
            const engine::vector<VulkanQueue>& deviceQueues() const;
            engine::shared_ptr<CommandAllocatorImplVulkan> allocator() const;

            engine::shared_ptr<TextureImplIf> createTexture(const Device& device, Queue& queue, const TextureDescription& desc) override;
            void uploadBuffer(CommandList& commandList, BufferSRV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferUAV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferCBV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferIBV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferVBV buffer, const tools::ByteRange& data, size_t startElement = 0) override;

            void uploadBuffer(CommandListImplVulkan& commandList, BufferSRV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplVulkan& commandList, BufferUAV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplVulkan& commandList, BufferCBV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplVulkan& commandList, BufferIBV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplVulkan& commandList, BufferVBV buffer, const tools::ByteRange& data, size_t startElement = 0);

            void uploadRawBuffer(CommandListImplIf* commandList, Buffer buffer, const tools::ByteRange& data, size_t startBytes) override;

            const platform::Window& window() const override;
            void window(engine::shared_ptr<platform::Window> window) override;
            int width() const override;
            int height() const override;

            QueueInfo createQueue(CommandListType type) const;
            void destroyQueue(QueueInfo queue) const;

            engine::shared_ptr<CommandAllocatorImplIf> createCommandAllocator(CommandListType type, const char* name) override;
            void freeCommandAllocator(engine::shared_ptr<CommandAllocatorImplIf> allocator) override;

            void waitForIdle() override;

            DescriptorHeapImplVulkan& descriptorHeap();

            void setCurrentFenceValue(CommandListType type, engine::FenceValue value) override;
            void processUploads(engine::FenceValue value, bool force = false) override;

            engine::unique_ptr<GpuMarkerContainer> getMarkerContainer() override;
            void returnMarkerContainer(engine::unique_ptr<GpuMarkerContainer>&& container) override;

            VulkanInstance& vulkanInstance() { return m_instance; };
            const VulkanInstance& vulkanInstance() const { return m_instance; };

            CpuTexture grabTexture(Device& device, TextureSRV texture) override;
            TextureSRVOwner loadTexture(Device& device, const CpuTexture& texture) override;
            void copyTexture(Device& device, const CpuTexture& texture, TextureSRV dst) override;

            TextureBufferCopyDesc getTextureBufferCopyDesc(size_t width, size_t height, Format format) override;
        private:
            std::mutex m_mutex;
            VulkanInstance m_instance;
            VkPhysicalDevice m_physicalDevice;
            engine::shared_ptr<VkDevice> m_device;
            engine::shared_ptr<VkSurfaceKHR> m_surface;
            engine::shared_ptr<platform::Window> m_window;
            const engine::vector<const char*> m_requiredExtensions;
            engine::shared_ptr<CommandAllocatorImplVulkan> m_allocator;
            mutable engine::vector<VulkanQueue> queues;
            BufferSRVOwner m_grabBuffer;

            friend class FenceImplVulkan;
            mutable FencePool m_fencePool;

            struct UploadAllocation
            {
                tools::RingBuffer::AllocStruct ptr;
                engine::FenceValue value;
            };
            engine::vector<UploadAllocation> m_uploadAllocations;
            engine::FenceValue m_currentFenceValue;
            engine::FenceValue m_currentCopyFenceValue;
            engine::shared_ptr<Fence> m_uploadFence;
            Queue* m_deviceQueue;

            tools::RingBuffer m_uploadAllocator;
            BufferOwner m_uploadBuffer;
            engine::shared_ptr<NullResources> m_nullResources;

            void uploadBuffer(CommandListImplVulkan& commandList, Buffer buffer, const tools::ByteRange& data, size_t startElement);

            bool isSuitableDevice(const VkPhysicalDevice& device) const;
            bool checkExtensionSupport(const VkPhysicalDevice& device) const;
            engine::vector<VulkanQueue> enumerateQueues(const VkPhysicalDevice& device) const;
            void createLogicalDevice();
            void createSurface();

            //do some kind of CommandAllocator list that DX12CommandList constructor can ask for
            std::queue<engine::shared_ptr<CommandAllocatorImplVulkan>> m_commandAllocatorsDirect;
            std::queue<engine::shared_ptr<CommandAllocatorImplVulkan>> m_commandAllocatorsCopy;
            std::queue<engine::vector<engine::shared_ptr<CommandAllocatorImplVulkan>>> m_inFlightCommandAllocatorsDirect; // these are not referenced by command buffers but can still be in flight
            std::queue<engine::vector<engine::shared_ptr<CommandAllocatorImplVulkan>>> m_inFlightCommandAllocatorsCopy; // these are not referenced by command buffers but can still be in flight

            engine::vector<engine::shared_ptr<CommandAllocatorImplVulkan>> m_returnedCommandAllocatorsDirect;
            engine::vector<engine::shared_ptr<CommandAllocatorImplVulkan>> m_returnedCommandAllocatorsCopy;
            engine::vector<engine::shared_ptr<CommandAllocatorImplVulkan>> m_inUseCommandAllocatorsDirect; // these are referenced by command buffers
            engine::vector<engine::shared_ptr<CommandAllocatorImplVulkan>> m_inUseCommandAllocatorsCopy; // these are referenced by command buffers

            engine::unique_ptr<DescriptorHeapImplVulkan> m_descriptorHeap;
        };
    }
}
