#pragma once

#include "engine/graphics/DeviceImplIf.h"
#include "engine/rendering/BufferSettings.h"
#include "containers/memory.h"
#include "tools/ComPtr.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/dx12/DX12Common.h"
#include "engine/graphics/dx12/DX12CommonNoDep.h"
#include "engine/graphics/dx12/DX12FenceStorage.h"
#include "engine/graphics/GpuMarkerStorage.h"
#include "containers/unordered_map.h"
#include "tools/MemoryAllocator.h"
#include "tools/RingBuffer.h"
#include "engine/graphics/Device.h"
#include <queue>

#ifndef _DURANGO
struct ID3D12DebugDevice;
extern ID3D12DebugDevice* debugInterface;
#endif

struct ID3D12Device;

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#undef UPLOADBUFFER_MEMORYALLOCATOR_ALIGNEDCHUNKS
#define UPLOADBUFFER_MEMORYALLOCATOR_RINGBUFFER

namespace platform
{
    class Window;
}

namespace engine
{
    namespace implementation
    {
        class CommandListImplDX12;
        class CommandAllocatorImpl;
		class TextureImpl;
		class BufferImpl;
    }

	class Device;
    class Buffer;
    class BufferSRV;
    class BufferIBV;
    class BufferVBV;
    class BufferCBV;
    class Texture;
    class Queue;
    class Device;
    class CommandList;
    struct TextureDescription;
    enum class CommandListType;

    enum class Format;

    namespace implementation
    {
        class CommandAllocatorImplIf;
        class PipelineImpl;

        class DeviceImplDX12 : public DeviceImplIf
        {
        public:
            DeviceImplDX12(engine::shared_ptr<platform::Window> window, const engine::string& preferredAdapter = "");
            virtual ~DeviceImplDX12();

			void createFences(Device& device) override;

            void nullResources(engine::shared_ptr<NullResources> nullResources) override;
            NullResources& nullResources() override;

            engine::shared_ptr<TextureImplIf> createTexture(const Device& device, Queue& queue, const TextureDescription& desc) override;
            void uploadBuffer(CommandList& commandList, BufferSRV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferUAV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferCBV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferIBV buffer, const tools::ByteRange& data, size_t startElement = 0) override;
            void uploadBuffer(CommandList& commandList, BufferVBV buffer, const tools::ByteRange& data, size_t startElement = 0) override;

            void uploadBuffer(CommandListImplDX12& commandList, BufferSRV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplDX12& commandList, BufferUAV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplDX12& commandList, BufferCBV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplDX12& commandList, BufferIBV buffer, const tools::ByteRange& data, size_t startElement = 0);
            void uploadBuffer(CommandListImplDX12& commandList, BufferVBV buffer, const tools::ByteRange& data, size_t startElement = 0);

            void uploadRawBuffer(CommandListImplIf* commandList, Buffer buffer, const tools::ByteRange& data, size_t startBytes) override;

            const platform::Window& window() const override;
			void window(engine::shared_ptr<platform::Window> window) override;
            int width() const override;
            int height() const override;

            void waitForIdle() override;
            DescriptorHeapsDX12& heaps();
            const DescriptorHeapsDX12& heaps() const;

            engine::shared_ptr<CommandAllocatorImplIf> createCommandAllocator(CommandListType type, const char* name) override;
            void freeCommandAllocator(engine::shared_ptr<CommandAllocatorImplIf> allocator) override;

            const FenceStorageDX12& fenceStorage() const
            {
                return m_fenceStorage;
            }

            engine::unique_ptr<GpuMarkerContainer> getMarkerContainer() override;
            void returnMarkerContainer(engine::unique_ptr<GpuMarkerContainer>&& container) override;

			void setCurrentFenceValue(CommandListType type, engine::FenceValue value) override;
			void processUploads(engine::FenceValue value, bool force = false) override;

            CpuTexture grabTexture(Device& device, TextureSRV texture) override;
            TextureSRVOwner loadTexture(Device& device, const CpuTexture& texture) override;
            void copyTexture(Device& device, const CpuTexture& texture, TextureSRV dst) override;

            TextureBufferCopyDesc getTextureBufferCopyDesc(size_t width, size_t height, Format format) override;
        public:
            ID3D12Device* device() const;
#ifdef DXR_BUILD
            ID3D12Device5* dxrDevice() const;
#endif
            ID3D12CommandSignature* drawIndirectSignature();
            ID3D12CommandSignature* drawIndexedIndirectSignature();
            ID3D12CommandSignature* drawIndexedInstancedIndirectSignature();
            ID3D12CommandSignature* dispatchIndirectSignature();
            ID3D12CommandSignature* executeIndirectClusterSignature(ID3D12RootSignature* rootSignature);


        private:
			friend class SwapChainImplDX12;
            friend class BindlessTextureSRVImplDX12;
            friend class BindlessTextureUAVImplDX12;
            friend class BindlessBufferSRVImplDX12;
            friend class BindlessBufferUAVImplDX12;
            friend class CommandListImplDX12;
            std::mutex m_mutex;
            tools::ComPtr<ID3D12Device> m_device;
#ifdef DXR_BUILD
			tools::ComPtr<ID3D12Device5> m_dxrDevice;
#endif
			engine::shared_ptr<Fence> m_graphicsQueueUploadFence;
            engine::shared_ptr<Fence> m_copyQueueUploadFence;
			Queue* m_deviceGraphicsQueue;
            Queue* m_deviceCopyQueue;
            BufferSRVOwner m_grabBuffer;

            engine::shared_ptr<platform::Window> m_window;
            DescriptorHeapsDX12 m_descriptorHeaps;
            BufferOwner m_uploadBuffer;
            engine::shared_ptr<NullResources> m_nullResources;
            FenceStorageDX12 m_fenceStorage;
            int m_currentHeap;
            GpuMarkerStorage m_gpuMarkerStorage;

			struct UploadAllocation
			{
				tools::RingBuffer::AllocStruct ptr;
				engine::FenceValue value;
			};
			engine::vector<UploadAllocation> m_uploadAllocations;
			engine::FenceValue m_currentFenceValue;
            engine::FenceValue m_currentCopyFenceValue;

#ifdef UPLOADBUFFER_MEMORYALLOCATOR_RINGBUFFER
            tools::RingBuffer m_allocator;
#endif

#ifdef UPLOADBUFFER_MEMORYALLOCATOR_ALIGNEDCHUNKS
            tools::MemoryAllocator m_allocator;
            engine::vector<engine::shared_ptr<void>> m_currentUploadBufferList;
            std::queue<engine::vector<engine::shared_ptr<void>>> m_uploadBuffers;
#endif

            tools::ComPtr<ID3D12CommandSignature> m_drawSignature;
            tools::ComPtr<ID3D12CommandSignature> m_drawIndexedSignature;
            tools::ComPtr<ID3D12CommandSignature> m_drawIndexedInstancedSignature;
            tools::ComPtr<ID3D12CommandSignature> m_dispatchSignature;
            engine::unordered_map<ID3D12RootSignature*, ID3D12CommandSignature*> m_executeIndirectClusterSignature;

			void uploadBufferInternal(CommandListImplDX12& commandList, Buffer buffer, const tools::ByteRange& data, size_t startElement);
        };
    }
}
