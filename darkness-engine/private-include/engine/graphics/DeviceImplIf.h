#pragma once

#include "containers/memory.h"
#include "containers/vector.h"
#include "engine/graphics/Format.h"
#include "tools/Debug.h"

namespace tools
{
    class ByteRange;
}

namespace platform
{
    class Window;
}

namespace engine
{
    class Device;
    class Queue;
    class CommandList;
    class BufferSRV;
    class BufferUAV;
    class BufferCBV;
    class BufferIBV;
    class BufferVBV;
    class Buffer;
    class TextureSRV;
    class TextureUAV;
    class TextureSRVOwner;
    struct NullResources;
    struct TextureDescription;
    enum class CommandListType;
    class GpuMarkerContainer;
    using FenceValue = unsigned long long;

    struct TextureBufferCopyDesc
    {
        size_t elementSize;
        size_t elements;
        size_t bufferSize;

        size_t pitch;
        size_t pitchBytes;
        size_t width;
        size_t height;
        Format format;
        bool zeroUp;
    };

    struct CpuTexture
    {
        shared_ptr<vector<uint8_t>> data;
        size_t pitch;
        size_t pitchBytes;
        size_t width;
        size_t height;
        Format format;
        bool zeroUp;
        CpuTexture flipYAxis() const
        {
            CpuTexture result;
            result.pitch = pitch;
            result.pitchBytes = pitchBytes;
            result.width = width;
            result.height = height;
            result.format = format;
            result.zeroUp = zeroUp;
            result.data = engine::make_shared<vector<uint8_t>>();
            result.data->resize(data->size());

            auto src = data->data();
            ASSERT(height > 0, "Zero height in flip");
            auto dst = result.data->data() + (height-1) * pitchBytes;
            for (int i = 0; i < height; ++i)
            {
                memcpy(dst, src, pitchBytes);
                src += pitchBytes;
                dst -= pitchBytes;
            }
            return result;
        }

        CpuTexture tightPack() const
        {
            CpuTexture result;
            result.pitch = width;
            result.pitchBytes = result.pitch * static_cast<int>(formatBytes(format));
            result.width = width;
            result.height = height;
            result.format = format;
            result.zeroUp = zeroUp;
            result.data = engine::make_shared<vector<uint8_t>>();
            result.data->resize(formatBytes(format, width, height));

            bool flipY = false;

            auto src = data->data();
            auto dst = flipY ? result.data->data() + ((static_cast<int64_t>(height) - 1) * static_cast<int64_t>(result.pitchBytes)) : result.data->data();

            if (data->size() != result.data->size())
                return result;

            for (int i = 0; i < height; ++i)
            {
                memcpy(dst, src, result.pitchBytes);
                src += pitchBytes;
                if(flipY)
                    dst -= result.pitchBytes;
                else
                    dst += result.pitchBytes;
            }
            return result;
        }
    };

    namespace implementation
    {
        class CommandAllocatorImplIf;
        class CommandListImplIf;
        class TextureImplIf;
        class DescriptorHeaps;

        class DeviceImplIf
        {
        public:
            virtual ~DeviceImplIf() {};

            virtual void createFences(Device& device) = 0;

            virtual void nullResources(engine::shared_ptr<NullResources> nullResources) = 0;
            virtual NullResources& nullResources() = 0;

            virtual engine::shared_ptr<TextureImplIf> createTexture(const Device& device, Queue& queue, const TextureDescription& desc) = 0;
            virtual void uploadBuffer(CommandList& commandList, BufferSRV buffer, const tools::ByteRange& data, size_t startElement) = 0;
            virtual void uploadBuffer(CommandList& commandList, BufferUAV buffer, const tools::ByteRange& data, size_t startElement) = 0;
            virtual void uploadBuffer(CommandList& commandList, BufferCBV buffer, const tools::ByteRange& data, size_t startElement) = 0;
            virtual void uploadBuffer(CommandList& commandList, BufferIBV buffer, const tools::ByteRange& data, size_t startElement) = 0;
            virtual void uploadBuffer(CommandList& commandList, BufferVBV buffer, const tools::ByteRange& data, size_t startElement) = 0;

            virtual void uploadRawBuffer(CommandListImplIf* commandList, Buffer buffer, const tools::ByteRange& data, size_t startBytes) = 0;

            virtual const platform::Window& window() const = 0;
            virtual void window(engine::shared_ptr<platform::Window> window) = 0;
            virtual int width() const = 0;
            virtual int height() const = 0;

            virtual void waitForIdle() = 0;

            virtual engine::shared_ptr<CommandAllocatorImplIf> createCommandAllocator(CommandListType type, const char* name) = 0;
            virtual void freeCommandAllocator(engine::shared_ptr<CommandAllocatorImplIf> allocator) = 0;

            virtual engine::unique_ptr<GpuMarkerContainer> getMarkerContainer() = 0;
            virtual void returnMarkerContainer(engine::unique_ptr<GpuMarkerContainer>&& container) = 0;

            virtual void setCurrentFenceValue(CommandListType type, engine::FenceValue value) = 0;
            virtual void processUploads(engine::FenceValue value, bool force) = 0;

            virtual CpuTexture grabTexture(Device& device, TextureSRV texture) = 0;
            virtual TextureSRVOwner loadTexture(Device& device, const CpuTexture& texture) = 0;
            virtual void copyTexture(Device& device, const CpuTexture& texture, TextureSRV dst) = 0;

            virtual TextureBufferCopyDesc getTextureBufferCopyDesc(size_t width, size_t height, Format format) = 0;
        };
    }
}
