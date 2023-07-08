#pragma once

#include <cstdint>
#include "engine/graphics/Format.h"
#include "engine/graphics/CommonNoDep.h"
#include "tools/ByteRange.h"
#include "shaders/ShaderPodTypes.h"

namespace engine
{
    struct SampleDescription
    {
        unsigned int count;
        unsigned int quality;
    };

    enum class TextureLayout
    {
        Unknown,
        RowMajor,
        UndefinedSwizzle64KB,
        StandardSwizzle64KB
    };

    enum class ResourceFlags
    {
        None,
        AllowRenderTarget,
        AllowDepthStencil,
        AllowUnorderedAccess,
        DenyShaderResource,
        AllowCrossAdapter,
        AllowSimultaneousAccess
    };

    enum class ResourceState;

    struct DepthStencilValue
    {
        float depth;
        unsigned char stencil;
    };

    struct ClearValue
    {
        Format format;
        union
        {
            float color[4];
            DepthStencilValue depthStencil;
        };
    };

    constexpr const std::size_t InvalidElementsValue = static_cast<std::size_t>(-1);
    constexpr const std::size_t InvalidElementSizeValue = static_cast<std::size_t>(-1);
    struct BufferDescription
    {

        struct Descriptor
        {
            Format format = Format::UNKNOWN;
            size_t elements = InvalidElementsValue;
            size_t elementSize = InvalidElementSizeValue;
            size_t firstElement = 0ull;
            ResourceUsage usage;
            bool structured = false;
            bool append = false;
            bool indirectArgument = false;
            bool indexBuffer = false;
            bool vertexBuffer = false;
            const char* name = nullptr;
        };
        Descriptor descriptor;

        BufferDescription& format(Format value)
        {
            descriptor.format = value;
            return *this;
        }
        BufferDescription& elements(size_t value)
        {
            descriptor.elements = value;
            return *this;
        }
        BufferDescription& elementSize(size_t value)
        {
            descriptor.elementSize = value;
            return *this;
        }
        BufferDescription& firstElement(size_t value)
        {
            descriptor.firstElement = value;
            return *this;
        }
        BufferDescription& usage(ResourceUsage value)
        {
            descriptor.usage = value;
            return *this;
        }
        BufferDescription& structured(bool value)
        {
            descriptor.structured = value;
            return *this;
        }
        BufferDescription& append(bool value)
        {
            descriptor.append = value;
            return *this;
        }
        BufferDescription& indirectArgument(bool value)
        {
            descriptor.indirectArgument = value;
            return *this;
        }
        BufferDescription& name(const char* value)
        {
            descriptor.name = value;
            return *this;
        }

        struct InitialData
        {
            engine::vector<uint8_t> data;
            size_t elementStart;
            size_t elements;
            size_t elementSize;
            explicit operator bool() const
            {
                return data.size() > 0;
            }

            InitialData()
                : elementStart{ 0 }
                , elements{ 0 }
                , elementSize{ 0 }
            {}

            template<typename T>
            InitialData(const engine::vector<T>& srcdata, size_t elemStart = 0)
            {
                elements = srcdata.size();
                elementSize = sizeof(T);
                auto size = elements * elementSize;
                data = engine::vector<uint8_t>(size, 0);
                std::memcpy(data.data(), reinterpret_cast<const uint8_t*>(srcdata.data()), size);
                elementStart = elemStart;
            }

            InitialData(const tools::ByteRange& srcdata, size_t alignment, size_t elemStart = 0)
            {
                elements = (srcdata.length() + (alignment - 1ull)) & ~(alignment - 1ull);
                data = engine::vector<uint8_t>(srcdata.sizeBytes(), 0);
                std::memcpy(data.data(), reinterpret_cast<const uint8_t*>(srcdata.start), srcdata.sizeBytes());
                elementStart = elemStart;
                elementSize = srcdata.elementSize;
            }
        };
        InitialData initialData;
        BufferDescription& setInitialData(InitialData data)
        {
            initialData = data;
            descriptor.elements = data.elements;
            descriptor.elementSize = data.elementSize;
            descriptor.firstElement = data.elementStart;
            return *this;
        };
    };

    struct TextureDescription
    {
        struct Descriptor
        {
            Format format = Format::UNKNOWN;
            size_t width = 0;
            size_t height = 0;
            size_t depth = 1;
            size_t arraySlices = 1;
            size_t mipLevels = 1;
            size_t samples = 1;
            ResourceDimension dimension = ResourceDimension::Texture2D;
            ResourceUsage usage = ResourceUsage::GpuRead;
            bool append = false;
            Float4 optimizedClearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
            float optimizedDepthClearValue = 1.0f;
            uint8_t optimizedStencilClearValue = 0;
            const char* name;
            bool shared = false;
        };
        Descriptor descriptor;

        TextureDescription& format(Format value)
        {
            descriptor.format = value;
            return *this;
        }
        TextureDescription& width(size_t value)
        {
            descriptor.width = value;
            return *this;
        }
        TextureDescription& height(size_t value)
        {
            descriptor.height = value;
            return *this;
        }
        TextureDescription& depth(size_t value)
        {
            descriptor.depth = value;
            return *this;
        }
        TextureDescription& arraySlices(size_t value)
        {
            descriptor.arraySlices = value;
            return *this;
        }
        TextureDescription& mipLevels(size_t value)
        {
            descriptor.mipLevels = value;
            return *this;
        }
        TextureDescription& samples(size_t value)
        {
            descriptor.samples = value;
            return *this;
        }
        TextureDescription& dimension(ResourceDimension value)
        {
            descriptor.dimension = value;
            if (value == ResourceDimension::TextureCube)
                descriptor.arraySlices = 6;
            return *this;
        }
        TextureDescription& usage(ResourceUsage value)
        {
            descriptor.usage = value;
            return *this;
        }
        TextureDescription& optimizedClearValue(Float4 value)
        {
            descriptor.optimizedClearValue = value;
            return *this;
        }
        TextureDescription& optimizedDepthClearValue(float value)
        {
            descriptor.optimizedDepthClearValue = value;
            return *this;
        }
        TextureDescription& optimizedStencilClearValue(uint8_t value)
        {
            descriptor.optimizedStencilClearValue = value;
            return *this;
        }
        TextureDescription& name(const char* value)
        {
            descriptor.name = value;
            return *this;
        }

        TextureDescription& shared(bool value)
        {
            descriptor.shared = value;
            return *this;
        }

        struct InitialData
        {
            tools::ByteRange data;
            size_t pitch;
            size_t slicePitch;
            explicit operator bool() const
            {
                return data.size() > 0;
            }

            InitialData()
                : pitch{ 0 }
                , slicePitch{ 0 }
            {}

            template<typename T>
            InitialData(const engine::vector<T>& srcdata, size_t _pitch, size_t _slicePitch)
                : data{ &srcdata[0], &srcdata[srcdata.size() - 1] + 1 }
                , pitch{ _pitch }
                , slicePitch{ _slicePitch }
            {}

            InitialData(const tools::ByteRange& srcdata, size_t _pitch, size_t _slicePitch)
                : data{ srcdata }
                , pitch{ _pitch }
                , slicePitch{ _slicePitch }
            {}
        };
        InitialData initialData;
        TextureDescription& setInitialData(const InitialData& data)
        {
            initialData = data;
            return *this;
        };
    };

    class Buffer;
    class BufferSRV;
    class BufferUAV;
    class BufferSRVOwner;
    class BufferUAVOwner;

    class Texture;
    class TextureSRV;
    class TextureUAV;
    class TextureSRVOwner;
    class TextureUAVOwner;

    namespace implementation
    {
        class DeviceImplIf;

        class BufferImplIf
        {
        public:
            virtual ~BufferImplIf() {};

            virtual void* map(const DeviceImplIf* device) = 0;
            virtual void unmap(const DeviceImplIf* device) = 0;

            virtual const BufferDescription::Descriptor& description() const = 0;
            virtual ResourceState state() const = 0;
            virtual void state(ResourceState _state) = 0;
        };

        class BufferSRVImplIf
        {
        public:
            virtual ~BufferSRVImplIf() {};

            virtual const BufferDescription::Descriptor& description() const = 0;
            virtual Buffer buffer() const = 0;
            virtual uint64_t uniqueId() const = 0;
        };

        class BufferUAVImplIf
        {
        public:
            virtual ~BufferUAVImplIf() {};

            virtual const BufferDescription::Descriptor& description() const = 0;
            virtual Buffer buffer() const = 0;
            virtual uint64_t uniqueId() const = 0;
            virtual size_t structureCounterOffsetBytes() const = 0;
        };

        class BufferIBVImplIf
        {
        public:
            virtual ~BufferIBVImplIf() {};

            virtual const BufferDescription::Descriptor& description() const = 0;
            virtual Buffer buffer() const = 0;
            virtual uint64_t uniqueId() const = 0;
        };

        class BufferCBVImplIf
        {
        public:
            virtual ~BufferCBVImplIf() {};

            virtual const BufferDescription::Descriptor& description() const = 0;
            virtual Buffer buffer() const = 0;
            virtual uint64_t uniqueId() const = 0;
        };

        class BufferVBVImplIf
        {
        public:
            virtual ~BufferVBVImplIf() {};

            virtual const BufferDescription::Descriptor& description() const = 0;
            virtual Buffer buffer() const = 0;
            virtual uint64_t uniqueId() const = 0;
        };

        class BindlessBufferSRVImplIf
        {
        public:
            virtual ~BindlessBufferSRVImplIf() {};

            virtual uint32_t push(BufferSRVOwner buffer) = 0;
            virtual size_t size() const = 0;
            virtual BufferSRV get(size_t index) = 0;
            virtual uint64_t resourceId() const = 0;

            virtual void updateDescriptors(DeviceImplIf* device) = 0;
            virtual bool change() const = 0;
            virtual void change(bool value) = 0;
        };

        class BindlessBufferUAVImplIf
        {
        public:
            virtual ~BindlessBufferUAVImplIf() {};

            virtual uint32_t push(BufferUAVOwner buffer) = 0;
            virtual size_t size() const = 0;
            virtual BufferUAV get(size_t index) = 0;
            virtual uint64_t resourceId() const = 0;

            virtual void updateDescriptors(DeviceImplIf* device) = 0;
            virtual bool change() const = 0;
            virtual void change(bool value) = 0;
        };

        class RaytracingAccelerationStructureImplIf
        {
        public:
            virtual ~RaytracingAccelerationStructureImplIf() {};

            virtual const BufferDescription::Descriptor& description() const = 0;
            virtual ResourceState state() const = 0;
            virtual void state(ResourceState _state) = 0;
            virtual uint64_t resourceId() const = 0;
        };

        class TextureImplIf
        {
        public:
            virtual ~TextureImplIf() {};

            virtual void* map(const DeviceImplIf* device) = 0;
            virtual void unmap(const DeviceImplIf* device) = 0;

            virtual const TextureDescription::Descriptor& description() const = 0;

            virtual ResourceState state(int slice, int mip) const = 0;
            virtual void state(int slice, int mip, ResourceState state) = 0;
        };

        class TextureSRVImplIf
        {
        public:
            virtual ~TextureSRVImplIf() {};

            virtual const TextureDescription::Descriptor& description() const = 0;

            virtual Texture texture() const = 0;
            virtual Format format() const = 0;
            virtual size_t width() const = 0;
            virtual size_t height() const = 0;
            virtual size_t depth() const = 0;
            virtual ResourceDimension dimension() const = 0;
            virtual uint64_t uniqueId() const = 0;
            virtual const SubResource& subResource() const = 0;
        };

        class TextureUAVImplIf
        {
        public:
            virtual ~TextureUAVImplIf() {};

            virtual const TextureDescription::Descriptor& description() const = 0;

            virtual Texture texture() const = 0;
            virtual Format format() const = 0;
            virtual size_t width() const = 0;
            virtual size_t height() const = 0;
            virtual size_t depth() const = 0;
            virtual ResourceDimension dimension() const = 0;
            virtual uint64_t uniqueId() const = 0;
            virtual const SubResource& subResource() const = 0;
        };

        class TextureDSVImplIf
        {
        public:
            virtual ~TextureDSVImplIf() {};

            virtual const TextureDescription::Descriptor& description() const = 0;

            virtual Texture texture() const = 0;
            virtual Format format() const = 0;
            virtual size_t width() const = 0;
            virtual size_t height() const = 0;
            virtual uint64_t uniqueId() const = 0;
            virtual const SubResource& subResource() const = 0;
        };

        class TextureRTVImplIf
        {
        public:
            virtual ~TextureRTVImplIf() {};

            virtual const TextureDescription::Descriptor& description() const = 0;

            virtual Texture texture() const = 0;
            virtual Format format() const = 0;
            virtual size_t width() const = 0;
            virtual size_t height() const = 0;
            virtual uint64_t uniqueId() const = 0;
            virtual const SubResource& subResource() const = 0;
        };

        class BindlessTextureSRVImplIf
        {
        public:
            virtual ~BindlessTextureSRVImplIf() {};

            virtual uint32_t push(TextureSRVOwner texture) = 0;
            virtual size_t size() const = 0;
            virtual TextureSRV get(size_t index) = 0;
            virtual uint64_t resourceId() const = 0;
            virtual void updateDescriptors(DeviceImplIf* device) = 0;
            virtual bool change() const = 0;
            virtual void change(bool value) = 0;
        };

        class BindlessTextureUAVImplIf
        {
        public:
            virtual ~BindlessTextureUAVImplIf() {};

            virtual uint32_t push(TextureUAVOwner texture) = 0;
            virtual size_t size() const = 0;
            virtual TextureUAV get(size_t index) = 0;
            virtual uint64_t resourceId() const = 0;
            virtual void updateDescriptors(DeviceImplIf* device) = 0;
            virtual bool change() const = 0;
            virtual void change(bool value) = 0;
        };
    }
}
