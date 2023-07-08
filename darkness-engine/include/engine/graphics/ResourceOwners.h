#pragma once

#include "engine/graphics/Resources.h"
#include "containers/memory.h"
#include <functional>

namespace engine
{
	namespace implementation
	{
		class BufferImplIf;
		class BufferSRVImplIf;
		class BufferUAVImplIf;
		class BufferIBVImplIf;
		class BufferCBVImplIf;
		class BufferVBVImplIf;
        class BindlessBufferSRVImplIf;
        class BindlessBufferUAVImplIf;

		class RaytracingAccelerationStructureImplIf;

		class TextureImplIf;
		class TextureSRVImplIf;
		class TextureUAVImplIf;
		class TextureDSVImplIf;
		class TextureRTVImplIf;
        class BindlessTextureSRVImplIf;
        class BindlessTextureUAVImplIf;

		class DeviceImplDX12;
		class DeviceImplVulkan;
	}

	template<typename T>
	class Returner
	{
	public:
		Returner() = default;
		Returner(
			std::function<void(engine::shared_ptr<T>)> returnImpl,
			engine::shared_ptr<T> returnValue)
			: m_return{ returnImpl }
			, m_returnValue{ returnValue }
		{}
		Returner(const Returner&) = delete;
		Returner(Returner&&) = default;
		Returner& operator=(const Returner&) = delete;
		Returner& operator=(Returner&&) = default;
		~Returner()
		{
			m_return(m_returnValue);
		}

	private:
		std::function<void(engine::shared_ptr<T>)> m_return;
		engine::shared_ptr<T> m_returnValue;
	};

	class Buffer;
	class BufferSRVOwner;
	class BufferUAVOwner;
	class BufferIBVOwner;
	class BufferCBVOwner;
	class BufferVBVOwner;
	class BufferOwner
	{
	public:
		BufferOwner() = default;
		BufferOwner(const BufferOwner&) = default;
		BufferOwner(BufferOwner&&) = default;
		BufferOwner& operator=(const BufferOwner&) = default;
		BufferOwner& operator=(BufferOwner&&) = default;

		operator Buffer();
		operator const Buffer() const;

		Buffer resource() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		BufferOwner(
			engine::shared_ptr<implementation::BufferImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::BufferImplIf>)> returnImplementation);
	private:
		friend class BufferSRVOwner;
		friend class BufferUAVOwner;
		friend class BufferIBVOwner;
		friend class BufferCBVOwner;
		friend class BufferVBVOwner;
		friend class DeviceImplDX12;
		friend class DeviceImplVulkan;
		BufferOwner(engine::shared_ptr<implementation::BufferImplIf> implementation,
					engine::shared_ptr<Returner<implementation::BufferImplIf>> returner);

		engine::shared_ptr<implementation::BufferImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::BufferImplIf>> m_returner;
	};

	class BufferSRV;
	class BufferSRVOwner
	{
	public:
		BufferSRVOwner();
		BufferSRVOwner(const BufferSRVOwner&) = default;
		BufferSRVOwner(BufferSRVOwner&&) = default;
		BufferSRVOwner& operator=(const BufferSRVOwner&) = default;
		BufferSRVOwner& operator=(BufferSRVOwner&&) = default;

		operator BufferSRV();
		operator const BufferSRV() const;

		BufferSRV resource() const;

		operator BufferOwner();
		operator BufferOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		BufferSRVOwner(
			engine::shared_ptr<implementation::BufferSRVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::BufferSRVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
			engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation);
	private:
		engine::shared_ptr<implementation::BufferSRVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::BufferSRVImplIf>> m_returner;
		engine::shared_ptr<implementation::BufferImplIf> m_bufferImplementation;
		engine::shared_ptr<Returner<implementation::BufferImplIf>> m_bufferReturner;
	};
	
	class BufferUAV;
	class BufferUAVOwner
	{
	public:
		BufferUAVOwner();
		BufferUAVOwner(const BufferUAVOwner&) = default;
		BufferUAVOwner(BufferUAVOwner&&) = default;
		BufferUAVOwner& operator=(const BufferUAVOwner&) = default;
		BufferUAVOwner& operator=(BufferUAVOwner&&) = default;

		operator BufferUAV();
		operator const BufferUAV() const;

		BufferUAV resource() const;

		operator BufferOwner();
		operator BufferOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		BufferUAVOwner(
			engine::shared_ptr<implementation::BufferUAVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::BufferUAVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
			engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation);
	private:
		engine::shared_ptr<implementation::BufferUAVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::BufferUAVImplIf>> m_returner;
		engine::shared_ptr<implementation::BufferImplIf> m_bufferImplementation;
		engine::shared_ptr<Returner<implementation::BufferImplIf>> m_bufferReturner;
	};

	class BufferIBV;
	class BufferIBVOwner
	{
	public:
		BufferIBVOwner();
		BufferIBVOwner(const BufferIBVOwner&) = default;
		BufferIBVOwner(BufferIBVOwner&&) = default;
		BufferIBVOwner& operator=(const BufferIBVOwner&) = default;
		BufferIBVOwner& operator=(BufferIBVOwner&&) = default;

		operator BufferIBV();
		operator const BufferIBV() const;

		BufferIBV resource() const;

		operator BufferOwner();
		operator BufferOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		BufferIBVOwner(
			engine::shared_ptr<implementation::BufferIBVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::BufferIBVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
			engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation);
	private:
		engine::shared_ptr<implementation::BufferIBVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::BufferIBVImplIf>> m_returner;
		engine::shared_ptr<implementation::BufferImplIf> m_bufferImplementation;
		engine::shared_ptr<Returner<implementation::BufferImplIf>> m_bufferReturner;
	};

	class BufferCBV;
	class BufferCBVOwner
	{
	public:
		BufferCBVOwner();
		BufferCBVOwner(const BufferCBVOwner&) = default;
		BufferCBVOwner(BufferCBVOwner&&) = default;
		BufferCBVOwner& operator=(const BufferCBVOwner&) = default;
		BufferCBVOwner& operator=(BufferCBVOwner&&) = default;

		operator BufferCBV();
		operator const BufferCBV() const;

		BufferCBV resource() const;

		operator BufferOwner();
		operator BufferOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		BufferCBVOwner(
			engine::shared_ptr<implementation::BufferCBVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::BufferCBVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
			engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation);
	private:
		engine::shared_ptr<implementation::BufferCBVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::BufferCBVImplIf>> m_returner;
		engine::shared_ptr<implementation::BufferImplIf> m_bufferImplementation;
		engine::shared_ptr<Returner<implementation::BufferImplIf>> m_bufferReturner;
	};

	class BufferVBV;
	class BufferVBVOwner
	{
	public:
		BufferVBVOwner();
		BufferVBVOwner(const BufferVBVOwner&) = default;
		BufferVBVOwner(BufferVBVOwner&&) = default;
		BufferVBVOwner& operator=(const BufferVBVOwner&) = default;
		BufferVBVOwner& operator=(BufferVBVOwner&&) = default;

		operator BufferVBV();
		operator const BufferVBV() const;

		BufferVBV resource() const;

		operator BufferOwner();
		operator BufferOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		BufferVBVOwner(
			engine::shared_ptr<implementation::BufferVBVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::BufferVBVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
			engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation);
	private:
		engine::shared_ptr<implementation::BufferVBVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::BufferVBVImplIf>> m_returner;
		engine::shared_ptr<implementation::BufferImplIf> m_bufferImplementation;
		engine::shared_ptr<Returner<implementation::BufferImplIf>> m_bufferReturner;
	};

    class BindlessBufferSRVOwner
    {
    public:
        BindlessBufferSRVOwner();
        BindlessBufferSRVOwner(const BindlessBufferSRVOwner&) = default;
        BindlessBufferSRVOwner(BindlessBufferSRVOwner&&) = default;
        BindlessBufferSRVOwner& operator=(const BindlessBufferSRVOwner&) = default;
        BindlessBufferSRVOwner& operator=(BindlessBufferSRVOwner&&) = default;

        operator BindlessBufferSRV();
        operator const BindlessBufferSRV() const;

        uint32_t push(BufferSRVOwner texture);
        size_t size() const;
        BufferSRV get(size_t index);
        uint64_t resourceId() const;
    protected:
        friend class Device;
        friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
        BindlessBufferSRVOwner(
            engine::shared_ptr<implementation::BindlessBufferSRVImplIf> impl,
            std::function<void(engine::shared_ptr<implementation::BindlessBufferSRVImplIf>)> returnImplementation);
    private:
        friend class DeviceImplDX12;
		friend class DeviceImplVulkan;
        BindlessBufferSRVOwner(
            engine::shared_ptr<implementation::BindlessBufferSRVImplIf> implementation,
            engine::shared_ptr<Returner<implementation::BindlessBufferSRVImplIf>> returner);

        engine::shared_ptr<implementation::BindlessBufferSRVImplIf> m_implementation;
        engine::shared_ptr<Returner<implementation::BindlessBufferSRVImplIf>> m_returner;
    };

    class BindlessBufferUAVOwner
    {
    public:
        BindlessBufferUAVOwner();
        BindlessBufferUAVOwner(const BindlessBufferUAVOwner&) = default;
        BindlessBufferUAVOwner(BindlessBufferUAVOwner&&) = default;
        BindlessBufferUAVOwner& operator=(const BindlessBufferUAVOwner&) = default;
        BindlessBufferUAVOwner& operator=(BindlessBufferUAVOwner&&) = default;

        operator BindlessBufferUAV();
        operator const BindlessBufferUAV() const;

        uint32_t push(BufferUAVOwner texture);
        size_t size() const;
        BufferUAV get(size_t index);
        uint64_t resourceId() const;
    protected:
        friend class Device;
        friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
        BindlessBufferUAVOwner(
            engine::shared_ptr<implementation::BindlessBufferUAVImplIf> impl,
            std::function<void(engine::shared_ptr<implementation::BindlessBufferUAVImplIf>)> returnImplementation);
    private:
        friend class DeviceImplDX12;
		friend class DeviceImplVulkan;
        BindlessBufferUAVOwner(
            engine::shared_ptr<implementation::BindlessBufferUAVImplIf> implementation,
            engine::shared_ptr<Returner<implementation::BindlessBufferUAVImplIf>> returner);

        engine::shared_ptr<implementation::BindlessBufferUAVImplIf> m_implementation;
        engine::shared_ptr<Returner<implementation::BindlessBufferUAVImplIf>> m_returner;
    };

	class RaytracingAccelerationStructureOwner
	{
	public:
		RaytracingAccelerationStructureOwner();
		RaytracingAccelerationStructureOwner(const RaytracingAccelerationStructureOwner&) = default;
		RaytracingAccelerationStructureOwner(RaytracingAccelerationStructureOwner&&) = default;
		RaytracingAccelerationStructureOwner& operator=(const RaytracingAccelerationStructureOwner&) = default;
		RaytracingAccelerationStructureOwner& operator=(RaytracingAccelerationStructureOwner&&) = default;

		operator RaytracingAccelerationStructure();
		operator const RaytracingAccelerationStructure() const;

		RaytracingAccelerationStructure resource() const;
		operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		RaytracingAccelerationStructureOwner(
			engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf>)> returnImplementation);
	private:
		friend class DeviceImplDX12;
		friend class DeviceImplVulkan;
		RaytracingAccelerationStructureOwner(engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf> implementation,
			engine::shared_ptr<Returner<implementation::RaytracingAccelerationStructureImplIf>> returner);

		engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::RaytracingAccelerationStructureImplIf>> m_returner;
	};

	class Texture;
	class TextureSRVOwner;
	class TextureUAVOwner;
	class TextureDSVOwner;
	class TextureRTVOwner;
	class TextureOwner
	{
	public:
		TextureOwner();
		TextureOwner(const TextureOwner&) = default;
		TextureOwner(TextureOwner&&) = default;
		TextureOwner& operator=(const TextureOwner&) = default;
		TextureOwner& operator=(TextureOwner&&) = default;

		operator Texture();
		operator const Texture() const;

		Texture resource() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		TextureOwner(
			engine::shared_ptr<implementation::TextureImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::TextureImplIf>)> returnImplementation);
	private:
		friend class TextureSRVOwner;
		friend class TextureUAVOwner;
		friend class TextureDSVOwner;
		friend class TextureRTVOwner;
		TextureOwner(
			engine::shared_ptr<implementation::TextureImplIf> implementation,
			engine::shared_ptr<Returner<implementation::TextureImplIf>> returner);

		engine::shared_ptr<implementation::TextureImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::TextureImplIf>> m_returner;
	};

	class TextureSRV;
	class TextureSRVOwner
	{
	public:
		TextureSRVOwner();
		TextureSRVOwner(const TextureSRVOwner&) = default;
		TextureSRVOwner(TextureSRVOwner&&) = default;
		TextureSRVOwner& operator=(const TextureSRVOwner&) = default;
		TextureSRVOwner& operator=(TextureSRVOwner&&) = default;

		operator TextureSRV();
		operator const TextureSRV() const;

		TextureSRV resource() const;

		operator TextureOwner();
		operator TextureOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		TextureSRVOwner(
			engine::shared_ptr<implementation::TextureSRVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::TextureSRVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::TextureImplIf> textureImpl,
			engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation);
	private:
		engine::shared_ptr<implementation::TextureSRVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::TextureSRVImplIf>> m_returner;
		engine::shared_ptr<implementation::TextureImplIf> m_textureImplementation;
		engine::shared_ptr<Returner<implementation::TextureImplIf>> m_textureReturner;
	};

	class TextureUAV;
	class TextureUAVOwner
	{
	public:
		TextureUAVOwner();
		TextureUAVOwner(const TextureUAVOwner&) = default;
		TextureUAVOwner(TextureUAVOwner&&) = default;
		TextureUAVOwner& operator=(const TextureUAVOwner&) = default;
		TextureUAVOwner& operator=(TextureUAVOwner&&) = default;

		operator TextureUAV();
		operator const TextureUAV() const;

		TextureUAV resource() const;

		operator TextureOwner();
		operator TextureOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		TextureUAVOwner(
			engine::shared_ptr<implementation::TextureUAVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::TextureUAVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::TextureImplIf> textureImpl,
			engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation);
	private:
		engine::shared_ptr<implementation::TextureUAVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::TextureUAVImplIf>> m_returner;
		engine::shared_ptr<implementation::TextureImplIf> m_textureImplementation;
		engine::shared_ptr<Returner<implementation::TextureImplIf>> m_textureReturner;
	};

	class TextureDSV;
	class TextureDSVOwner
	{
	public:
		TextureDSVOwner();
		TextureDSVOwner(const TextureDSVOwner&) = default;
		TextureDSVOwner(TextureDSVOwner&&) = default;
		TextureDSVOwner& operator=(const TextureDSVOwner&) = default;
		TextureDSVOwner& operator=(TextureDSVOwner&&) = default;

		operator TextureDSV();
		operator const TextureDSV() const;

		TextureDSV resource() const;

		operator TextureOwner();
		operator TextureOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		TextureDSVOwner(
			engine::shared_ptr<implementation::TextureDSVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::TextureDSVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::TextureImplIf> textureImpl,
			engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation);
	private:
		engine::shared_ptr<implementation::TextureDSVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::TextureDSVImplIf>> m_returner;
		engine::shared_ptr<implementation::TextureImplIf> m_textureImplementation;
		engine::shared_ptr<Returner<implementation::TextureImplIf>> m_textureReturner;
	};

	class TextureRTV;
	class TextureRTVOwner
	{
	public:
		TextureRTVOwner();
		TextureRTVOwner(const TextureRTVOwner&) = default;
		TextureRTVOwner(TextureRTVOwner&&) = default;
		TextureRTVOwner& operator=(const TextureRTVOwner&) = default;
		TextureRTVOwner& operator=(TextureRTVOwner&&) = default;

		operator TextureRTV();
		operator const TextureRTV() const;

		TextureRTV resource() const;

		operator TextureOwner();
		operator TextureOwner() const;
		explicit operator bool() const;
	protected:
		friend class Device;
		friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
		friend class implementation::SwapChainImplDX12;
		friend class implementation::SwapChainImplVulkan;
		TextureRTVOwner(
			engine::shared_ptr<implementation::TextureRTVImplIf> impl,
			std::function<void(engine::shared_ptr<implementation::TextureRTVImplIf>)> returnImplementation,
			engine::shared_ptr<implementation::TextureImplIf> textureImpl,
			engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation);
	private:
		engine::shared_ptr<implementation::TextureRTVImplIf> m_implementation;
		engine::shared_ptr<Returner<implementation::TextureRTVImplIf>> m_returner;
		engine::shared_ptr<implementation::TextureImplIf> m_textureImplementation;
		engine::shared_ptr<Returner<implementation::TextureImplIf>> m_textureReturner;
	};

    class BindlessTextureSRVOwner
    {
    public:
        BindlessTextureSRVOwner();
        BindlessTextureSRVOwner(const BindlessTextureSRVOwner&) = default;
        BindlessTextureSRVOwner(BindlessTextureSRVOwner&&) = default;
        BindlessTextureSRVOwner& operator=(const BindlessTextureSRVOwner&) = default;
        BindlessTextureSRVOwner& operator=(BindlessTextureSRVOwner&&) = default;

        operator BindlessTextureSRV();
        operator const BindlessTextureSRV() const;

        uint32_t push(TextureSRVOwner texture);
        size_t size() const;
        TextureSRV get(size_t index);
        uint64_t resourceId() const;
    protected:
        friend class Device;
        friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
        BindlessTextureSRVOwner(
            engine::shared_ptr<implementation::BindlessTextureSRVImplIf> impl,
            std::function<void(engine::shared_ptr<implementation::BindlessTextureSRVImplIf>)> returnImplementation);

    private:
        friend class DeviceImplDX12;
		friend class DeviceImplVulkan;
        BindlessTextureSRVOwner(
            engine::shared_ptr<implementation::BindlessTextureSRVImplIf> implementation,
            engine::shared_ptr<Returner<implementation::BindlessTextureSRVImplIf>> returner);

        engine::shared_ptr<implementation::BindlessTextureSRVImplIf> m_implementation;
        engine::shared_ptr<Returner<implementation::BindlessTextureSRVImplIf>> m_returner;
    };

    class BindlessTextureUAVOwner
    {
    public:
        BindlessTextureUAVOwner();
        BindlessTextureUAVOwner(const BindlessTextureUAVOwner&) = default;
        BindlessTextureUAVOwner(BindlessTextureUAVOwner&&) = default;
        BindlessTextureUAVOwner& operator=(const BindlessTextureUAVOwner&) = default;
        BindlessTextureUAVOwner& operator=(BindlessTextureUAVOwner&&) = default;

        operator BindlessTextureUAV();
        operator const BindlessTextureUAV() const;

        uint32_t push(TextureUAVOwner texture);
        size_t size() const;
        TextureUAV get(size_t index);
        uint64_t resourceId() const;
    protected:
        friend class Device;
        friend class implementation::DeviceImplDX12;
		friend class implementation::DeviceImplVulkan;
        BindlessTextureUAVOwner(
            engine::shared_ptr<implementation::BindlessTextureUAVImplIf> impl,
            std::function<void(engine::shared_ptr<implementation::BindlessTextureUAVImplIf>)> returnImplementation);
    private:
        friend class DeviceImplDX12;
		friend class DeviceImplVulkan;
        BindlessTextureUAVOwner(
            engine::shared_ptr<implementation::BindlessTextureUAVImplIf> implementation,
            engine::shared_ptr<Returner<implementation::BindlessTextureUAVImplIf>> returner);

        engine::shared_ptr<implementation::BindlessTextureUAVImplIf> m_implementation;
        engine::shared_ptr<Returner<implementation::BindlessTextureUAVImplIf>> m_returner;
    };
}
