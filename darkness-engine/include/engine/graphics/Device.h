#pragma once

#include "engine/graphics/DeviceImplIf.h"
#include "tools/ByteRange.h"
#include "containers/memory.h"
#include "containers/vector.h"
#include "containers/string.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/ResourceCache.h"
#include "engine/graphics/Fence.h"
#include "shaders/ShaderTypes.h"
#include "engine/graphics/Pipeline.h"
#include "engine/rendering/ModelResources.h"
#include "engine/graphics/ShaderStorage.h"

#include "shaders/core/instance/RemoveElement.h"

#include "shaders/core/tools/cleartexture/ClearTexture1df.h"
#include "shaders/core/tools/cleartexture/ClearTexture2df.h"
#include "shaders/core/tools/cleartexture/ClearTexture3df.h"
#include "shaders/core/tools/cleartexture/ClearTexture1df2.h"
#include "shaders/core/tools/cleartexture/ClearTexture2df2.h"
#include "shaders/core/tools/cleartexture/ClearTexture3df2.h"
#include "shaders/core/tools/cleartexture/ClearTexture1df3.h"
#include "shaders/core/tools/cleartexture/ClearTexture2df3.h"
#include "shaders/core/tools/cleartexture/ClearTexture3df3.h"
#include "shaders/core/tools/cleartexture/ClearTexture1df4.h"
#include "shaders/core/tools/cleartexture/ClearTexture2df4.h"
#include "shaders/core/tools/cleartexture/ClearTexture3df4.h"
#include "shaders/core/tools/cleartexture/ClearTexture1du.h"
#include "shaders/core/tools/cleartexture/ClearTexture2du.h"
#include "shaders/core/tools/cleartexture/ClearTexture3du.h"
#include "shaders/core/tools/cleartexture/ClearTexture1du2.h"
#include "shaders/core/tools/cleartexture/ClearTexture2du2.h"
#include "shaders/core/tools/cleartexture/ClearTexture3du2.h"
#include "shaders/core/tools/cleartexture/ClearTexture1du3.h"
#include "shaders/core/tools/cleartexture/ClearTexture2du3.h"
#include "shaders/core/tools/cleartexture/ClearTexture3du3.h"
#include "shaders/core/tools/cleartexture/ClearTexture1du4.h"
#include "shaders/core/tools/cleartexture/ClearTexture2du4.h"
#include "shaders/core/tools/cleartexture/ClearTexture3du4.h"

#include "shaders/core/tools/copytexture/CopyTexture1df.h"
#include "shaders/core/tools/copytexture/CopyTexture2df.h"
#include "shaders/core/tools/copytexture/CopyTexture3df.h"
#include "shaders/core/tools/copytexture/CopyTexture1df2.h"
#include "shaders/core/tools/copytexture/CopyTexture2df2.h"
#include "shaders/core/tools/copytexture/CopyTexture3df2.h"
#include "shaders/core/tools/copytexture/CopyTexture1df3.h"
#include "shaders/core/tools/copytexture/CopyTexture2df3.h"
#include "shaders/core/tools/copytexture/CopyTexture3df3.h"
#include "shaders/core/tools/copytexture/CopyTexture1df4.h"
#include "shaders/core/tools/copytexture/CopyTexture2df4.h"
#include "shaders/core/tools/copytexture/CopyTexture3df4.h"
#include "shaders/core/tools/copytexture/CopyDepthTexture.h"

#include "shaders/core/tools/copytexturertv/CopyTextureRTV2df.h"
#include "shaders/core/tools/copytexturertv/CopyTextureRTV2df2.h"
#include "shaders/core/tools/copytexturertv/CopyTextureRTV2df3.h"
#include "shaders/core/tools/copytexturertv/CopyTextureRTV2df4.h"
#include "shaders/core/tools/copytexturertv/CopyTextureRTV2df34.h"

#include "shaders/core/tools/CopyBufferIndirect.h"

#include <queue>
#include <mutex>

namespace platform
{
    class Window;
}

namespace engine
{
    namespace implementation
    {
        class CommandListImplIf;
		class DeviceImplIf;

		class BufferImplIf;
		class BufferSRVImplIf;
		class BufferUAVImplIf;
		class BufferIBVImplIf;
		class BufferCBVImplIf;
		class BufferVBVImplIf;
        class BindlessBufferSRVImplIf;
        class BindlessBufferUAVImplIf;

		class TextureImplIf;
		class TextureSRVImplIf;
		class TextureUAVImplIf;
		class TextureDSVImplIf;
		class TextureRTVImplIf;
        class BindlessTextureSRVImplIf;
        class BindlessTextureUAVImplIf;
    }

    namespace image
    {
        class Image;
    }
    class Mesh;

    class Queue;
    class SwapChain;
    struct BufferDescription;
    struct TextureDescription;

    class Buffer;
    class BufferView;
    class BufferSRV;
    class BufferUAV;
    class BufferIBV;
    class BufferCBV;
    class BufferVBV;
    class BindlessBufferSRV;
    class BindlessBufferUAV;

	class BufferOwner;
	class BufferSRVOwner;
	class BufferUAVOwner;
	class BufferIBVOwner;
	class BufferCBVOwner;
	class BufferVBVOwner;
    class BindlessBufferSRVOwner;
    class BindlessBufferUAVOwner;

	class TextureOwner;
	class TextureSRVOwner;
	class TextureUAVOwner;
	class TextureDSVOwner;
	class TextureRTVOwner;
    class BindlessTextureSRVOwner;
    class BindlessTextureUAVOwner;

    
    class Texture;
    class TextureSRV;
    class TextureUAV;
    class TextureDSV;
    class TextureRTV;
    class TextureView;
    class BindlessTextureSRV;
    class BindlessTextureUAV;

    class CommandList;
    class ShaderBinary;
    class Fence;
    class Semaphore;
    class RootSignature;
    class Sampler;
    struct SamplerDescription;
    enum class Format;

	class ResidencyManagerV2;
	class FontManager;

    class BufferMath;

    struct NullResources
    {
        BufferSRVOwner bufferSRV;
        BufferUAVOwner bufferUAV;
		BufferSRVOwner bufferStructuredSRV;
		BufferUAVOwner bufferStructuredUAV;
        TextureSRVOwner textureSRV;
        TextureSRVOwner textureCubeSRV;
        TextureUAVOwner textureUAV;
        engine::unordered_map<engine::ResourceDimension, engine::unordered_map<engine::Format, TextureSRVOwner>> nullTextureSRV;
        engine::unordered_map<engine::ResourceDimension, engine::unordered_map<engine::Format, TextureUAVOwner>> nullTextureUAV;
        engine::unordered_map<engine::Format, BufferSRVOwner> nullBufferSRV;
        engine::unordered_map<engine::Format, BufferUAVOwner> nullBufferUAV;
        Sampler sampler;
    };

    struct CorePipelines
    {
        engine::Pipeline<shaders::RemoveElement> removeElement;

        engine::Pipeline<shaders::ClearTexture1df> clearTexture1df;
        engine::Pipeline<shaders::ClearTexture2df> clearTexture2df;
        engine::Pipeline<shaders::ClearTexture3df> clearTexture3df;

        engine::Pipeline<shaders::ClearTexture1df2> clearTexture1df2;
        engine::Pipeline<shaders::ClearTexture2df2> clearTexture2df2;
        engine::Pipeline<shaders::ClearTexture3df2> clearTexture3df2;

        engine::Pipeline<shaders::ClearTexture1df3> clearTexture1df3;
        engine::Pipeline<shaders::ClearTexture2df3> clearTexture2df3;
        engine::Pipeline<shaders::ClearTexture3df3> clearTexture3df3;

        engine::Pipeline<shaders::ClearTexture1df4> clearTexture1df4;
        engine::Pipeline<shaders::ClearTexture2df4> clearTexture2df4;
        engine::Pipeline<shaders::ClearTexture3df4> clearTexture3df4;

        engine::Pipeline<shaders::ClearTexture1du> clearTexture1du;
        engine::Pipeline<shaders::ClearTexture2du> clearTexture2du;
        engine::Pipeline<shaders::ClearTexture3du> clearTexture3du;

        engine::Pipeline<shaders::ClearTexture1du2> clearTexture1du2;
        engine::Pipeline<shaders::ClearTexture2du2> clearTexture2du2;
        engine::Pipeline<shaders::ClearTexture3du2> clearTexture3du2;

        engine::Pipeline<shaders::ClearTexture1du3> clearTexture1du3;
        engine::Pipeline<shaders::ClearTexture2du3> clearTexture2du3;
        engine::Pipeline<shaders::ClearTexture3du3> clearTexture3du3;

        engine::Pipeline<shaders::ClearTexture1du4> clearTexture1du4;
        engine::Pipeline<shaders::ClearTexture2du4> clearTexture2du4;
        engine::Pipeline<shaders::ClearTexture3du4> clearTexture3du4;

        engine::Pipeline<shaders::CopyTexture1df> copyTexture1df;
        engine::Pipeline<shaders::CopyTexture2df> copyTexture2df;
        engine::Pipeline<shaders::CopyTexture3df> copyTexture3df;

        engine::Pipeline<shaders::CopyTexture1df2> copyTexture1df2;
        engine::Pipeline<shaders::CopyTexture2df2> copyTexture2df2;
        engine::Pipeline<shaders::CopyTexture3df2> copyTexture3df2;

        engine::Pipeline<shaders::CopyTexture1df3> copyTexture1df3;
        engine::Pipeline<shaders::CopyTexture2df3> copyTexture2df3;
        engine::Pipeline<shaders::CopyTexture3df3> copyTexture3df3;

        engine::Pipeline<shaders::CopyTexture1df4> copyTexture1df4;
        engine::Pipeline<shaders::CopyTexture2df4> copyTexture2df4;
        engine::Pipeline<shaders::CopyTexture3df4> copyTexture3df4;

        engine::Pipeline<shaders::CopyTextureRTV2df> copyTextureRTV2df;
        engine::Pipeline<shaders::CopyTextureRTV2df2> copyTextureRTV2df2;
        engine::Pipeline<shaders::CopyTextureRTV2df3> copyTextureRTV2df3;
        engine::Pipeline<shaders::CopyTextureRTV2df4> copyTextureRTV2df4;
        engine::Pipeline<shaders::CopyTextureRTV2df34> copyTextureRTV2df34;

        engine::Pipeline<shaders::CopyBufferIndirect> copyBufferIndirect;

		engine::Pipeline<shaders::CopyDepthTexture> copyDepthTexture;

        engine::shared_ptr<BufferMath> bufferMath;
    };

    class Device
    {
    public:
        Device(engine::shared_ptr<platform::Window> window, const char* deviceName, GraphicsApi api = GraphicsApi::DX12);
		void createModelResources();
        ~Device();

		void window(engine::shared_ptr<platform::Window> window);

        int width() const;
        int height() const;

		Queue& queue(CommandListType type = CommandListType::Direct);

        CommandList createCommandList(const char* name, CommandListType type = CommandListType::Direct) const;

        engine::shared_ptr<SwapChain> createSwapChain(
            bool fullscreen = false, 
            bool vsync = true,
            SwapChain* oldSwapChain = nullptr);
        std::weak_ptr<SwapChain> currentSwapChain();
        const std::weak_ptr<SwapChain> currentSwapChain() const;

        Sampler createSampler(const SamplerDescription& desc);
        RootSignature createRootSignature() const;

        BufferOwner createBuffer(const BufferDescription& desc) const;
        BufferSRVOwner createBufferSRV(const BufferOwner& buffer, const BufferDescription& desc) const;
        BufferUAVOwner createBufferUAV(const BufferOwner& buffer, const BufferDescription& desc) const;
        BufferIBVOwner createBufferIBV(const BufferOwner& buffer, const BufferDescription& desc) const;
        BufferCBVOwner createBufferCBV(const BufferOwner& buffer, const BufferDescription& desc) const;
        BufferVBVOwner createBufferVBV(const BufferOwner& buffer, const BufferDescription& desc) const;

        BufferSRVOwner createBufferSRV(const BufferDescription& desc) const;
        BufferUAVOwner createBufferUAV(const BufferDescription& desc) const;
        BufferIBVOwner createBufferIBV(const BufferDescription& desc) const;
        BufferCBVOwner createBufferCBV(const BufferDescription& desc) const;
        BufferVBVOwner createBufferVBV(const BufferDescription& desc) const;
							   
        BufferSRVOwner createBufferSRV(const BufferOwner& buffer) const;
        BufferUAVOwner createBufferUAV(const BufferOwner& buffer) const;
        BufferIBVOwner createBufferIBV(const BufferOwner& buffer) const;
        BufferCBVOwner createBufferCBV(const BufferOwner& buffer) const;
        BufferVBVOwner createBufferVBV(const BufferOwner& buffer) const;
					
		RaytracingAccelerationStructureOwner createRaytracingAccelerationStructure(
			BufferSRV vertexBuffer,
			BufferIBV indexBuffer, 
			const BufferDescription& desc) const;

		// Resource cached buffer
        BufferSRVOwner createBufferSRV(ResourceKey key, const BufferOwner& buffer, const BufferDescription& desc) const;
        BufferSRVOwner createBufferSRV(ResourceKey key, const BufferDescription& desc) const;
        BufferSRVOwner createBufferSRV(ResourceKey key, const BufferOwner& buffer) const;
							   
        BufferIBVOwner createBufferIBV(ResourceKey key, const BufferOwner& buffer, const BufferDescription& desc) const;
        BufferIBVOwner createBufferIBV(ResourceKey key, const BufferDescription& desc) const;
        BufferIBVOwner createBufferIBV(ResourceKey key, const BufferOwner& buffer) const;

        // Bindless buffers
        BindlessBufferSRVOwner createBindlessBufferSRV();
        BindlessBufferUAVOwner createBindlessBufferUAV();
		
		TextureOwner createTexture(const TextureDescription& desc);
		
		TextureSRVOwner createTextureSRV(const TextureDescription& desc);
		TextureSRVOwner createTextureSRV(const TextureOwner& texture, const TextureDescription& desc) const;
		TextureSRVOwner createTextureSRV(const TextureOwner& texture) const;
		TextureSRVOwner createTextureSRV(const TextureDSVOwner& texture) const;
		TextureSRVOwner createTextureSRV(const TextureOwner& texture, SubResource subResources) const;
		TextureSRVOwner createTextureSRV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources) const;
		
		TextureUAVOwner createTextureUAV(const TextureDescription& desc);
		TextureUAVOwner createTextureUAV(const TextureOwner& texture, const TextureDescription& desc) const;
		TextureUAVOwner createTextureUAV(const TextureOwner& texture) const;
		TextureUAVOwner createTextureUAV(const TextureOwner& texture, SubResource subResources) const;
		TextureUAVOwner createTextureUAV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources) const;
		
		TextureDSVOwner createTextureDSV(const TextureDescription& desc, SubResource subResources = SubResource());
		TextureDSVOwner createTextureDSV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources = SubResource()) const;
		TextureDSVOwner createTextureDSV(const TextureOwner& texture, SubResource subResources = SubResource()) const;
		
		TextureRTVOwner createTextureRTV(const TextureDescription& desc, SubResource subResources = SubResource());
		TextureRTVOwner createTextureRTV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources = SubResource()) const;
		TextureRTVOwner createTextureRTV(const TextureOwner& texture, SubResource subResources = SubResource()) const;
		
		// Resource cached textures
		TextureSRVOwner createTextureSRV(ResourceKey key, const TextureDescription& desc);
		TextureSRVOwner createTextureSRV(ResourceKey key, const TextureOwner& texture, const TextureDescription& desc) const;
		TextureSRVOwner createTextureSRV(ResourceKey key, const TextureOwner& texture) const;

        CpuTexture grabTexture(TextureSRV texture);
        TextureSRVOwner loadTexture(const CpuTexture& texture);
        void copyTexture(const CpuTexture& texture, TextureSRV dst);

        // Bindless textures
        BindlessTextureSRVOwner createBindlessTextureSRV();
        BindlessTextureUAVOwner createBindlessTextureUAV();

        template<typename T>
        bool cachedDataExists(ResourceKey key) const;

        void uploadBuffer(CommandList& commandList, BufferSRV buffer, const tools::ByteRange& data, uint32_t startElement = 0);
        void uploadBuffer(CommandList& commandList, BufferCBV buffer, const tools::ByteRange& data, uint32_t startElement = 0);
        void uploadBuffer(CommandList& commandList, BufferIBV buffer, const tools::ByteRange& data, uint32_t startElement = 0);
        void uploadBuffer(CommandList& commandList, BufferVBV buffer, const tools::ByteRange& data, uint32_t startElement = 0);

        template<typename T>
        Pipeline<T> createPipeline()
        {
            return Pipeline<T>(*this, m_shaderStorage, m_api);
        }

        uint64_t frameNumber() const;
        void frameNumber(uint64_t frame);

        void processCommandLists(bool fetchQueryResults);

        engine::vector<engine::vector<QueryResultTicks>> fetchQueryResults();

        ShaderStorage& shaderStorage() { return m_shaderStorage; };

        implementation::DeviceImplIf* native() { return m_impl.get(); };
        const implementation::DeviceImplIf* native() const { return m_impl.get(); };

        TextureBufferCopyDesc getTextureBufferCopyDesc(size_t width, size_t height, Format format);
    public:

        engine::shared_ptr<image::ImageIf> createImage(
            ResourceKey key,
            const engine::string& filename,
            Format type = Format::BC7_UNORM,
            int width = -1,
            int height = -1,
            int slices = -1,
            int mips = -1,
			image::ImageType imageType = image::ImageType::DDS);

        engine::shared_ptr<image::ImageIf> createImage(
            const engine::string& filename,
            Format type = Format::BC7_UNORM,
            int width = -1,
            int height = -1,
            int slices = -1,
            int mips = -1,
			image::ImageType imageType = image::ImageType::DDS);

        engine::shared_ptr<SubMeshInstance> createMesh(
            ResourceKey key,
            const engine::string& filename,
            uint32_t meshIndex);

        Fence createFence(const char* name) const;
        Semaphore createSemaphore() const;

        void waitForIdle();
        void shutdown(bool value = true);

        //void submit(engine::vector<CommandList>& commandLists, CommandListType = CommandListType::Direct);
        void submit(CommandList& commandList, CommandListType = CommandListType::Direct);
        //void submitBlocking(engine::vector<CommandList>& commandLists, CommandListType = CommandListType::Direct);
        void submitBlocking(CommandList& commandList, CommandListType = CommandListType::Direct);
        //void submitFenced(engine::vector<CommandList>& commandLists, Fence& fence, CommandListType = CommandListType::Direct);
        //void submitFenced(CommandList& commandList, Fence& fence, CommandListType = CommandListType::Direct);

        void present(
            Semaphore& signalSemaphore,
            SwapChain& swapChain,
            unsigned int chainIndex);
        void passivePresent();

        void getNewFrame();

        NullResources& nullResouces()
        {
            return *m_nullResouces;
        }

        ResourceCache& resourceCache()
        {
            return *m_resourceCache;
        }

		ModelResources& modelResources();

        engine::shared_ptr<CorePipelines>& corePipelines()
        {
            return m_corePipelines;
        }

		ResidencyManagerV2& residencyV2()
		{
			return *m_residencyManagerV2;
		}

		FontManager& fontManager()
		{
			return *m_fontManager;
		}

        GraphicsApi api() const { return m_api; };

    private:
        GraphicsApi m_api;
        engine::shared_ptr<implementation::DeviceImplIf> m_impl;

    private:
        
        ShaderStorage m_shaderStorage;
        mutable std::recursive_mutex m_mutex;
        std::weak_ptr<SwapChain> m_swapChain;
        engine::shared_ptr<Queue> m_queue;
        engine::shared_ptr<Queue> m_copyQueue;
        engine::shared_ptr<ResourceCache> m_resourceCache;
        engine::shared_ptr<NullResources> m_nullResouces;
        engine::shared_ptr<ModelResources> m_modelResources;
        engine::shared_ptr<CorePipelines> m_corePipelines;
		engine::shared_ptr<ResidencyManagerV2> m_residencyManagerV2;
		engine::shared_ptr<FontManager> m_fontManager;
        Fence m_submitFence;
        Fence m_submitCopyQueueFence;
        Fence m_frameFence;
        uint64_t m_frameNumber;
        bool m_waitingForClose;
        bool m_destroy;
        std::unordered_map<CommandListType, engine::vector<uint32_t>> m_listCounts;
        std::unordered_map<CommandListType, uint32_t> m_listMaxes;
        engine::unordered_map<uint64_t, Sampler> m_samplerCache;

        friend class CommandList;
		friend class implementation::DeviceImplDX12;
        friend class implementation::DeviceImplVulkan;
        void returnCommandList(implementation::CommandListImplIf* cmd, CommandListType type, const char* name);
        //engine::shared_ptr<std::map<CommandListType, engine::vector<CommandList>>> m_freeCommandLists;
        //engine::shared_ptr<std::map<CommandListType, engine::vector<CommandList>>> m_inUseCommandLists;

        void clearCommandLists();
        engine::shared_ptr<engine::vector<CommandList>> m_freeCommandListsDirect;
        engine::shared_ptr<engine::vector<CommandList>> m_inUseCommandListsDirect;
        engine::shared_ptr<engine::vector<CommandList>> m_freeCommandListsCopy;
        engine::shared_ptr<engine::vector<CommandList>> m_inUseCommandListsCopy;

        struct PresentFence
        {
            uint64_t frameNumber;
            Fence presentFence;
        };
        engine::vector<PresentFence> m_presentFences;

        engine::vector<engine::vector<QueryResultTicks>> m_resolvedQuerys;

		struct ReturnedResourceBuffer { engine::shared_ptr<implementation::BufferImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceBufferSRV { engine::shared_ptr<implementation::BufferSRVImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceBufferUAV { engine::shared_ptr<implementation::BufferUAVImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceBufferIBV { engine::shared_ptr<implementation::BufferIBVImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceBufferCBV { engine::shared_ptr<implementation::BufferCBVImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceBufferVBV { engine::shared_ptr<implementation::BufferVBVImplIf> resource; FenceValue returnTime; };
        struct ReturnedResourceBindlessBufferSRV { engine::shared_ptr<implementation::BindlessBufferSRVImplIf> resource; FenceValue returnTime; };
        struct ReturnedResourceBindlessBufferUAV { engine::shared_ptr<implementation::BindlessBufferUAVImplIf> resource; FenceValue returnTime; };
		struct ReturnedRaytracingAccelerationStructure { engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf> resource; FenceValue returnTime; };

		struct ReturnedResourceTexture	  { engine::shared_ptr<implementation::TextureImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceTextureSRV { engine::shared_ptr<implementation::TextureSRVImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceTextureUAV { engine::shared_ptr<implementation::TextureUAVImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceTextureDSV { engine::shared_ptr<implementation::TextureDSVImplIf> resource; FenceValue returnTime; };
		struct ReturnedResourceTextureRTV { engine::shared_ptr<implementation::TextureRTVImplIf> resource; FenceValue returnTime; };
        struct ReturnedResourceBindlessTextureSRV { engine::shared_ptr<implementation::BindlessTextureSRVImplIf> resource; FenceValue returnTime; };
        struct ReturnedResourceBindlessTextureUAV { engine::shared_ptr<implementation::BindlessTextureUAVImplIf> resource; FenceValue returnTime; };

		mutable std::queue<ReturnedResourceBuffer> m_returnedBuffers;
		mutable std::queue<ReturnedResourceBufferSRV> m_returnedBuffersSRV;
		mutable std::queue<ReturnedResourceBufferUAV> m_returnedBuffersUAV;
		mutable std::queue<ReturnedResourceBufferIBV> m_returnedBuffersIBV;
		mutable std::queue<ReturnedResourceBufferCBV> m_returnedBuffersCBV;
		mutable std::queue<ReturnedResourceBufferVBV> m_returnedBuffersVBV;
        mutable std::queue<ReturnedResourceBindlessBufferSRV> m_returnedBindlessBuffersSRV;
        mutable std::queue<ReturnedResourceBindlessBufferUAV> m_returnedBindlessBuffersUAV;

		mutable std::queue<ReturnedRaytracingAccelerationStructure> m_returnedRaytracingAccelerationStructure;
		
		mutable std::queue<ReturnedResourceTexture> m_returnedTextures;
		mutable std::queue<ReturnedResourceTextureSRV> m_returnedTexturesSRV;
		mutable std::queue<ReturnedResourceTextureUAV> m_returnedTexturesUAV;
		mutable std::queue<ReturnedResourceTextureDSV> m_returnedTexturesDSV;
		mutable std::queue<ReturnedResourceTextureRTV> m_returnedTexturesRTV;
        mutable std::queue<ReturnedResourceBindlessTextureSRV> m_returnedBindlessTexturesSRV;
		mutable std::queue<ReturnedResourceBindlessTextureUAV> m_returnedBindlessTexturesUAV;

		friend class implementation::SwapChainImplDX12;
        friend class implementation::SwapChainImplVulkan;
		friend class RenderSetup;
		void clearReturnedResources() const;

		uint64_t getResourceReturnFrame() const;
    };
}
