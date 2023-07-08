#include "engine/graphics/CommandList.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Common.h"
#include "shaders/ShaderTypes.h"
#include "engine/primitives/Color.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/dx12/DX12Device.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanCommandList.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalCommandList.h"
#endif

static int CommandListCount = 0;

using namespace tools;
using namespace engine::implementation;

namespace engine
{
    void CommandList::makeSureHasCorePipelines(engine::shared_ptr<CorePipelines> pipelines)
    {
        if (!m_impl->abs().m_corePipelines)
            m_impl->abs().m_corePipelines = pipelines;
    }

    CommandList::CommandList(
        const Device& device, 
        const char* name, 
        CommandListType type, 
        GraphicsApi api)
        : m_impl{}
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::shared_ptr<CommandListImplIf>(new CommandListImplDX12(
                const_cast<Device*>(&device), type, name),
                [&device, this, type](CommandListImplIf* impl)
                { const_cast<Device*>(&device)->returnCommandList(impl, type, impl->abs().m_name); });
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::shared_ptr<CommandListImplIf>(new CommandListImplVulkan(
                const_cast<Device*>(&device), type, name),
                [&device, this, type](CommandListImplIf* impl)
                { const_cast<Device*>(&device)->returnCommandList(impl, type, impl->abs().m_name); });

        m_impl->abs().m_device = &device;
        m_impl->abs().m_name = name;
        m_impl->abs().m_corePipelines = const_cast<Device*>(&device)->corePipelines();
        m_impl->abs().m_type = type;
        m_impl->abs().m_api = api;

        if(!m_impl->abs().commandListSemaphore)
            m_impl->abs().commandListSemaphore = engine::shared_ptr<Semaphore>(new Semaphore(*const_cast<Device*>(&device), api));
    }

    CommandList::CommandList(
        implementation::CommandListImplIf* impl, 
        const Device& device, 
        const char* name, 
        CommandListType type,
        GraphicsApi api)
        : m_impl{ impl,
                [&device, this, type](CommandListImplIf* impl)
                {
                    const_cast<Device*>(&device)->returnCommandList(impl, type, impl->abs().m_name);
                } }
    {
        m_impl->abs().m_device = &device;
        m_impl->abs().m_name = name;
        m_impl->abs().m_corePipelines = const_cast<Device*>(&device)->corePipelines();
        m_impl->abs().m_type = type;
        m_impl->abs().m_api = api;

        if (!m_impl->abs().commandListSemaphore)
            m_impl->abs().commandListSemaphore = engine::shared_ptr<Semaphore>(new Semaphore(*const_cast<Device*>(&device), api));
    }

    CommandListType CommandList::type() const
    {
        return m_impl->abs().m_type;
    }

    Semaphore& CommandList::commandListSemaphore()
    {
        return *m_impl->abs().commandListSemaphore;
    }

    const char* CommandList::name() const
    {
        return m_impl->abs().m_name;
    }

    void CommandList::name(const char* name)
    {
        m_impl->abs().m_name = name;
    }

    engine::vector<Format>& CommandList::lastSetRTVFormats()
    {
        return m_impl->abs().m_lastSetRTVFormats;
    }

    Format CommandList::lastSetDSVFormat()
    {
        return m_impl->abs().m_lastSetDSVFormat;
    }

	unsigned int CommandList::lastSetSamples()
	{
		return m_impl->abs().msaaCount;
	}

    CommandListType CommandList::commandListType() const
    {
        return m_impl->abs().m_type;
    }

    engine::vector<BufferUAVOwner>& CommandList::debugBuffers()
    {
        return m_impl->abs().m_debugBuffers;
    }

    unsigned long long CommandList::submitFenceValue() const
    {
        return m_impl->abs().m_submitFenceValue;
    }

    void CommandList::submitFenceValue(unsigned long long value)
    {
        m_impl->abs().m_submitFenceValue = value;
    }

    void CommandList::clear()
    {
        m_impl->clear();
    }

    void CommandList::reset(engine::shared_ptr<implementation::PipelineImplIf> pipelineState)
    {
        m_impl->reset(pipelineState.get());
    }

    void CommandList::transition(Texture resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(Texture resource, ResourceState state, const SubResource& subResource)
    {
        m_impl->transition(resource, state, subResource);
    }

    void CommandList::transition(TextureRTV resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(TextureSRV resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(TextureDSV resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(Buffer resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(BufferSRV resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(BufferIBV resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(BufferVBV resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::transition(BufferCBV resource, ResourceState state)
    {
        m_impl->transition(resource, state);
    }

    void CommandList::setPredicate(BufferSRV buffer, uint64_t offset, PredicationOp op)
    {
        if (buffer.valid())
            transition(buffer, ResourceState::Predication);

        m_impl->setPredicate(buffer, offset, op);
    }

    Barrier CommandList::createBarrier(
        ResourceBarrierFlags flags,
        TextureRTV resource,
        ResourceState before,
        ResourceState after,
        unsigned int subResource,
        const Semaphore& waitSemaphore,
        const Semaphore& signalSemaphore
    )
    {
        return Barrier(
            *this, 
            flags, 
            resource, 
            before, 
            after, 
            subResource,
            waitSemaphore, 
            signalSemaphore,
            m_impl->abs().m_api
        );
    }

    void CommandList::setRenderTargets(engine::vector<TextureRTV> targets)
    {
        m_impl->abs().m_lastSetRTVFormats.clear();
        for (auto&& target : targets)
        {
            m_impl->abs().m_lastSetRTVFormats.emplace_back(target.format());
            if(target.valid())
                transition(target, ResourceState::RenderTarget);
        }
		if (targets.size() > 0)
			m_impl->abs().msaaCount = static_cast<unsigned int>(targets[0].texture().description().samples);
		else
			m_impl->abs().msaaCount = 1;
        m_impl->setRenderTargets(targets);
    }

    void CommandList::setRenderTargets(
        engine::vector<TextureRTV> targets,
        TextureDSV dsv)
    {
        m_impl->abs().m_lastSetRTVFormats.clear();
        for (auto&& target : targets)
        {
            m_impl->abs().m_lastSetRTVFormats.emplace_back(target.format());
            if (target.valid())
                transition(target, ResourceState::RenderTarget);
        }
        transition(dsv, ResourceState::DepthWrite);
        m_impl->abs().m_lastSetDSVFormat = dsv.format();

		if (targets.size() > 0)
			m_impl->abs().msaaCount = static_cast<unsigned int>(targets[0].texture().description().samples);
		else
			m_impl->abs().msaaCount = 1;

        m_impl->setRenderTargets(targets, dsv);
    }

    void CommandList::clearRenderTargetView(TextureRTV target)
    {
        auto clearValue = target.texture().description().optimizedClearValue;
        m_impl->clearTextureRTV(target, { clearValue.x, clearValue.y, clearValue.z, clearValue.w });
    }

    void CommandList::clearRenderTargetView(TextureRTV target, const Color4f& color)
    {
        m_impl->clearTextureRTV(target, color);
    }

    void CommandList::clearDepthStencilView(TextureDSV target, float depth, uint8_t stencil)
    {
        m_impl->clearTextureDSV(target, depth, stencil);
    }

    void CommandList::clearTexture(TextureUAV texture, float value)
    {
        auto& m_corePipelines = m_impl->abs().m_corePipelines;

        auto width = texture.width();
        auto height = texture.height();
        auto depth = texture.depth();

        switch (formatComponents(texture.format()))
        {
            case 1:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df.cs.tex = texture;
                        m_corePipelines->clearTexture1df.cs.size = { static_cast<uint32_t>(width), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1df.cs.value = value;
                        bindPipe(m_corePipelines->clearTexture1df);
                        dispatch(roundUpToMultiple(width, 64ull) / 64ull, 1ull, 1ull);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df.cs.tex = texture;
                        m_corePipelines->clearTexture2df.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 1u, 1u };
                        m_corePipelines->clearTexture2df.cs.value = value;
                        bindPipe(m_corePipelines->clearTexture2df);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df.cs.tex = texture;
                        m_corePipelines->clearTexture3df.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 
                            static_cast<uint32_t>(depth), 1u };
                        m_corePipelines->clearTexture3df.cs.value = value;
                        bindPipe(m_corePipelines->clearTexture3df);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 2:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df2.cs.tex = texture;
                        m_corePipelines->clearTexture1df2.cs.size = { static_cast<uint32_t>(width), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1df2.cs.value = { value, value };
                        bindPipe(m_corePipelines->clearTexture1df2);
                        dispatch(roundUpToMultiple(width, 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df2.cs.tex = texture;
                        m_corePipelines->clearTexture2df2.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 1u, 1u };
                        m_corePipelines->clearTexture2df2.cs.value = { value, value };
                        bindPipe(m_corePipelines->clearTexture2df2);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df2.cs.tex = texture;
                        m_corePipelines->clearTexture3df2.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 
                            static_cast<uint32_t>(depth), 1u };
                        m_corePipelines->clearTexture3df2.cs.value = { value, value };
                        bindPipe(m_corePipelines->clearTexture3df2);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 3:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df3.cs.tex = texture;
                        m_corePipelines->clearTexture1df3.cs.size = { static_cast<uint32_t>(width), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1df3.cs.value = { value, value, value };
                        bindPipe(m_corePipelines->clearTexture1df3);
                        dispatch(roundUpToMultiple(width, 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df3.cs.tex = texture;
                        m_corePipelines->clearTexture2df3.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 1u, 1u };
                        m_corePipelines->clearTexture2df3.cs.value = { value, value, value };
                        bindPipe(m_corePipelines->clearTexture2df3);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df3.cs.tex = texture;
                        m_corePipelines->clearTexture3df3.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 
                            static_cast<uint32_t>(depth), 1u };
                        m_corePipelines->clearTexture3df3.cs.value = { value, value, value };
                        bindPipe(m_corePipelines->clearTexture3df3);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 4:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df4.cs.tex = texture;
                        m_corePipelines->clearTexture1df4.cs.size = { static_cast<uint32_t>(width), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1df4.cs.value = { value, value, value, value };
                        bindPipe(m_corePipelines->clearTexture1df4);
                        dispatch(roundUpToMultiple(width, 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df4.cs.tex = texture;
                        m_corePipelines->clearTexture2df4.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 1u, 1u };
                        m_corePipelines->clearTexture2df4.cs.value = { value, value, value, value };
                        bindPipe(m_corePipelines->clearTexture2df4);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df4.cs.tex = texture;
                        m_corePipelines->clearTexture3df4.cs.size = { 
                            static_cast<uint32_t>(width), 
                            static_cast<uint32_t>(height), 
                            static_cast<uint32_t>(depth), 1u };
                        m_corePipelines->clearTexture3df4.cs.value = { value, value, value, value };
                        bindPipe(m_corePipelines->clearTexture3df4);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
        }
    }

    void CommandList::clearTexture(TextureUAV texture, Vector4f value)
    {
        auto& m_corePipelines = m_impl->abs().m_corePipelines;

        switch (formatComponents(texture.format()))
        {
            case 1:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df.cs.tex = texture;
                        m_corePipelines->clearTexture1df.cs.size = { 
                            static_cast<uint32_t>(texture.width()), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1df.cs.value = value.x;
                        bindPipe(m_corePipelines->clearTexture1df);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df.cs.tex = texture;
                        m_corePipelines->clearTexture2df.cs.size = { 
                            static_cast<uint32_t>(texture.width()), 
                            static_cast<uint32_t>(texture.height()), 1u, 1u };
                        m_corePipelines->clearTexture2df.cs.value = value.x;
                        bindPipe(m_corePipelines->clearTexture2df);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df.cs.tex = texture;
                        m_corePipelines->clearTexture3df.cs.size = { 
                            static_cast<uint32_t>(texture.width()), 
                            static_cast<uint32_t>(texture.height()), 
                            static_cast<uint32_t>(texture.depth()), 1u };
                        m_corePipelines->clearTexture3df.cs.value = value.x;
                        bindPipe(m_corePipelines->clearTexture3df);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 2:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df2.cs.tex = texture;
                        m_corePipelines->clearTexture1df2.cs.size = Vector4<size_t>{ texture.width(), 1ull, 1ull, 1ull };
                        m_corePipelines->clearTexture1df2.cs.value = value.xy();
                        bindPipe(m_corePipelines->clearTexture1df2);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df2.cs.tex = texture;
                        m_corePipelines->clearTexture2df2.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2df2.cs.value = value.xy();
                        bindPipe(m_corePipelines->clearTexture2df2);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df2.cs.tex = texture;
                        m_corePipelines->clearTexture3df2.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3df2.cs.value = value.xy();
                        bindPipe(m_corePipelines->clearTexture3df2);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 3:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df3.cs.tex = texture;
                        m_corePipelines->clearTexture1df3.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1df3.cs.value = value.xyz();
                        bindPipe(m_corePipelines->clearTexture1df3);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df3.cs.tex = texture;
                        m_corePipelines->clearTexture2df3.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2df3.cs.value = value.xyz();
                        bindPipe(m_corePipelines->clearTexture2df3);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df3.cs.tex = texture;
                        m_corePipelines->clearTexture3df3.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3df3.cs.value = value.xyz();
                        bindPipe(m_corePipelines->clearTexture3df3);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 4:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1df4.cs.tex = texture;
                        m_corePipelines->clearTexture1df4.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1df4.cs.value = value;
                        bindPipe(m_corePipelines->clearTexture1df4);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2df4.cs.tex = texture;
                        m_corePipelines->clearTexture2df4.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2df4.cs.value = value;
                        bindPipe(m_corePipelines->clearTexture2df4);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3df4.cs.tex = texture;
                        m_corePipelines->clearTexture3df4.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3df4.cs.value = value;
                        bindPipe(m_corePipelines->clearTexture3df4);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
					case ResourceDimension::TextureCube:
					{
						m_corePipelines->clearTexture2df4.cs.tex = texture;
						m_corePipelines->clearTexture2df4.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
						m_corePipelines->clearTexture2df4.cs.value = value;
						bindPipe(m_corePipelines->clearTexture2df4);
						dispatch(
							roundUpToMultiple(texture.width(), 8u) / 8u,
							roundUpToMultiple(texture.height(), 8u) / 8u,
							1);
						break;
					}
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
        }
    }

    void CommandList::clearTexture(TextureUAV texture, uint32_t value)
    {
        auto& m_corePipelines = m_impl->abs().m_corePipelines;

        switch (formatComponents(texture.format()))
        {
            case 1:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du.cs.tex = texture;
                        m_corePipelines->clearTexture1du.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du.cs.value.x = value;
                        bindPipe(m_corePipelines->clearTexture1du);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du.cs.tex = texture;
                        m_corePipelines->clearTexture2du.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du.cs.value.x = value;
                        bindPipe(m_corePipelines->clearTexture2du);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du.cs.tex = texture;
                        m_corePipelines->clearTexture3du.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du.cs.value.x = value;
                        bindPipe(m_corePipelines->clearTexture3du);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 2:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du2.cs.tex = texture;
                        m_corePipelines->clearTexture1du2.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du2.cs.value.x = value;
                        m_corePipelines->clearTexture1du2.cs.value.y = value;
                        bindPipe(m_corePipelines->clearTexture1du2);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du2.cs.tex = texture;
                        m_corePipelines->clearTexture2du2.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du2.cs.value.x = value;
                        m_corePipelines->clearTexture2du2.cs.value.y = value;
                        bindPipe(m_corePipelines->clearTexture2du2);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du2.cs.tex = texture;
                        m_corePipelines->clearTexture3du2.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du2.cs.value.x = value;
                        m_corePipelines->clearTexture3du2.cs.value.y = value;
                        bindPipe(m_corePipelines->clearTexture3du2);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 3:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du3.cs.tex = texture;
                        m_corePipelines->clearTexture1du3.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du3.cs.value.x = value;
                        m_corePipelines->clearTexture1du3.cs.value.y = value;
                        m_corePipelines->clearTexture1du3.cs.value.z = value;
                        bindPipe(m_corePipelines->clearTexture1du3);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du3.cs.tex = texture;
                        m_corePipelines->clearTexture2du3.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du3.cs.value.x = value;
                        m_corePipelines->clearTexture2du3.cs.value.y = value;
                        m_corePipelines->clearTexture2du3.cs.value.z = value;
                        bindPipe(m_corePipelines->clearTexture2du3);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du3.cs.tex = texture;
                        m_corePipelines->clearTexture3du3.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du3.cs.value.x = value;
                        m_corePipelines->clearTexture3du3.cs.value.y = value;
                        m_corePipelines->clearTexture3du3.cs.value.z = value;
                        bindPipe(m_corePipelines->clearTexture3du3);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 4:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du4.cs.tex = texture;
                        m_corePipelines->clearTexture1du4.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du4.cs.value.x = value;
                        m_corePipelines->clearTexture1du4.cs.value.y = value;
                        m_corePipelines->clearTexture1du4.cs.value.z = value;
                        m_corePipelines->clearTexture1du4.cs.value.w = value;
                        bindPipe(m_corePipelines->clearTexture1du4);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du4.cs.tex = texture;
                        m_corePipelines->clearTexture2du4.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du4.cs.value.x = value;
                        m_corePipelines->clearTexture2du4.cs.value.y = value;
                        m_corePipelines->clearTexture2du4.cs.value.z = value;
                        m_corePipelines->clearTexture2du4.cs.value.w = value;
                        bindPipe(m_corePipelines->clearTexture2du4);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du4.cs.tex = texture;
                        m_corePipelines->clearTexture3du4.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du4.cs.value.x = value;
                        m_corePipelines->clearTexture3du4.cs.value.y = value;
                        m_corePipelines->clearTexture3du4.cs.value.z = value;
                        m_corePipelines->clearTexture3du4.cs.value.w = value;
                        bindPipe(m_corePipelines->clearTexture3du4);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
        }
    }

    void CommandList::clearTexture(TextureUAV texture, Vector4<uint32_t> value)
    {
        auto& m_corePipelines = m_impl->abs().m_corePipelines;

        switch (formatComponents(texture.format()))
        {
            case 1:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du.cs.tex = texture;
                        m_corePipelines->clearTexture1du.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du.cs.value.x = value.x;
                        bindPipe(m_corePipelines->clearTexture1du);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du.cs.tex = texture;
                        m_corePipelines->clearTexture2du.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du.cs.value.x = value.x;
                        bindPipe(m_corePipelines->clearTexture2du);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du.cs.tex = texture;
                        m_corePipelines->clearTexture3du.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du.cs.value.x = value.x;
                        bindPipe(m_corePipelines->clearTexture3du);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 2:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du2.cs.tex = texture;
                        m_corePipelines->clearTexture1du2.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du2.cs.value.x = value.x;
                        m_corePipelines->clearTexture1du2.cs.value.y = value.y;
                        bindPipe(m_corePipelines->clearTexture1du2);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du2.cs.tex = texture;
                        m_corePipelines->clearTexture2du2.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du2.cs.value.x = value.x;
                        m_corePipelines->clearTexture2du2.cs.value.y = value.y;
                        bindPipe(m_corePipelines->clearTexture2du2);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du2.cs.tex = texture;
                        m_corePipelines->clearTexture3du2.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du2.cs.value.x = value.x;
                        m_corePipelines->clearTexture3du2.cs.value.y = value.y;
                        bindPipe(m_corePipelines->clearTexture3du2);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 3:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du3.cs.tex = texture;
                        m_corePipelines->clearTexture1du3.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du3.cs.value.x = value.x;
                        m_corePipelines->clearTexture1du3.cs.value.y = value.y;
                        m_corePipelines->clearTexture1du3.cs.value.z = value.z;
                        bindPipe(m_corePipelines->clearTexture1du3);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du3.cs.tex = texture;
                        m_corePipelines->clearTexture2du3.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du3.cs.value.x = value.x;
                        m_corePipelines->clearTexture2du3.cs.value.y = value.y;
                        m_corePipelines->clearTexture2du3.cs.value.z = value.z;
                        bindPipe(m_corePipelines->clearTexture2du3);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du3.cs.tex = texture;
                        m_corePipelines->clearTexture3du3.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du3.cs.value.x = value.x;
                        m_corePipelines->clearTexture3du3.cs.value.y = value.y;
                        m_corePipelines->clearTexture3du3.cs.value.z = value.z;
                        bindPipe(m_corePipelines->clearTexture3du3);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
            case 4:
            {
                switch (texture.dimension())
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->clearTexture1du4.cs.tex = texture;
                        m_corePipelines->clearTexture1du4.cs.size = Vector4<size_t>{ texture.width(), 1u, 1u, 1u };
                        m_corePipelines->clearTexture1du4.cs.value.x = value.x;
                        m_corePipelines->clearTexture1du4.cs.value.y = value.y;
                        m_corePipelines->clearTexture1du4.cs.value.z = value.z;
                        m_corePipelines->clearTexture1du4.cs.value.w = value.w;
                        bindPipe(m_corePipelines->clearTexture1du4);
                        dispatch(roundUpToMultiple(texture.width(), 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->clearTexture2du4.cs.tex = texture;
                        m_corePipelines->clearTexture2du4.cs.size = Vector4<size_t>{ texture.width(), texture.height(), 1u, 1u };
                        m_corePipelines->clearTexture2du4.cs.value.x = value.x;
                        m_corePipelines->clearTexture2du4.cs.value.y = value.y;
                        m_corePipelines->clearTexture2du4.cs.value.z = value.z;
                        m_corePipelines->clearTexture2du4.cs.value.w = value.w;
                        bindPipe(m_corePipelines->clearTexture2du4);
                        dispatch(
                            roundUpToMultiple(texture.width(), 8u) / 8u,
                            roundUpToMultiple(texture.height(), 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->clearTexture3du4.cs.tex = texture;
                        m_corePipelines->clearTexture3du4.cs.size = Vector4<size_t>{ texture.width(), texture.height(), texture.depth(), 1u };
                        m_corePipelines->clearTexture3du4.cs.value.x = value.x;
                        m_corePipelines->clearTexture3du4.cs.value.y = value.y;
                        m_corePipelines->clearTexture3du4.cs.value.z = value.z;
                        m_corePipelines->clearTexture3du4.cs.value.w = value.w;
                        bindPipe(m_corePipelines->clearTexture3du4);
                        dispatch(
                            roundUpToMultiple(texture.width(), 4u) / 4u,
                            roundUpToMultiple(texture.height(), 4u) / 4u,
                            roundUpToMultiple(texture.depth(), 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unhandled Resource dimension");
                        break;
                    }
                }
                break;
            }
        }
    }

    void CommandList::copyBuffer(
        Buffer srcBuffer,
        Buffer dstBuffer,
        uint64_t elements,
        size_t srcStartElement,
        size_t dstStartElement)
    {
        if((srcBuffer.description().usage != ResourceUsage::Upload))
            transition(srcBuffer, ResourceState::CopySource);
        if ((dstBuffer.description().usage != ResourceUsage::Upload) && (m_impl->type() != CommandListType::Copy))
            transition(dstBuffer, ResourceState::CopyDest);
        m_impl->copyBuffer(
            srcBuffer, 
            dstBuffer, 
            elements,
            srcStartElement,
            dstStartElement);
    }

    void CommandList::copyBufferBytes(
        Buffer srcBuffer,
        Buffer dstBuffer,
        uint64_t bytes,
        size_t srcStartByte,
        size_t dstStartByte)
    {
        if ((srcBuffer.description().usage != ResourceUsage::Upload))
            transition(srcBuffer, ResourceState::CopySource);
        if ((dstBuffer.description().usage != ResourceUsage::Upload) && (m_impl->type() != CommandListType::Copy))
            transition(dstBuffer, ResourceState::CopyDest);
        m_impl->copyBufferBytes(
            srcBuffer,
            dstBuffer,
            bytes,
            srcStartByte,
            dstStartByte);
    }

    void CommandList::copyBufferIndirect(
        Buffer /*srcBuffer*/,
        Buffer /*dstBuffer*/,
        Buffer /*count*/,
        Buffer* /*srcBufferIndex*/,
        Buffer* /*dstBufferIndex*/,
        Buffer* /*countIndex*/)
    {
        //ASSERT(formatBytes(srcBuffer.description().descriptor.format) % 4 == 0, "Indirect buffer copy can only handle buffer formats that have 4 bytes align");
        //ASSERT(formatBytes(dstBuffer.description().descriptor.format) % 4 == 0, "Indirect buffer copy can only handle buffer formats that have 4 bytes align");
        //ASSERT(formatBytes(count.description().descriptor.format) % 4 == 0, "Indirect buffer copy can only handle buffer formats that have 4 bytes align");

		ASSERT(false, "This is terrible. Don't");
#if 0
        auto& m_device = m_impl->abs().m_device;

        auto srcFormatBytes = formatBytes(srcBuffer.description().descriptor.format);
        auto dstFormatBytes = formatBytes(dstBuffer.description().descriptor.format);
        auto countFormatBytes = formatBytes(count.description().descriptor.format);

        if (srcBuffer.description().descriptor.format == Format::UNKNOWN)
            srcFormatBytes = 4;
        if (dstBuffer.description().descriptor.format == Format::UNKNOWN)
            dstFormatBytes = 4;
        if (count.description().descriptor.format == Format::UNKNOWN)
            countFormatBytes = 4;

        BufferSRV srcBufferSRV = m_device->createBufferSRV(
            srcBuffer,
            BufferDescription()
            .format(Format::R32_UINT)
            .elements(srcFormatBytes * srcBuffer.description().descriptor.elements / sizeof(uint32_t))
            .name("copyBufferIndirect SrcSRV"));

        BufferUAV dstBufferUAV = m_device->createBufferUAV(
            dstBuffer,
            BufferDescription()
            .format(Format::R32_UINT)
            .elements(dstFormatBytes * dstBuffer.description().descriptor.elements / sizeof(uint32_t))
            .name("copyBufferIndirect DstUAV"));

        BufferSRV countSRV = m_device->createBufferSRV(BufferDescription()
            .format(Format::R32_UINT)
            .elements(1)
            .usage(ResourceUsage::GpuReadWrite)
            .name("copyBufferIndirect CountSRV"));
        BufferUAV countUAV = m_device->createBufferUAV(countSRV.buffer());
        
        BufferSRV srcIndexSRV;
        BufferUAV srcIndexUAV;
        if (srcBufferIndex)
        {
            srcIndexSRV = m_device->createBufferSRV(BufferDescription()
                .format(Format::R32_UINT)
                .elements(1)
                .usage(ResourceUsage::GpuReadWrite)
                .name("copyBufferIndirect SrcIndexSRV"));
            srcIndexUAV = m_device->createBufferUAV(srcIndexSRV.buffer());

            m_impl->abs().m_boundBuffers.emplace_back(*srcBufferIndex);
            m_impl->abs().m_boundBufferUAVs.emplace_back(srcIndexUAV);
            m_impl->abs().m_boundBufferSRVs.emplace_back(srcIndexSRV);
        }

        BufferSRV dstIndexSRV;
        BufferUAV dstIndexUAV;
        if (dstBufferIndex)
        {
            dstIndexSRV = m_device->createBufferSRV(BufferDescription()
                .format(Format::R32_UINT)
                .elements(1)
                .usage(ResourceUsage::GpuReadWrite)
                .name("copyBufferIndirect DstIndexSRV"));
            dstIndexUAV = m_device->createBufferUAV(dstIndexSRV.buffer());

            m_impl->abs().m_boundBuffers.emplace_back(*dstBufferIndex);
            m_impl->abs().m_boundBufferSRVs.emplace_back(dstIndexSRV);
            m_impl->abs().m_boundBufferUAVs.emplace_back(dstIndexUAV);
        }

        BufferSRV countIndexSRV;
        BufferUAV countIndexUAV;
        if (countIndex)
        {
            countIndexSRV = m_device->createBufferSRV(BufferDescription()
                .format(Format::R32_UINT)
                .elements(1)
                .usage(ResourceUsage::GpuReadWrite)
                .name("copyBufferIndirect CountIndexSRV"));
            countIndexUAV = m_device->createBufferUAV(countIndexSRV.buffer());

            m_impl->abs().m_boundBuffers.emplace_back(*countIndex);
            m_impl->abs().m_boundBufferSRVs.emplace_back(countIndexSRV);
            m_impl->abs().m_boundBufferUAVs.emplace_back(countIndexUAV);
        }

        m_impl->abs().m_boundBuffers.emplace_back(srcBuffer);
        m_impl->abs().m_boundBuffers.emplace_back(dstBuffer);
        m_impl->abs().m_boundBuffers.emplace_back(count);

        m_impl->abs().m_boundBufferUAVs.emplace_back(dstBufferUAV);
        m_impl->abs().m_boundBufferSRVs.emplace_back(srcBufferSRV);

        m_impl->abs().m_boundBufferUAVs.emplace_back(countUAV);
        m_impl->abs().m_boundBufferSRVs.emplace_back(countSRV);
        

        transition(srcBufferSRV, ResourceState::CopySource);
        transition(dstBufferUAV.buffer(), ResourceState::CopyDest);

        auto& m_corePipelines = *m_impl->abs().m_corePipelines;

        this->copyBuffer(count, countUAV.buffer(), 1);
        m_corePipelines.bufferMath->perform(
			*this, countUAV, BufferMathOperation::Multiplication, 1, 0, 
            static_cast<uint32_t>(srcFormatBytes) / static_cast<uint32_t>(sizeof(uint32_t)));

        if (srcBufferIndex)
        {
            this->copyBuffer(*srcBufferIndex, srcIndexUAV.buffer(), 1);
            m_corePipelines.bufferMath->perform(*this, srcIndexUAV, BufferMathOperation::Multiplication, 1, 0,
				static_cast<uint32_t>(srcFormatBytes) / static_cast<uint32_t>(sizeof(uint32_t)));
        }

        if (dstBufferIndex)
        {
            this->copyBuffer(*dstBufferIndex, dstIndexUAV.buffer(), 1);
            m_corePipelines.bufferMath->perform(*this, dstIndexUAV, BufferMathOperation::Multiplication, 1, 0,
				static_cast<uint32_t>(dstFormatBytes) / static_cast<uint32_t>(sizeof(uint32_t)));
        }

        if (countIndex)
        {
            this->copyBuffer(*countIndex, countIndexUAV.buffer(), 1);
            m_corePipelines.bufferMath->perform(*this, countIndexUAV, BufferMathOperation::Multiplication, 1, 0,
				static_cast<uint32_t>(countFormatBytes) / static_cast<uint32_t>(sizeof(uint32_t)));
        }

        m_corePipelines.copyBufferIndirect.cs.src = srcBufferSRV;
        m_corePipelines.copyBufferIndirect.cs.dst = dstBufferUAV;
        m_corePipelines.copyBufferIndirect.cs.count = countSRV;
        if (srcBufferIndex)
            m_corePipelines.copyBufferIndirect.cs.srcIndex = srcIndexSRV;
        if (dstBufferIndex)
            m_corePipelines.copyBufferIndirect.cs.dstIndex = dstIndexSRV;
        if (countIndex)
            m_corePipelines.copyBufferIndirect.cs.countIndex = countIndexSRV;

        this->bindPipe(m_corePipelines.copyBufferIndirect);
        this->dispatch(1, 1, 1);
#endif
    }

    void CommandList::bindPipe(
        implementation::PipelineImplIf* pipelineImpl,
        shaders::PipelineConfiguration* configuration)
    {
        if (m_impl->abs().m_api == GraphicsApi::DX12)
        {
            setDebugBuffers(configuration);
            m_impl->bindPipe(pipelineImpl, configuration);
            savePipeline(configuration);
        }
        else if (m_impl->abs().m_api == GraphicsApi::Vulkan)
        {
            savePipeline(configuration);
            setDebugBuffers(configuration);
            m_impl->bindPipe(pipelineImpl, configuration);
        }
    }

    void CommandList::setDebugBuffers(shaders::PipelineConfiguration* configuration)
    {
        auto saveBinding = [this](shaders::Shader* shader)
        {
            if (shader->hasDebugBuffer())
            {
                auto debugUav = m_impl->abs().m_device->createBufferUAV(BufferDescription()
                    .append(true)
                    .elementSize(sizeof(ShaderDebugOutput))
                    .elements(100000)
                    .name("shaderDebug output buffer")
                    .structured(true)
                    .usage(ResourceUsage::GpuReadWrite)
                );

                auto cmd = m_impl->abs().m_device->createCommandList("Debug buffers structure count");
                cmd.setStructureCounter(debugUav, 0);
                const_cast<Device*>(m_impl->abs().m_device)->submitBlocking(cmd);

                shader->setDebugBuffer(debugUav);

                this->m_impl->abs().m_debugBuffers.emplace_back(debugUav);
            }
        };

        if (configuration->hasVertexShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->vertexShader()));
        }
        if (configuration->hasPixelShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->pixelShader()));
        }
        if (configuration->hasComputeShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->computeShader()));
        }
        if (configuration->hasDomainShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->domainShader()));
        }
        if (configuration->hasGeometryShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->geometryShader()));
        }
        if (configuration->hasHullShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->hullShader()));
        }
    }

    void CommandList::savePipeline(shaders::PipelineConfiguration* configuration)
    {
        auto saveBinding = [this](shaders::Shader* shader, bool pixelShader)
        {
            const auto& srvs = shader->texture_srvs();
            const auto& uavs = shader->texture_uavs();
            const auto& bsrvs = shader->buffer_srvs();
            const auto& buavs = shader->buffer_uavs();

            const auto& bindless_srvs = shader->bindless_texture_srvs();
            const auto& bindless_uavs = shader->bindless_texture_uavs();
            const auto& bindless_bsrvs = shader->bindless_buffer_srvs();
            const auto& bindless_buavs = shader->bindless_buffer_uavs();

            for (auto&& srv : srvs)
            {
                if (srv.valid())
                {
                    if (srv.texture().description().usage != ResourceUsage::DepthStencil)
                    {
                        auto localSubRes = srv.subResource();

                        uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
                            static_cast<uint32_t>(srv.texture().arraySlices()) :
                            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.arraySliceCount), static_cast<uint32_t>(srv.texture().arraySlices()) - localSubRes.firstArraySlice));

                        uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
                            static_cast<uint32_t>(srv.texture().mipLevels()) :
                            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.mipCount), static_cast<uint32_t>(srv.texture().mipLevels()) - localSubRes.firstMipLevel));

                        for (uint32_t slice = localSubRes.firstArraySlice; slice < localSubRes.firstArraySlice + sliceCount; ++slice)
                        {
                            for (uint32_t mip = localSubRes.firstMipLevel; mip < localSubRes.firstMipLevel + mipCount; ++mip)
                            {
                                if (srv.texture().state(slice, mip) != ResourceState::Common)
                                    this->transition(srv.texture(), 
                                        pixelShader ? ResourceState::PixelShaderResource : ResourceState::NonPixelShaderResource,
                                        SubResource{ mip, 1, slice, 1 });
                                else
                                    srv.texture().state(slice, mip, pixelShader ? ResourceState::PixelShaderResource : ResourceState::NonPixelShaderResource);
                            }
                        }
                    }
                    else
                    {
                        if (m_impl->abs().m_api == GraphicsApi::DX12)
                        {
                            this->transition(srv.texture(), ResourceState::GenericRead);
                        }
                        else if (m_impl->abs().m_api == GraphicsApi::Vulkan)
                        {
                            this->transition(srv.texture(), ResourceState::PixelShaderResource);
                        }
                    }
                }
            }

            for (auto&& uav : uavs)
            {
                if (uav.valid())
                {
                    if (uav.texture().description().usage != ResourceUsage::DepthStencil)
                    {
                        auto localSubRes = uav.subResource();

                        uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
                            static_cast<uint32_t>(uav.texture().arraySlices()) :
                            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.arraySliceCount), static_cast<uint32_t>(uav.texture().arraySlices()) - localSubRes.firstArraySlice));

                        uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
                            static_cast<uint32_t>(uav.texture().mipLevels()) :
                            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.mipCount), static_cast<uint32_t>(uav.texture().mipLevels()) - localSubRes.firstMipLevel));

                        for (uint32_t slice = localSubRes.firstArraySlice; slice < localSubRes.firstArraySlice + sliceCount; ++slice)
                        {
                            for (uint32_t mip = localSubRes.firstMipLevel; mip < localSubRes.firstMipLevel + mipCount; ++mip)
                            {
                                this->transition(uav.texture(), ResourceState::UnorderedAccess, SubResource{ mip, 1, slice, 1 });
                            }
                        }
                    }
                    else
                        this->transition(uav.texture(), ResourceState::UnorderedAccess);
                }
            }
            
            for (auto&& bsrv : bsrvs)
            {
                if (bsrv.valid())
                {
                    if (bsrv.buffer().state() != ResourceState::Common)
                        this->transition(bsrv.buffer(), ResourceState::GenericRead);
                        //this->transition(bsrv.buffer(), pixelShader ? ResourceState::PixelShaderResource : ResourceState::NonPixelShaderResource);
                    else
                        bsrv.buffer().state(ResourceState::GenericRead);
                        //bsrv.buffer().state(pixelShader ? ResourceState::PixelShaderResource : ResourceState::NonPixelShaderResource);
                }
            }
            
            for (auto&& buav : buavs)
            { 
                if (buav.valid())
                {
                    this->transition(buav.buffer(), ResourceState::UnorderedAccess);
                }
            }

            for (auto&& _srv : bindless_srvs)
            {
				auto& srv = *const_cast<BindlessTextureSRV*>(_srv);
				
				if (srv.change())
				// this check is faulty. even though the resources might not change
				// the states of the resources can change and need to be reset
				{
					for (size_t i = 0; i < srv.size(); ++i)
					{
						TextureSRV texsrv = srv.get(i);
						if (texsrv.valid())
						{
							auto localSubRes = texsrv.subResource();

							uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
								static_cast<uint32_t>(texsrv.texture().arraySlices()) :
								static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.arraySliceCount), static_cast<uint32_t>(texsrv.texture().arraySlices()) - localSubRes.firstArraySlice));

							uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
								static_cast<uint32_t>(texsrv.texture().mipLevels()) :
								static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.mipCount), static_cast<uint32_t>(texsrv.texture().mipLevels()) - localSubRes.firstMipLevel));

							for (uint32_t slice = localSubRes.firstArraySlice; slice < localSubRes.firstArraySlice + sliceCount; ++slice)
							{
								for (uint32_t mip = localSubRes.firstMipLevel; mip < localSubRes.firstMipLevel + mipCount; ++mip)
								{
									if (texsrv.texture().description().usage != ResourceUsage::DepthStencil)
										this->transition(
											texsrv.texture(),
											pixelShader ? ResourceState::PixelShaderResource : ResourceState::NonPixelShaderResource,
											SubResource{ mip, 1, slice, 1 });
                                    else
                                    {
                                        if (m_impl->abs().m_api == GraphicsApi::DX12)
                                        {
                                            this->transition(texsrv.texture(), ResourceState::GenericRead);
                                        }
                                        else if (m_impl->abs().m_api == GraphicsApi::Vulkan)
                                        {
                                            this->transition(texsrv.texture(), ResourceState::PixelShaderResource);
                                        }
                                    }
								}
							}
						}
					}
					srv.change(false);
				}
            }

            for (const auto& _uav : bindless_uavs)
            {
				auto& uav = *const_cast<BindlessTextureUAV*>(_uav);
				//if (uav.change())
				{
					for (size_t i = 0; i < uav.size(); ++i)
					{
						TextureUAV texsrv = uav.get(i);
						if (texsrv.valid())
						{
							auto localSubRes = texsrv.subResource();

							uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
								static_cast<uint32_t>(texsrv.texture().arraySlices()) :
								static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.arraySliceCount), static_cast<uint32_t>(texsrv.texture().arraySlices()) - localSubRes.firstArraySlice));

							uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
								static_cast<uint32_t>(texsrv.texture().mipLevels()) :
								static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.mipCount), static_cast<uint32_t>(texsrv.texture().mipLevels()) - localSubRes.firstMipLevel));

							for (uint32_t slice = localSubRes.firstArraySlice; slice < localSubRes.firstArraySlice + sliceCount; ++slice)
							{
								for (uint32_t mip = localSubRes.firstMipLevel; mip < localSubRes.firstMipLevel + mipCount; ++mip)
								{
									if (texsrv.texture().description().usage != ResourceUsage::DepthStencil)
										this->transition(
											texsrv.texture(),
											ResourceState::UnorderedAccess,
											SubResource{ mip, 1, slice, 1 });
									else
										this->transition(texsrv.texture(), ResourceState::UnorderedAccess);
								}
							}
						}
					}
					//uav.change(false);
				}
            }

            for (auto&& _bsrv : bindless_bsrvs)
            {
				auto& bsrv = *const_cast<BindlessBufferSRV*>(_bsrv);
				//if (bsrv.change())
				{
					for (size_t i = 0; i < bsrv.size(); ++i)
					{
						if (bsrv.get(i).valid())
						{
							this->transition(bsrv.get(i).buffer(), pixelShader ? ResourceState::PixelShaderResource : ResourceState::NonPixelShaderResource);
						}
					}
					//bsrv.change(false);
				}
            }

            for (const auto& _buav : bindless_buavs)
            {
				auto& buav = *const_cast<BindlessBufferUAV*>(_buav);
				//if (buav.change())
				{
					for (size_t i = 0; i < buav.size(); ++i)
					{
						if (buav.get(i).valid())
						{
							this->transition(buav.get(i).buffer(), ResourceState::UnorderedAccess);
						}
					}
					//buav.change(false);
				}
            }
        };

        if (configuration->hasVertexShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->vertexShader()), false);
            /*for(auto&& range : const_cast<engine::shaders::Shader*>(configuration->vertexShader())->constants())
            {
                this->transition(range.buffer->buffer(), ResourceState::VertexAndConstantBuffer);
            }*/
        }
        if (configuration->hasPixelShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->pixelShader()), true);
            /*for (auto&& range : const_cast<engine::shaders::Shader*>(configuration->pixelShader())->constants())
            {
                if(range.buffer)
                    this->transition(range.buffer->buffer(), ResourceState::VertexAndConstantBuffer);
            }*/
        }
        if (configuration->hasComputeShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->computeShader()), false);
            /*for (auto&& range : const_cast<engine::shaders::Shader*>(configuration->computeShader())->constants())
            {
                if (range.buffer)
                    this->transition(range.buffer->buffer(), ResourceState::VertexAndConstantBuffer);
            }*/
        }
        if (configuration->hasDomainShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->domainShader()), false);
            /*for (auto&& range : const_cast<engine::shaders::Shader*>(configuration->domainShader())->constants())
            {
                if (range.buffer)
                    this->transition(range.buffer->buffer(), ResourceState::VertexAndConstantBuffer);
            }*/
        }
        if (configuration->hasGeometryShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->geometryShader()), false);
            /*for (auto&& range : const_cast<engine::shaders::Shader*>(configuration->geometryShader())->constants())
            {
                if (range.buffer)
                    this->transition(range.buffer->buffer(), ResourceState::VertexAndConstantBuffer);
            }*/
        }
        if (configuration->hasHullShader())
        {
            saveBinding(const_cast<engine::shaders::Shader*>(configuration->hullShader()), false);
            /*for (auto&& range : const_cast<engine::shaders::Shader*>(configuration->hullShader())->constants())
            {
                if (range.buffer)
                    this->transition(range.buffer->buffer(), ResourceState::VertexAndConstantBuffer);
            }*/
        }
    }

    void CommandList::setViewPorts(const engine::vector<Viewport>& viewports)
    {
        m_impl->setViewPorts(viewports);
    }

    void CommandList::setScissorRects(const engine::vector<Rectangle>& rects)
    {
        m_impl->setScissorRects(rects);
    }

    engine::vector<QueryResultTicks> CommandList::fetchQueryResults(uint64_t freq)
    {
        return m_impl->fetchQueryResults(freq);
    }

    void CommandList::bindVertexBuffer(BufferVBV buffer)
    {
        transition(buffer, ResourceState::VertexAndConstantBuffer);
        m_impl->bindVertexBuffer(buffer);
    }

    void CommandList::clearBuffer(BufferUAV buffer, uint32_t value, size_t startElement, size_t numElements)
    {
        transition(buffer.buffer(), ResourceState::UnorderedAccess);
        m_impl->clearBuffer(buffer, value, startElement, numElements);
    }

    void CommandList::clearBuffer(BufferUAV buffer, uint32_t value)
    {
        transition(buffer.buffer(), ResourceState::UnorderedAccess);
        m_impl->clearBuffer(buffer, value, 0, buffer.buffer().description().elements);
    }

    void CommandList::beginRenderPass(engine::shared_ptr<implementation::PipelineImplIf> pipeline, int /*frameBufferIndex*/)
    {
        //m_impl->beginRenderPass(*pipeline, frameBufferIndex);
    }

    void CommandList::begin()
    {
        m_impl->begin();
    }

    void CommandList::draw(size_t vertexCount)
    {
        m_impl->draw(vertexCount);
    }

    void CommandList::drawIndexedInstanced(
        BufferIBV buffer,
        size_t indexCount,
        size_t instanceCount,
        size_t firstIndex,
        int32_t vertexOffset,
        size_t firstInstance)
    {
        transition(buffer, ResourceState::IndexBuffer);
        m_impl->bindIndexBuffer(buffer);
        m_impl->drawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void CommandList::drawIndexedIndirect(BufferIBV buffer, Buffer indirectArguments, uint64_t argumentBufferOffset)
    {
        transition(buffer, ResourceState::IndexBuffer);
        m_impl->bindIndexBuffer(buffer);

        transition(indirectArguments, ResourceState::IndirectArgument);
        m_impl->drawIndexedIndirect(indirectArguments, argumentBufferOffset);
    }

    void CommandList::drawIndexedInstancedIndirect(
        BufferIBV indexBuffer,
        Buffer indirectArguments,
        uint64_t argumentBufferOffset,
        Buffer indirectArgumentsCountBuffer,
        uint64_t countBufferOffset)
    {
        transition(indexBuffer, ResourceState::IndexBuffer);
        transition(indirectArguments, ResourceState::IndirectArgument);
        transition(indirectArgumentsCountBuffer, ResourceState::IndirectArgument);
        m_impl->drawIndexedInstancedIndirect(
            indexBuffer,
            indirectArguments,
            argumentBufferOffset,
            indirectArgumentsCountBuffer,
            countBufferOffset);
    }

    void CommandList::dispatch(
        size_t threadGroupCountX,
        size_t threadGroupCountY,
        size_t threadGroupCountZ)
    {
        m_impl->dispatch(
            threadGroupCountX,
            threadGroupCountY,
            threadGroupCountZ);
    }

    void CommandList::drawIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset)
    {
        transition(indirectArguments, ResourceState::IndirectArgument);
        m_impl->drawIndirect(indirectArguments, argumentBufferOffset);
    }

    void CommandList::dispatchIndirect(Buffer indirectArgs, uint64_t argumentBufferOffset)
    {
        transition(indirectArgs, ResourceState::IndirectArgument);
        m_impl->dispatchIndirect(indirectArgs, argumentBufferOffset);
    }

    void CommandList::executeIndexedIndirect(
        BufferIBV indexBuffer,
        Buffer indirectArguments,
        uint64_t argumentBufferOffset,
        Buffer countBuffer,
        uint64_t countBufferOffsetBytes)
    {
        transition(indexBuffer, ResourceState::IndexBuffer);
        transition(indirectArguments, ResourceState::IndirectArgument);
        transition(countBuffer, ResourceState::IndirectArgument);
        m_impl->executeIndexedIndirect(
            indexBuffer,
            indirectArguments, argumentBufferOffset,
            countBuffer, countBufferOffsetBytes);
    }

    void CommandList::executeBundle(CommandList& commandList)
    {
        m_impl->executeBundle(commandList.native());
    }

    void CommandList::dispatchMesh(
        size_t threadGroupCountX,
        size_t threadGroupCountY,
        size_t threadGroupCountZ)
    {
        m_impl->dispatchMesh(
            threadGroupCountX,
            threadGroupCountY,
            threadGroupCountZ);
    }

    void CommandList::end()
    {
        m_impl->end();
    }

    void CommandList::endRenderPass()
    {
        //m_impl->endRenderPass();
    }

    /*void CommandList::transitionTexture(const Texture& image, ImageLayout from, ImageLayout to)
    {
        m_impl->transitionTexture(image, from, to);
    }*/

    void CommandList::copyTexture(
        TextureSRV src,
        TextureUAV dst,
        size_t srcLeft,
        size_t srcTop,
        size_t srcFront,
        size_t dstLeft,
        size_t dstTop,
        size_t dstFront,
        size_t width,
        size_t height,
        size_t depth)
    {
        ASSERT(src.texture().description().dimension == dst.texture().description().dimension,
            "CopyTexture failed. Src and Dst need to have the same dimension (1D, 2D, 3D)");

        auto& m_corePipelines = m_impl->abs().m_corePipelines;

        switch (formatComponents(src.texture().format()))
        {
            case 1:
            {
                switch (src.texture().description().dimension)
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->copyTexture1df.cs.src = src;
                        m_corePipelines->copyTexture1df.cs.dst = dst;
                        m_corePipelines->copyTexture1df.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture1df.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture1df.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture1df.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture1df.cs.copyWidth.x = static_cast<uint32_t>(width);
                        bindPipe(m_corePipelines->copyTexture1df);
                        dispatch(roundUpToMultiple(width, 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->copyTexture2df.cs.src = src;
                        m_corePipelines->copyTexture2df.cs.dst = dst;
                        m_corePipelines->copyTexture2df.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture2df.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture2df.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture2df.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture2df.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture2df.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture2df.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture2df.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture2df.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture2df.cs.copyHeight.x = static_cast<uint32_t>(height);
                        bindPipe(m_corePipelines->copyTexture2df);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->copyTexture3df.cs.src = src;
                        m_corePipelines->copyTexture3df.cs.dst = dst;
                        m_corePipelines->copyTexture3df.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture3df.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture3df.cs.srcFront.x = static_cast<uint32_t>(srcFront);
                        m_corePipelines->copyTexture3df.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture3df.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture3df.cs.srcTextureHeight.x = static_cast<uint32_t>(src.depth());
                        m_corePipelines->copyTexture3df.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture3df.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture3df.cs.dstFront.x = static_cast<uint32_t>(dstFront);
                        m_corePipelines->copyTexture3df.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture3df.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture3df.cs.dstTextureDepth.x = static_cast<uint32_t>(dst.depth());
                        m_corePipelines->copyTexture3df.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture3df.cs.copyHeight.x = static_cast<uint32_t>(height);
                        m_corePipelines->copyTexture3df.cs.copyDepth.x = static_cast<uint32_t>(depth);
                        bindPipe(m_corePipelines->copyTexture3df);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unknown resource dimension");
                        break;
                    }
                }
                break;
            }
            case 2:
            {
                switch (src.texture().description().dimension)
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->copyTexture1df2.cs.src = src;
                        m_corePipelines->copyTexture1df2.cs.dst = dst;
                        m_corePipelines->copyTexture1df2.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture1df2.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture1df2.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture1df2.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture1df2.cs.copyWidth.x = static_cast<uint32_t>(width);
                        bindPipe(m_corePipelines->copyTexture1df2);
                        dispatch(roundUpToMultiple(width, 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->copyTexture2df2.cs.src = src;
                        m_corePipelines->copyTexture2df2.cs.dst = dst;
                        m_corePipelines->copyTexture2df2.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture2df2.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture2df2.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture2df2.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture2df2.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture2df2.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture2df2.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture2df2.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture2df2.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture2df2.cs.copyHeight.x = static_cast<uint32_t>(height);
                        bindPipe(m_corePipelines->copyTexture2df2);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->copyTexture3df2.cs.src = src;
                        m_corePipelines->copyTexture3df2.cs.dst = dst;
                        m_corePipelines->copyTexture3df2.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture3df2.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture3df2.cs.srcFront.x = static_cast<uint32_t>(srcFront);
                        m_corePipelines->copyTexture3df2.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture3df2.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture3df2.cs.srcTextureHeight.x = static_cast<uint32_t>(src.depth());
                        m_corePipelines->copyTexture3df2.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture3df2.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture3df2.cs.dstFront.x = static_cast<uint32_t>(dstFront);
                        m_corePipelines->copyTexture3df2.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture3df2.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture3df2.cs.dstTextureDepth.x = static_cast<uint32_t>(dst.depth());
                        m_corePipelines->copyTexture3df2.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture3df2.cs.copyHeight.x = static_cast<uint32_t>(height);
                        m_corePipelines->copyTexture3df2.cs.copyDepth.x = static_cast<uint32_t>(depth);
                        bindPipe(m_corePipelines->copyTexture3df2);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unknown resource dimension");
                        break;
                    }
                }
                break;
            }
            case 3:
            {
                switch (src.texture().description().dimension)
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->copyTexture1df3.cs.src = src;
                        m_corePipelines->copyTexture1df3.cs.dst = dst;
                        m_corePipelines->copyTexture1df3.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture1df3.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture1df3.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture1df3.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture1df3.cs.copyWidth.x = static_cast<uint32_t>(width);
                        bindPipe(m_corePipelines->copyTexture1df3);
                        dispatch(roundUpToMultiple(width, 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->copyTexture2df3.cs.src = src;
                        m_corePipelines->copyTexture2df3.cs.dst = dst;
                        m_corePipelines->copyTexture2df3.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture2df3.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture2df3.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture2df3.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture2df3.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture2df3.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture2df3.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture2df3.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture2df3.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture2df3.cs.copyHeight.x = static_cast<uint32_t>(height);
                        bindPipe(m_corePipelines->copyTexture2df3);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->copyTexture3df3.cs.src = src;
                        m_corePipelines->copyTexture3df3.cs.dst = dst;
                        m_corePipelines->copyTexture3df3.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture3df3.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture3df3.cs.srcFront.x = static_cast<uint32_t>(srcFront);
                        m_corePipelines->copyTexture3df3.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture3df3.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture3df3.cs.srcTextureHeight.x = static_cast<uint32_t>(src.depth());
                        m_corePipelines->copyTexture3df3.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture3df3.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture3df3.cs.dstFront.x = static_cast<uint32_t>(dstFront);
                        m_corePipelines->copyTexture3df3.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture3df3.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture3df3.cs.dstTextureDepth.x = static_cast<uint32_t>(dst.depth());
                        m_corePipelines->copyTexture3df3.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture3df3.cs.copyHeight.x = static_cast<uint32_t>(height);
                        m_corePipelines->copyTexture3df3.cs.copyDepth.x = static_cast<uint32_t>(depth);
                        bindPipe(m_corePipelines->copyTexture3df3);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unknown resource dimension");
                        break;
                    }
                }
                break;
            }
            case 4:
            {
                switch (src.texture().description().dimension)
                {
                    case ResourceDimension::Texture1D:
                    {
                        m_corePipelines->copyTexture1df4.cs.src = src;
                        m_corePipelines->copyTexture1df4.cs.dst = dst;
                        m_corePipelines->copyTexture1df4.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture1df4.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture1df4.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture1df4.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture1df4.cs.copyWidth.x = static_cast<uint32_t>(width);
                        bindPipe(m_corePipelines->copyTexture1df4);
                        dispatch(roundUpToMultiple(width, 64u) / 64u, 1, 1);
                        break;
                    }
                    case ResourceDimension::Texture2D:
                    {
                        m_corePipelines->copyTexture2df4.cs.src = src;
                        m_corePipelines->copyTexture2df4.cs.dst = dst;
                        m_corePipelines->copyTexture2df4.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture2df4.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture2df4.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture2df4.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture2df4.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture2df4.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture2df4.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture2df4.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture2df4.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture2df4.cs.copyHeight.x = static_cast<uint32_t>(height);
                        bindPipe(m_corePipelines->copyTexture2df4);
                        dispatch(
                            roundUpToMultiple(width, 8u) / 8u,
                            roundUpToMultiple(height, 8u) / 8u,
                            1);
                        break;
                    }
                    case ResourceDimension::Texture3D:
                    {
                        m_corePipelines->copyTexture3df4.cs.src = src;
                        m_corePipelines->copyTexture3df4.cs.dst = dst;
                        m_corePipelines->copyTexture3df4.cs.srcTop.x = static_cast<uint32_t>(srcTop);
                        m_corePipelines->copyTexture3df4.cs.srcLeft.x = static_cast<uint32_t>(srcLeft);
                        m_corePipelines->copyTexture3df4.cs.srcFront.x = static_cast<uint32_t>(srcFront);
                        m_corePipelines->copyTexture3df4.cs.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                        m_corePipelines->copyTexture3df4.cs.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                        m_corePipelines->copyTexture3df4.cs.srcTextureHeight.x = static_cast<uint32_t>(src.depth());
                        m_corePipelines->copyTexture3df4.cs.dstTop.x = static_cast<uint32_t>(dstTop);
                        m_corePipelines->copyTexture3df4.cs.dstLeft.x = static_cast<uint32_t>(dstLeft);
                        m_corePipelines->copyTexture3df4.cs.dstFront.x = static_cast<uint32_t>(dstFront);
                        m_corePipelines->copyTexture3df4.cs.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                        m_corePipelines->copyTexture3df4.cs.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                        m_corePipelines->copyTexture3df4.cs.dstTextureDepth.x = static_cast<uint32_t>(dst.depth());
                        m_corePipelines->copyTexture3df4.cs.copyWidth.x = static_cast<uint32_t>(width);
                        m_corePipelines->copyTexture3df4.cs.copyHeight.x = static_cast<uint32_t>(height);
                        m_corePipelines->copyTexture3df4.cs.copyDepth.x = static_cast<uint32_t>(depth);
                        bindPipe(m_corePipelines->copyTexture3df4);
                        dispatch(
                            roundUpToMultiple(width, 4u) / 4u,
                            roundUpToMultiple(height, 4u) / 4u,
                            roundUpToMultiple(depth, 4u) / 4u);
                        break;
                    }
                    default:
                    {
                        ASSERT(false, "Unknown resource dimension");
                        break;
                    }
                }
                break;
            }
        }
    }

    GraphicsApi CommandList::api() const
    {
        return m_impl->abs().m_api;
    }

    void CommandList::copyTexture(
        TextureSRV src,
        TextureRTV dst,
        size_t srcLeft,
        size_t srcTop,
        size_t /*srcFront*/,
        size_t dstLeft,
        size_t dstTop,
        size_t /*dstFront*/,
        size_t width,
        size_t height,
        size_t /*depth*/)
    {
        //ASSERT(src.dimension() == dst.texture().description().descriptor.dimension,
        //    "CopyTexture failed. Src and Dst need to have the same dimension (1D, 2D, 3D)");
        
        ASSERT(src.dimension() == ResourceDimension::Texture2D);

        auto& m_corePipelines = m_impl->abs().m_corePipelines;

        setRenderTargets({ dst });

        switch (formatComponents(src.texture().format()))
        {
            case 1:
            {
                m_corePipelines->copyTextureRTV2df.ps.src = src;
                m_corePipelines->copyTextureRTV2df.ps.srcTop.x = static_cast<uint32_t>(srcTop);
                m_corePipelines->copyTextureRTV2df.ps.srcLeft.x = static_cast<uint32_t>(srcLeft);
                m_corePipelines->copyTextureRTV2df.ps.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                m_corePipelines->copyTextureRTV2df.ps.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                m_corePipelines->copyTextureRTV2df.ps.dstTop.x = static_cast<uint32_t>(dstTop);
                m_corePipelines->copyTextureRTV2df.ps.dstLeft.x = static_cast<uint32_t>(dstLeft);
                m_corePipelines->copyTextureRTV2df.ps.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                m_corePipelines->copyTextureRTV2df.ps.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                m_corePipelines->copyTextureRTV2df.ps.copyWidth.x = static_cast<uint32_t>(width);
                m_corePipelines->copyTextureRTV2df.ps.copyHeight.x = static_cast<uint32_t>(height);
                bindPipe(m_corePipelines->copyTextureRTV2df);
                draw(3u);
                break;
            }
            case 2:
            {
                m_corePipelines->copyTextureRTV2df2.ps.src = src;
                m_corePipelines->copyTextureRTV2df2.ps.srcTop.x = static_cast<uint32_t>(srcTop);
                m_corePipelines->copyTextureRTV2df2.ps.srcLeft.x = static_cast<uint32_t>(srcLeft);
                m_corePipelines->copyTextureRTV2df2.ps.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                m_corePipelines->copyTextureRTV2df2.ps.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                m_corePipelines->copyTextureRTV2df2.ps.dstTop.x = static_cast<uint32_t>(dstTop);
                m_corePipelines->copyTextureRTV2df2.ps.dstLeft.x = static_cast<uint32_t>(dstLeft);
                m_corePipelines->copyTextureRTV2df2.ps.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                m_corePipelines->copyTextureRTV2df2.ps.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                m_corePipelines->copyTextureRTV2df2.ps.copyWidth.x = static_cast<uint32_t>(width);
                m_corePipelines->copyTextureRTV2df2.ps.copyHeight.x = static_cast<uint32_t>(height);
                bindPipe(m_corePipelines->copyTextureRTV2df2);
                draw(3u);
                break;
            }
            case 3:
            {
                if (formatComponents(dst.texture().format()) == 3)
                {
                    m_corePipelines->copyTextureRTV2df3.ps.src = src;
                    m_corePipelines->copyTextureRTV2df3.ps.srcTop.x = static_cast<uint32_t>(srcTop);
                    m_corePipelines->copyTextureRTV2df3.ps.srcLeft.x = static_cast<uint32_t>(srcLeft);
                    m_corePipelines->copyTextureRTV2df3.ps.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                    m_corePipelines->copyTextureRTV2df3.ps.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                    m_corePipelines->copyTextureRTV2df3.ps.dstTop.x = static_cast<uint32_t>(dstTop);
                    m_corePipelines->copyTextureRTV2df3.ps.dstLeft.x = static_cast<uint32_t>(dstLeft);
                    m_corePipelines->copyTextureRTV2df3.ps.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                    m_corePipelines->copyTextureRTV2df3.ps.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                    m_corePipelines->copyTextureRTV2df3.ps.copyWidth.x = static_cast<uint32_t>(width);
                    m_corePipelines->copyTextureRTV2df3.ps.copyHeight.x = static_cast<uint32_t>(height);
                    bindPipe(m_corePipelines->copyTextureRTV2df3);
                    draw(3u);
                }
                else if (formatComponents(dst.texture().format()) == 4)
                {
                    m_corePipelines->copyTextureRTV2df34.ps.src = src;
                    m_corePipelines->copyTextureRTV2df34.ps.srcTop.x = static_cast<uint32_t>(srcTop);
                    m_corePipelines->copyTextureRTV2df34.ps.srcLeft.x = static_cast<uint32_t>(srcLeft);
                    m_corePipelines->copyTextureRTV2df34.ps.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                    m_corePipelines->copyTextureRTV2df34.ps.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                    m_corePipelines->copyTextureRTV2df34.ps.dstTop.x = static_cast<uint32_t>(dstTop);
                    m_corePipelines->copyTextureRTV2df34.ps.dstLeft.x = static_cast<uint32_t>(dstLeft);
                    m_corePipelines->copyTextureRTV2df34.ps.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                    m_corePipelines->copyTextureRTV2df34.ps.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                    m_corePipelines->copyTextureRTV2df34.ps.copyWidth.x = static_cast<uint32_t>(width);
                    m_corePipelines->copyTextureRTV2df34.ps.copyHeight.x = static_cast<uint32_t>(height);
                    bindPipe(m_corePipelines->copyTextureRTV2df34);
                    draw(3u);
                }
                break;
            }
            case 4:
            {
                m_corePipelines->copyTextureRTV2df4.ps.src = src;
                m_corePipelines->copyTextureRTV2df4.ps.srcTop.x = static_cast<uint32_t>(srcTop);
                m_corePipelines->copyTextureRTV2df4.ps.srcLeft.x = static_cast<uint32_t>(srcLeft);
                m_corePipelines->copyTextureRTV2df4.ps.srcTextureWidth.x = static_cast<uint32_t>(src.width());
                m_corePipelines->copyTextureRTV2df4.ps.srcTextureHeight.x = static_cast<uint32_t>(src.height());
                m_corePipelines->copyTextureRTV2df4.ps.dstTop.x = static_cast<uint32_t>(dstTop);
                m_corePipelines->copyTextureRTV2df4.ps.dstLeft.x = static_cast<uint32_t>(dstLeft);
                m_corePipelines->copyTextureRTV2df4.ps.dstTextureWidth.x = static_cast<uint32_t>(dst.width());
                m_corePipelines->copyTextureRTV2df4.ps.dstTextureHeight.x = static_cast<uint32_t>(dst.height());
                m_corePipelines->copyTextureRTV2df4.ps.copyWidth.x = static_cast<uint32_t>(width);
                m_corePipelines->copyTextureRTV2df4.ps.copyHeight.x = static_cast<uint32_t>(height);
                bindPipe(m_corePipelines->copyTextureRTV2df4);
                draw(3u);
                break;
            }
        }
    }

    void CommandList::transitionCommonSRV(TextureSRV srv, ResourceState state)
    {
        auto localSubRes = srv.subResource();
        
        uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ? 
            static_cast<uint32_t>(srv.texture().arraySlices()) : 
            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.arraySliceCount), static_cast<uint32_t>(srv.texture().arraySlices()) - localSubRes.firstArraySlice));

        uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
            static_cast<uint32_t>(srv.texture().mipLevels()) :
            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.mipCount), static_cast<uint32_t>(srv.texture().mipLevels()) - localSubRes.firstMipLevel));

        for (uint32_t slice = localSubRes.firstArraySlice; slice < localSubRes.firstArraySlice + sliceCount; ++slice)
        {
            for (uint32_t mip = localSubRes.firstMipLevel; mip < localSubRes.firstMipLevel + mipCount; ++mip)
            {
                if(srv.texture().state(slice, mip) != ResourceState::Common)
                    transition(srv.texture(), state, SubResource{ mip, 1, slice, 1 });
                srv.texture().state(slice, mip, state);
            }
        }
    }

    void CommandList::transitionCommonUAV(TextureUAV uav, ResourceState state)
    {
        auto localSubRes = uav.subResource();

        uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
            static_cast<uint32_t>(uav.texture().arraySlices()) :
            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.arraySliceCount), static_cast<uint32_t>(uav.texture().arraySlices()) - localSubRes.firstArraySlice));

        uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
            static_cast<uint32_t>(uav.texture().mipLevels()) :
            static_cast<uint32_t>(std::min(static_cast<uint32_t>(localSubRes.mipCount), static_cast<uint32_t>(uav.texture().mipLevels()) - localSubRes.firstMipLevel));

        for (uint32_t slice = localSubRes.firstArraySlice; slice < localSubRes.firstArraySlice + sliceCount; ++slice)
        {
            for (uint32_t mip = localSubRes.firstMipLevel; mip < localSubRes.firstMipLevel + mipCount; ++mip)
            {
                if (uav.texture().state(slice, mip) != ResourceState::Common)
                    transition(uav.texture(), state, SubResource{ mip, 1, slice, 1 });
                uav.texture().state(slice, mip, state);
            }
        }
    }

    void CommandList::copyTexture(TextureSRV src, TextureUAV dst)
    {
        transition(src, ResourceState::CopySource);
        transitionCommonUAV(dst, ResourceState::CopyDest);
        copyTexture(src, dst, 0, 0, 0, 0, 0, 0, src.width(), src.height(), src.depth());
    }

	void CommandList::copyTexture(TextureSRV src, TextureDSV dst)
	{
		transition(src, ResourceState::CopySource);
		copyDepthTexture(src, dst);
	}

    void CommandList::copyTexture(TextureSRV src, TextureRTV dst)
    {
        transition(src, ResourceState::CopySource);
        copyTexture(src, dst, 0, 0, 0, 0, 0, 0, src.width(), src.height(), src.depth());
    }

	void CommandList::copyDepthTexture(TextureSRV src, TextureDSV dst)
	{
		auto& m_corePipelines = m_impl->abs().m_corePipelines;

		DepthStencilOpDescription front;
		front.StencilFailOp = StencilOp::Keep;
		front.StencilDepthFailOp = StencilOp::Incr;
		front.StencilPassOp = StencilOp::Keep;
		front.StencilFunc = ComparisonFunction::Always;

		DepthStencilOpDescription back;
		back.StencilFailOp = StencilOp::Keep;
		back.StencilDepthFailOp = StencilOp::Decr;
		back.StencilPassOp = StencilOp::Keep;
		back.StencilFunc = ComparisonFunction::Always;

		m_corePipelines->copyDepthTexture.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
		m_corePipelines->copyDepthTexture.setRasterizerState(RasterizerDescription().cullMode(CullMode::None));
		m_corePipelines->copyDepthTexture.setDepthStencilState(DepthStencilDescription()
			.depthEnable(true)
			.depthWriteMask(DepthWriteMask::All)
			.depthFunc(ComparisonFunction::GreaterEqual)
			.frontFace(front)
			.backFace(back));

		m_corePipelines->copyDepthTexture.ps.srcDepth = src;
		m_corePipelines->copyDepthTexture.ps.mip.x = 0;// src.subResource().firstMipLevel;

		/*m_impl->abs().m_lastSetRTVFormats.clear();
		m_impl->abs().m_lastSetRTVFormats.emplace_back(dst.format());
		//transition(dst, ResourceState::RenderTarget);
		transition(dst, ResourceState::DepthWrite);
		m_impl->setRenderTargets(dst);*/
		setRenderTargets({}, dst);
		m_corePipelines->copyDepthTexture.setRenderTargetFormats({}, Format::D32_FLOAT);
		bindPipe(m_corePipelines->copyDepthTexture.m_impl.get(), &m_corePipelines->copyDepthTexture);
		draw(4);
	}

    void CommandList::copyTexture(TextureSRV src, TextureSRV dst)
    {
        transition(src, ResourceState::CopySource);
        transitionCommonSRV(dst, ResourceState::CopyDest);

        m_impl->copyTexture(src, dst);
    }

    void CommandList::copyTexture(TextureSRV src, BufferUAV dst)
    {
        transition(src, ResourceState::CopySource);

        if (dst.buffer().state() != ResourceState::Common)
            transition(dst.buffer(), ResourceState::CopyDest);
        dst.buffer().state(ResourceState::CopyDest);
        m_impl->copyTexture(src, dst);
    }

    void CommandList::copyTexture(TextureSRV src, BufferSRV dst)
    {
        transition(src, ResourceState::CopySource);

        if (dst.buffer().state() != ResourceState::Common)
            transition(dst.buffer(), ResourceState::CopyDest);
        dst.buffer().state(ResourceState::CopyDest);
        m_impl->copyTexture(src, dst);
    }

	void CommandList::copyTexture(
		Buffer		srcBuffer,
		size_t		srcOffset,
		size_t		srcWidth,
		size_t		srcHeight,
		size_t		srcRowPitch,
		Texture		dst,
		size_t		dstX,
		size_t		dstY,
		size_t		dstMip,
		size_t		dstSlice,
		size_t		dstMipCount)
	{
        if (m_impl->abs().m_api == GraphicsApi::Vulkan)
        {
            transition(srcBuffer, ResourceState::CopySource);
            transition(dst, ResourceState::CopyDest);
        }

		m_impl->copyTexture(
			srcBuffer, 
			srcOffset, 
			srcWidth, 
			srcHeight, 
			srcRowPitch, 
			dst,
			dstX,
			dstY,
			dstMip,
			dstSlice,
			dstMipCount);
	}

    void CommandList::setStructureCounter(BufferUAV buffer, uint32_t value)
    {
        m_impl->setStructureCounter(buffer, value);
    }

    void CommandList::copyStructureCounter(BufferUAV srcBuffer, Buffer dst, uint32_t dstByteOffset)
    {
        transition(srcBuffer.buffer(), ResourceState::CopySource);
        transition(dst, ResourceState::CopyDest);
        m_impl->copyStructureCounter(srcBuffer, dst, dstByteOffset);
    }
}
