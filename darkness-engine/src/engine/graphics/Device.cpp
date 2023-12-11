#include "engine/graphics/Device.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootSignature.h"
#include "engine/graphics/Format.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/font/FontManager.h"
#include "tools/hash/Hash.h"
#include "tools/PathTools.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12Resources.h"
#include "engine/graphics/dx12/DX12Pipeline.h"
#include "engine/graphics/dx12/DX12CommandList.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanPipeline.h"
#include "engine/graphics/vulkan/VulkanCommandList.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalDevice.h"
#include "engine/graphics/metal/MetalBuffer.h"
#include "engine/graphics/metal/MetalCommandList.h"
#endif


using namespace tools;
using namespace platform;
using namespace engine::implementation;
using namespace engine;

namespace engine
{
    Device::Device(
        engine::shared_ptr<platform::Window> window, 
        const char* deviceName, 
        GraphicsApi api, 
        const engine::string& preferredAdapter)
        : m_api{ api }
        , m_impl( (m_api == GraphicsApi::DX12) ? 
            static_pointer_cast<DeviceImplIf>(engine::shared_ptr<implementation::DeviceImplDX12>(new implementation::DeviceImplDX12(window, preferredAdapter))) :
            static_pointer_cast<DeviceImplIf>(engine::shared_ptr<implementation::DeviceImplVulkan>(new implementation::DeviceImplVulkan(window))) )
        , m_mutex{}
        , m_swapChain{}
        , m_queue{ nullptr }
        , m_resourceCache{ engine::make_shared<ResourceCache>() }
        , m_nullResouces{ engine::make_shared<NullResources>() }
        , m_corePipelines{ nullptr }
		, m_residencyManagerV2{ engine::make_shared<ResidencyManagerV2>(*this) }
		, m_fontManager{ nullptr }
        , m_submitFence{ createFence("Device submit fence") }
        , m_submitCopyQueueFence{ createFence("Device submit copy queue fence") }
        , m_frameFence{ createFence("Device frame fence") }
        // just a big number. easier to handle frame 0 - something.
		, m_frameNumber{ m_frameFence.currentCPUValue() }
        , m_waitingForClose{ false }
        , m_destroy{ false }
        , m_freeCommandListsDirect{ engine::make_shared<engine::vector<CommandList>>() }
        , m_inUseCommandListsDirect{ engine::make_shared<engine::vector<CommandList>>() }
        , m_freeCommandListsCopy{ engine::make_shared<engine::vector<CommandList>>() }
        , m_inUseCommandListsCopy{ engine::make_shared<engine::vector<CommandList>>() }
    {
        engine::string name = deviceName; name += " graphics queue";
        m_queue = engine::make_shared<Queue>(*this, CommandListType::Direct, m_api, name.c_str());
        engine::string cname = deviceName; name += " copy queue";
        m_copyQueue = engine::make_shared<Queue>(*this, CommandListType::Copy, m_api, cname.c_str());

        m_impl->createFences(*this);

        m_nullResouces->bufferSRV = createBufferSRV(BufferDescription().elements(1).format(Format::R32_UINT).usage(ResourceUsage::GpuReadWrite).name("Null BufferSRV"));
        m_nullResouces->bufferUAV = createBufferUAV(BufferDescription().elements(1).format(Format::R32_UINT).usage(ResourceUsage::GpuReadWrite).name("Null BufferUAV"));
		m_nullResouces->bufferStructuredSRV = createBufferSRV(BufferDescription().elements(1).elementSize(4).structured(true).usage(ResourceUsage::GpuReadWrite).name("Null BufferStructuredSRV"));
		m_nullResouces->bufferStructuredUAV = createBufferUAV(BufferDescription().elements(1).elementSize(4).structured(true).usage(ResourceUsage::GpuReadWrite).name("Null BufferStructuredUAV"));
        m_nullResouces->textureSRV = createTextureSRV(TextureDescription().width(1).height(1).format(Format::R8G8B8A8_UNORM).usage(ResourceUsage::GpuReadWrite).name("Null TextureSRV"));
        m_nullResouces->textureCubeSRV = createTextureSRV(TextureDescription()
            .width(1).height(1)
            .format(Format::R8G8B8A8_UNORM)
            .arraySlices(6)
            .usage(ResourceUsage::GpuReadWrite)
            .dimension(ResourceDimension::TextureCube)
            .name("Null TextureCubeSRV"));
        m_nullResouces->textureUAV = createTextureUAV(m_nullResouces->textureSRV);
        m_nullResouces->sampler = createSampler(SamplerDescription());
        m_impl->nullResources(m_nullResouces);

        m_corePipelines = engine::make_shared<CorePipelines>(CorePipelines{
            createPipeline<shaders::RemoveElement>(),

            createPipeline<shaders::ClearTexture1df>(),
            createPipeline<shaders::ClearTexture2df>(),
            createPipeline<shaders::ClearTexture3df>(),

            createPipeline<shaders::ClearTexture1df2>(),
            createPipeline<shaders::ClearTexture2df2>(),
            createPipeline<shaders::ClearTexture3df2>(),

            createPipeline<shaders::ClearTexture1df3>(),
            createPipeline<shaders::ClearTexture2df3>(),
            createPipeline<shaders::ClearTexture3df3>(),

            createPipeline<shaders::ClearTexture1df4>(),
            createPipeline<shaders::ClearTexture2df4>(),
            createPipeline<shaders::ClearTexture3df4>(),

            createPipeline<shaders::ClearTexture1du>(),
            createPipeline<shaders::ClearTexture2du>(),
            createPipeline<shaders::ClearTexture3du>(),

            createPipeline<shaders::ClearTexture1du2>(),
            createPipeline<shaders::ClearTexture2du2>(),
            createPipeline<shaders::ClearTexture3du2>(),

            createPipeline<shaders::ClearTexture1du3>(),
            createPipeline<shaders::ClearTexture2du3>(),
            createPipeline<shaders::ClearTexture3du3>(),

            createPipeline<shaders::ClearTexture1du4>(),
            createPipeline<shaders::ClearTexture2du4>(),
            createPipeline<shaders::ClearTexture3du4>(),

            createPipeline<shaders::CopyTexture1df>(),
            createPipeline<shaders::CopyTexture2df>(),
            createPipeline<shaders::CopyTexture3df>(),

            createPipeline<shaders::CopyTexture1df2>(),
            createPipeline<shaders::CopyTexture2df2>(),
            createPipeline<shaders::CopyTexture3df2>(),

            createPipeline<shaders::CopyTexture1df3>(),
            createPipeline<shaders::CopyTexture2df3>(),
            createPipeline<shaders::CopyTexture3df3>(),

            createPipeline<shaders::CopyTexture1df4>(),
            createPipeline<shaders::CopyTexture2df4>(),
            createPipeline<shaders::CopyTexture3df4>(),

            createPipeline<shaders::CopyTextureRTV2df>(),
            createPipeline<shaders::CopyTextureRTV2df2>(),
            createPipeline<shaders::CopyTextureRTV2df3>(),
            createPipeline<shaders::CopyTextureRTV2df4>(),
            createPipeline<shaders::CopyTextureRTV2df34>(),

            createPipeline<shaders::CopyBufferIndirect>(),

			createPipeline<shaders::CopyDepthTexture>(),

            engine::make_shared<BufferMath>(*this)
        });

        m_corePipelines->copyTextureRTV2df.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_corePipelines->copyTextureRTV2df.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_corePipelines->copyTextureRTV2df.setDepthStencilState(DepthStencilDescription().depthEnable(false));

        m_corePipelines->copyTextureRTV2df2.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_corePipelines->copyTextureRTV2df2.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_corePipelines->copyTextureRTV2df2.setDepthStencilState(DepthStencilDescription().depthEnable(false));

        m_corePipelines->copyTextureRTV2df3.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_corePipelines->copyTextureRTV2df3.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_corePipelines->copyTextureRTV2df3.setDepthStencilState(DepthStencilDescription().depthEnable(false));

        m_corePipelines->copyTextureRTV2df4.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_corePipelines->copyTextureRTV2df4.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_corePipelines->copyTextureRTV2df4.setDepthStencilState(DepthStencilDescription().depthEnable(false));

        m_corePipelines->copyTextureRTV2df34.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_corePipelines->copyTextureRTV2df34.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_corePipelines->copyTextureRTV2df34.setDepthStencilState(DepthStencilDescription().depthEnable(false));

		

        m_fontManager = engine::make_shared<FontManager>(*this);
    }

	void Device::window(engine::shared_ptr<platform::Window> window)
	{
		m_impl->window(window);
	}

	void Device::createModelResources()
	{
		m_modelResources = engine::make_shared<ModelResources>(*this);
	}

	ModelResources& Device::modelResources()
	{
		return *m_modelResources;
	}

	Queue& Device::queue(CommandListType type)
	{
		if (type == CommandListType::Direct)
			return *m_queue;
		return *m_copyQueue;
	}

    Device::~Device()
    {
        waitForIdle();
        m_samplerCache.clear();
        m_residencyManagerV2 = nullptr;
        m_fontManager = nullptr;
        m_corePipelines = nullptr;
        m_modelResources = nullptr;
        m_nullResouces = nullptr;
        m_impl->nullResources(m_nullResouces);
        m_shaderStorage.clear();

        clearCommandLists();
        clearReturnedResources();
    }

    void Device::processCommandLists(bool doFetchQueryResults)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto currentSubmitFenceValue = m_submitFence.currentGPUValue();
        auto currentSubmitCopyQueueFenceValue = m_submitCopyQueueFence.currentGPUValue();

        m_listCounts[CommandListType::Direct].emplace_back(static_cast<uint32_t>(m_inUseCommandListsDirect->size()));
        if (m_listCounts[CommandListType::Direct].size() > 5)
            m_listCounts[CommandListType::Direct].erase(m_listCounts[CommandListType::Direct].begin());

        m_listCounts[CommandListType::Copy].emplace_back(static_cast<uint32_t>(m_inUseCommandListsCopy->size()));
        if (m_listCounts[CommandListType::Copy].size() > 5)
            m_listCounts[CommandListType::Copy].erase(m_listCounts[CommandListType::Copy].begin());


        for (auto&& listtype : m_listCounts)
        {
            uint32_t maxCount = 0;
            for (auto&& count : listtype.second)
            {
                maxCount = count > maxCount ? count : maxCount;
            }
            m_listMaxes[listtype.first] = maxCount;
        }

        auto fetchQueryResults = [&](CommandListType type, engine::vector<CommandList>& list, engine::FenceValue submitFenceValue, bool doFetchQueryResults)
        {
            int MinFreeCount = static_cast<int>(m_listMaxes[type]);
            int CurrentFreeCount = 0;

            for (auto cmd = list.begin(); cmd != list.end();)
            {
                CommandList& cmdList = *cmd;
                if (cmdList.submitFenceValue() <= submitFenceValue)
                {
                    if (doFetchQueryResults)
                    {
                        uint64_t freq = 1;
                        if (cmdList.commandListType() == CommandListType::Direct)
                            freq = m_queue->timeStampFrequency();
                        else if (cmdList.commandListType() == CommandListType::Copy)
                            freq = m_copyQueue->timeStampFrequency();

                        auto queryResults = cmdList.fetchQueryResults(freq);
                        if (queryResults.size() > 0)
                        {
                            m_resolvedQuerys.emplace_back(std::move(queryResults));
                        }
                    }

                    if (CurrentFreeCount >= MinFreeCount)
                        m_destroy = true;
                    cmd = list.erase(cmd);
                    if (CurrentFreeCount >= MinFreeCount)
                        m_destroy = false;

                    ++CurrentFreeCount;
                }
                else
                    ++cmd;
            }
        };
        
        fetchQueryResults(CommandListType::Direct, *m_inUseCommandListsDirect, currentSubmitFenceValue, doFetchQueryResults);
        fetchQueryResults(CommandListType::Copy, *m_inUseCommandListsCopy, currentSubmitCopyQueueFenceValue, doFetchQueryResults);

        auto frameFenceValue = m_frameFence.currentGPUValue();
        while (m_returnedBuffersSRV.size() > 0) { if (m_returnedBuffersSRV.front().returnTime < frameFenceValue) { m_returnedBuffersSRV.pop(); } else break; }
        while (m_returnedBuffersUAV.size() > 0) { if (m_returnedBuffersUAV.front().returnTime < frameFenceValue) { m_returnedBuffersUAV.pop(); } else break; }
        while (m_returnedBuffersIBV.size() > 0) { if (m_returnedBuffersIBV.front().returnTime < frameFenceValue) { m_returnedBuffersIBV.pop(); } else break; }
        while (m_returnedBuffersCBV.size() > 0) { if (m_returnedBuffersCBV.front().returnTime < frameFenceValue) { m_returnedBuffersCBV.pop(); } else break; }
        while (m_returnedBuffersVBV.size() > 0) { if (m_returnedBuffersVBV.front().returnTime < frameFenceValue) { m_returnedBuffersVBV.pop(); } else break; }
        while (m_returnedBuffers.size() > 0) { if (m_returnedBuffers.front().returnTime < frameFenceValue) { m_returnedBuffers.pop(); } else break; }

        while (m_returnedTexturesSRV.size() > 0) { if (m_returnedTexturesSRV.front().returnTime < frameFenceValue) { m_returnedTexturesSRV.pop(); } else break; }
        while (m_returnedTexturesUAV.size() > 0) { if (m_returnedTexturesUAV.front().returnTime < frameFenceValue) { m_returnedTexturesUAV.pop(); } else break; }
        while (m_returnedTexturesDSV.size() > 0) { if (m_returnedTexturesDSV.front().returnTime < frameFenceValue) { m_returnedTexturesDSV.pop(); } else break; }
        while (m_returnedTexturesRTV.size() > 0) { if (m_returnedTexturesRTV.front().returnTime < frameFenceValue) { m_returnedTexturesRTV.pop(); } else break; }
        while (m_returnedTextures.size() > 0) { if (m_returnedTextures.front().returnTime < frameFenceValue) { m_returnedTextures.pop(); } else break; }

        m_impl->processUploads(currentSubmitFenceValue, false);
    }

    engine::vector<engine::vector<QueryResultTicks>> Device::fetchQueryResults()
    {
        engine::vector<engine::vector<QueryResultTicks>> result = std::move(m_resolvedQuerys);
        return result;
    }

    TextureBufferCopyDesc Device::getTextureBufferCopyDesc(size_t width, size_t height, Format format)
    {
        return m_impl->getTextureBufferCopyDesc(width, height, format);
    }

    /*void Device::submit(engine::vector<CommandList>& commandLists, CommandListType type)
    {
        auto& lists = *m_inUseCommandLists;
        for (auto&& commandList : commandLists)
        {
            lists[type].emplace_back(std::move(commandList));
            //commandList = createCommandList(lists[type].back().name(), type);
            m_submitFence.increaseCPUValue();
            lists[type].back().submitFenceValue(m_submitFence.currentCPUValue());
        }

        if (type == CommandListType::Direct)
            m_queue->submit(lists[type].back(), m_submitFence);
        else if (type == CommandListType::Copy)
            m_copyQueue->submit(lists[type].back(), m_submitFence);
    }*/

    void Device::submit(CommandList& commandList, CommandListType type)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        engine::vector<CommandList>* listPtr = nullptr;
        if (type == CommandListType::Direct)
            listPtr = m_inUseCommandListsDirect.get();
        else if (type == CommandListType::Copy)
            listPtr = m_inUseCommandListsCopy.get();

        //auto& lists = *m_inUseCommandLists;

        listPtr->emplace_back(std::move(commandList));
        //commandList = createCommandList(listPtr->back().name(), type);

        

        if (type == CommandListType::Direct)
        {
            m_submitFence.increaseCPUValue();

            m_impl->setCurrentFenceValue(CommandListType::Direct, m_submitFence.currentCPUValue());
            listPtr->back().submitFenceValue(m_submitFence.currentCPUValue());

            //LOG("Submitting commandlist: %llu", m_submitFence.currentCPUValue());
            m_queue->submit(listPtr->back(), m_submitFence);
        }
        else if (type == CommandListType::Copy)
        {
            m_submitCopyQueueFence.increaseCPUValue();

            m_impl->setCurrentFenceValue(CommandListType::Copy, m_submitCopyQueueFence.currentCPUValue());
            listPtr->back().submitFenceValue(m_submitCopyQueueFence.currentCPUValue());

            //LOG("Submitting copy commandlist: %llu", m_submitFence.currentCPUValue());
            m_copyQueue->submit(listPtr->back(), m_submitCopyQueueFence);
        }
    }

    /*void Device::submitBlocking(engine::vector<CommandList>& commandLists, CommandListType type)
    {
        if (type == CommandListType::Direct)
        {
            m_submitFence.increaseCPUValue();
            m_queue->submit(commandLists, m_submitFence);
        }
        else if (type == CommandListType::Copy)
        {
            m_submitFence.increaseCPUValue();
            m_copyQueue->submit(commandLists, m_submitFence);
        }
        m_submitFence.blockUntilSignaled();
    }*/

    void Device::submitBlocking(CommandList& commandList, CommandListType type)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        engine::vector<CommandList>* listPtr = nullptr;
        if (type == CommandListType::Direct)
            listPtr = m_inUseCommandListsDirect.get();
        else if (type == CommandListType::Copy)
            listPtr = m_inUseCommandListsCopy.get();

        //auto& lists = *m_inUseCommandLists;

        listPtr->emplace_back(std::move(commandList));
        //commandList = createCommandList(listPtr->back().name(), type);



        if (type == CommandListType::Direct)
        {
            m_submitFence.increaseCPUValue();

            m_impl->setCurrentFenceValue(CommandListType::Direct, m_submitFence.currentCPUValue());
            listPtr->back().submitFenceValue(m_submitFence.currentCPUValue());

            //LOG("Submitting commandlist: %llu", m_submitFence.currentCPUValue());
            m_queue->submit(listPtr->back(), m_submitFence);
            m_submitFence.blockUntilSignaled();
    }
        else if (type == CommandListType::Copy)
        {
            m_submitCopyQueueFence.increaseCPUValue();

            m_impl->setCurrentFenceValue(CommandListType::Copy, m_submitCopyQueueFence.currentCPUValue());
            listPtr->back().submitFenceValue(m_submitCopyQueueFence.currentCPUValue());

            //LOG("Submitting copy commandlist: %llu", m_submitFence.currentCPUValue());
            m_copyQueue->submit(listPtr->back(), m_submitCopyQueueFence);
            m_submitCopyQueueFence.blockUntilSignaled();
        }
    
#if 0
        if (type == CommandListType::Direct)
        {
            m_submitFence.increaseCPUValue();

            //LOG("Submitting commandlist: %llu", m_submitFence.currentCPUValue());

            m_queue->submit(commandList, m_submitFence);
            m_submitFence.blockUntilSignaled();
        }
        else if (type == CommandListType::Copy)
        {
            m_submitCopyQueueFence.increaseCPUValue();

            //LOG("Submitting copy commandlist: %llu", m_submitFence.currentCPUValue());

            m_copyQueue->submit(commandList, m_submitCopyQueueFence);
            m_submitCopyQueueFence.blockUntilSignaled();
        }
#endif   
    }

    /*void Device::submitFenced(engine::vector<CommandList>& commandLists, Fence& fence, CommandListType type)
    {
        if (type == CommandListType::Direct)
            m_queue->submit(commandLists, fence);
        else if (type == CommandListType::Copy)
            m_copyQueue->submit(commandLists, fence);
    }

    void Device::submitFenced(CommandList& commandList, Fence& fence, CommandListType type)
    {
        if (type == CommandListType::Direct)
            m_queue->submit(commandList, fence);
        else if (type == CommandListType::Copy)
            m_copyQueue->submit(commandList, fence);
    }*/

    void Device::present(
        Semaphore& signalSemaphore,
        SwapChain& swapChain,
        unsigned int chainIndex)
    {
        //m_queue->signal(signalSemaphore);
        m_queue->present(signalSemaphore, swapChain, chainIndex);

        ++m_frameNumber;
        m_queue->signal(m_frameFence, m_frameNumber);

        getNewFrame();
    }

    void Device::passivePresent()
    {
        ++m_frameNumber;
        m_queue->signal(m_frameFence, m_frameNumber);
    }

    void Device::getNewFrame()
    {
        // if we have less than 2 CPU generated frames
        // that have not finished, we can generate one more

        // how many generated frames that haven't finished do we have?
        auto gpuFrameNumber = m_frameFence.currentGPUValue();
        auto cpuFrameNumber = m_frameNumber;
        ASSERT(gpuFrameNumber <= cpuFrameNumber, "GPU is further than CPU, should never happen");
        auto framesRunning = cpuFrameNumber - gpuFrameNumber;

        if (framesRunning > 1)
            m_frameFence.blockUntilSignaled(cpuFrameNumber - 1);
    }

    void Device::shutdown(bool value)
    {
        m_destroy = value;
    }

    void Device::returnCommandList(implementation::CommandListImplIf* cmd, CommandListType type, const char* name)
    {
        if (m_destroy)
        {
            delete cmd;
        }
        else if (!m_waitingForClose)
        {
            //auto& lists = *m_freeCommandLists;
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            bool found = false;
            engine::vector<CommandList>* typeListPtr = nullptr;
            if (type == CommandListType::Direct)
                typeListPtr = m_freeCommandListsDirect.get();
            else if (type == CommandListType::Copy)
                typeListPtr = m_freeCommandListsCopy.get();

            engine::vector<CommandList>& typeList = *typeListPtr;

            for (int i = 0; i < typeList.size(); ++i)
            {
                if (typeList[i].m_impl.get() == cmd)
                {
                    found = true;
                    break;
                }
            }
			if (!found)
			{
				typeList.emplace_back(CommandList(cmd, *this, name, type, m_api));
				typeList.back().clear();
			}
			else
			{
				ASSERT(false, "List found already in the free list. That's impossible!");
			}
        }
        else if (m_waitingForClose)
        {
            delete cmd;
        }
    }

    CommandList Device::createCommandList(const char* name, CommandListType type) const
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        engine::vector<CommandList>* typeListPtr = nullptr;
        if (type == CommandListType::Direct)
            typeListPtr = m_freeCommandListsDirect.get();
        else if (type == CommandListType::Copy)
            typeListPtr = m_freeCommandListsCopy.get();

        auto& lists = *typeListPtr;
        if (!lists.empty())
        {
            auto cmd = std::move(lists.front());
            lists.erase(lists.begin());
            cmd.name(name);
            cmd.makeSureHasCorePipelines(m_corePipelines);
            return cmd;
        }
        return { *this, name, type, m_api };
    }


	engine::shared_ptr<SwapChain> Device::createSwapChain(
        bool fullscreen, 
        bool vsync,
        SwapChain* oldSwapChain)
    {
		engine::shared_ptr<SwapChain> result(new SwapChain(*this, *m_queue, m_api, fullscreen, vsync, oldSwapChain));
        m_swapChain = result;
        return result;
    }

	std::weak_ptr<SwapChain> Device::currentSwapChain()
    {
        return m_swapChain;
    }

    const std::weak_ptr<SwapChain> Device::currentSwapChain() const
    {
        return m_swapChain;
    }

    Sampler Device::createSampler(const SamplerDescription& desc)
    {
		auto samplerDescHash = desc.hash();
		auto found = m_samplerCache.find(samplerDescHash);
		if (found != m_samplerCache.end())
			return found->second;

		auto sampler = Sampler(*this, desc, m_api);
        m_samplerCache[samplerDescHash] = sampler;

        return sampler;
    }

    RootSignature Device::createRootSignature() const
    {
        return RootSignature(*this, m_api);
    }

	uint64_t Device::getResourceReturnFrame() const
	{
        return m_frameNumber;// m_submitFence.currentCPUValue();
	}

	BufferOwner Device::createBuffer(const BufferDescription& desc) const
    {
		BufferOwner buffer(
            (m_api == GraphicsApi::DX12) ? 
                static_pointer_cast<BufferImplIf>(engine::make_shared<BufferImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), desc)) : 
                static_pointer_cast<BufferImplIf>(engine::make_shared<BufferImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), desc)),
			[&](engine::shared_ptr<BufferImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffers.push(ReturnedResourceBuffer{ im, getResourceReturnFrame() });
			});

        if (desc.initialData.elements > 0)
        {
            auto cmd = createCommandList(desc.descriptor.name);
            m_impl->uploadRawBuffer(cmd.native(), buffer.resource(), desc.initialData.data, 0);
            const_cast<Device*>(this)->submitBlocking(cmd);
        }

        return buffer;
    }

	BufferSRVOwner Device::createBufferSRV(const BufferOwner& buffer, const BufferDescription& desc) const
    {
        return BufferSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferSRVImplIf>(engine::make_shared<BufferSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, desc)) :
                static_pointer_cast<BufferSRVImplIf>(engine::make_shared<BufferSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, desc)),
			[&](engine::shared_ptr<BufferSRVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersSRV.push(ReturnedResourceBufferSRV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferUAVOwner Device::createBufferUAV(const BufferOwner& buffer, const BufferDescription& desc) const
    {
        BufferDescription copyDesc = desc;
        if (copyDesc.descriptor.usage == ResourceUsage::GpuRead)
            copyDesc.descriptor.usage = ResourceUsage::GpuReadWrite;

        return BufferUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferUAVImplIf>(engine::make_shared<BufferUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, copyDesc)) :
                static_pointer_cast<BufferUAVImplIf>(engine::make_shared<BufferUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, copyDesc)),
			[&](engine::shared_ptr<BufferUAVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersUAV.push(ReturnedResourceBufferUAV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
            buffer.m_returner);
    }

	BufferIBVOwner Device::createBufferIBV(const BufferOwner& buffer, const BufferDescription& desc) const
    {
        auto temp = desc;
        temp.descriptor.indexBuffer = true;
		return BufferIBVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferIBVImplIf>(engine::make_shared<BufferIBVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, temp)) :
                static_pointer_cast<BufferIBVImplIf>(engine::make_shared<BufferIBVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, temp)),
			[&](engine::shared_ptr<BufferIBVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersIBV.push(ReturnedResourceBufferIBV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferCBVOwner Device::createBufferCBV(const BufferOwner& buffer, const BufferDescription& desc) const
    {
		return BufferCBVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferCBVImplIf>(engine::make_shared<BufferCBVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, desc)) :
                static_pointer_cast<BufferCBVImplIf>(engine::make_shared<BufferCBVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, desc)),
			[&](engine::shared_ptr<BufferCBVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersCBV.push(ReturnedResourceBufferCBV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferVBVOwner Device::createBufferVBV(const BufferOwner& buffer, const BufferDescription& desc) const
    {
        auto temp = desc;
        temp.descriptor.vertexBuffer = true;
		return BufferVBVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferVBVImplIf>(engine::make_shared<BufferVBVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, temp)) :
                static_pointer_cast<BufferVBVImplIf>(engine::make_shared<BufferVBVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, temp)),
			[&](engine::shared_ptr<BufferVBVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersVBV.push(ReturnedResourceBufferVBV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferSRVOwner Device::createBufferSRV(const BufferDescription& desc) const
    {
		auto buffer = createBuffer(desc);
		auto temp =  BufferSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferSRVImplIf>(engine::make_shared<BufferSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, desc)) :
                static_pointer_cast<BufferSRVImplIf>(engine::make_shared<BufferSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, desc)),
			[&](engine::shared_ptr<BufferSRVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersSRV.push(ReturnedResourceBufferSRV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
        return temp;
    }

	BufferUAVOwner Device::createBufferUAV(const BufferDescription& desc) const
    {
        BufferDescription copyDesc = desc;
        if (copyDesc.descriptor.usage == ResourceUsage::GpuRead)
            copyDesc.descriptor.usage = ResourceUsage::GpuReadWrite;

		auto buffer = createBuffer(copyDesc);

		return BufferUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferUAVImplIf>(engine::make_shared<BufferUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, copyDesc)) :
                static_pointer_cast<BufferUAVImplIf>(engine::make_shared<BufferUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, copyDesc)),
			[&](engine::shared_ptr<BufferUAVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersUAV.push(ReturnedResourceBufferUAV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferIBVOwner Device::createBufferIBV(const BufferDescription& desc) const
    {
        auto temp = desc;
        temp.descriptor.indexBuffer = true;
		auto buffer = createBuffer(temp);

		return BufferIBVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferIBVImplIf>(engine::make_shared<BufferIBVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, temp)) :
                static_pointer_cast<BufferIBVImplIf>(engine::make_shared<BufferIBVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, temp)),
			[&](engine::shared_ptr<BufferIBVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersIBV.push(ReturnedResourceBufferIBV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferCBVOwner Device::createBufferCBV(const BufferDescription& desc) const
    {
		auto buffer = createBuffer(desc);

		return BufferCBVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferCBVImplIf>(engine::make_shared<BufferCBVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, desc)) :
                static_pointer_cast<BufferCBVImplIf>(engine::make_shared<BufferCBVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, desc)),
			[&](engine::shared_ptr<BufferCBVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersCBV.push(ReturnedResourceBufferCBV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferVBVOwner Device::createBufferVBV(const BufferDescription& desc) const
    {
        auto temp = desc;
        temp.descriptor.vertexBuffer = true;
		auto buffer = createBuffer(temp);

		return BufferVBVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BufferVBVImplIf>(engine::make_shared<BufferVBVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), buffer, temp)) :
                static_pointer_cast<BufferVBVImplIf>(engine::make_shared<BufferVBVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), buffer, temp)),
			[&](engine::shared_ptr<BufferVBVImplIf> im)
			{
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
				m_returnedBuffersVBV.push(ReturnedResourceBufferVBV{ im, getResourceReturnFrame() });
			},
			buffer.m_implementation,
			buffer.m_returner);
    }

	BufferSRVOwner Device::createBufferSRV(const BufferOwner& buffer) const
    {
        BufferDescription temp = { buffer.resource().description() };
        if(temp.descriptor.usage == ResourceUsage::GpuReadWrite)
            temp.descriptor.usage = ResourceUsage::GpuRead;
        return createBufferSRV(buffer, temp);
    }

	BufferUAVOwner Device::createBufferUAV(const BufferOwner& buffer) const
    {
        BufferDescription copyDesc = { buffer.resource().description() };
        if (copyDesc.descriptor.usage == ResourceUsage::GpuRead)
            copyDesc.descriptor.usage = ResourceUsage::GpuReadWrite;

        return createBufferUAV(buffer, copyDesc);
    }

	BufferIBVOwner Device::createBufferIBV(const BufferOwner& buffer) const
    {
        return createBufferIBV(buffer, BufferDescription());
    }

	BufferCBVOwner Device::createBufferCBV(const BufferOwner& buffer) const
    {
        return createBufferCBV(buffer, BufferDescription());
    }

	BufferVBVOwner Device::createBufferVBV(const BufferOwner& buffer) const
    {
        return createBufferVBV(buffer, BufferDescription());
    }

	RaytracingAccelerationStructureOwner Device::createRaytracingAccelerationStructure(
		BufferSRV vertexBuffer,
		BufferIBV indexBuffer, 
		const BufferDescription& desc) const
	{
		return RaytracingAccelerationStructureOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<RaytracingAccelerationStructureImplIf>(engine::make_shared<RaytracingAccelerationStructureImplDX12>(*this, vertexBuffer, indexBuffer, desc)) :
                static_pointer_cast<RaytracingAccelerationStructureImplIf>(engine::make_shared<RaytracingAccelerationStructureImplVulkan>(*this, vertexBuffer, indexBuffer, desc)),
			[&](engine::shared_ptr<RaytracingAccelerationStructureImplIf> im)
		{
			m_returnedRaytracingAccelerationStructure.push(ReturnedRaytracingAccelerationStructure{ im, getResourceReturnFrame() });
		});
	}

	BufferSRVOwner Device::createBufferSRV(ResourceKey key, const BufferOwner& buffer, const BufferDescription& desc) const
    {
        return m_resourceCache->createBufferSRV(key, [this, buffer, desc]()->BufferSRVOwner
        {
            return this->createBufferSRV( buffer, desc );
        });
    }

	BufferSRVOwner Device::createBufferSRV(ResourceKey key, const BufferDescription& desc) const
    {
        return m_resourceCache->createBufferSRV(key, [this, desc]()->BufferSRVOwner
        {
            return this->createBufferSRV(createBuffer(desc), desc);
        });
    }

	BufferSRVOwner Device::createBufferSRV(ResourceKey key, const BufferOwner& buffer) const
    {
        return m_resourceCache->createBufferSRV(key, [this, buffer]()->BufferSRVOwner
        {
            return createBufferSRV(buffer, BufferDescription());
        });
    }

	BufferIBVOwner Device::createBufferIBV(ResourceKey key, const BufferOwner& buffer, const BufferDescription& desc) const
    {
        return m_resourceCache->createBufferIBV(key, [this, buffer, desc]()->BufferIBVOwner
        {
            return this->createBufferIBV(buffer, desc);
        });
    }

	BufferIBVOwner Device::createBufferIBV(ResourceKey key, const BufferDescription& desc) const
    {
        return m_resourceCache->createBufferIBV(key, [this, desc]()->BufferIBVOwner
        {
            return this->createBufferIBV(createBuffer(desc), desc);
        });
    }

	BufferIBVOwner Device::createBufferIBV(ResourceKey key, const BufferOwner& buffer) const
    {
        return m_resourceCache->createBufferIBV(key, [this, buffer]()->BufferIBVOwner
        {
            return createBufferIBV(buffer, BufferDescription());
        });
    }

    BindlessBufferSRVOwner Device::createBindlessBufferSRV()
    {
        return BindlessBufferSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BindlessBufferSRVImplIf>(engine::make_shared<BindlessBufferSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()))) :
                static_pointer_cast<BindlessBufferSRVImplIf>(engine::make_shared<BindlessBufferSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()))),
            [&](engine::shared_ptr<BindlessBufferSRVImplIf> im)
            {
                m_returnedBindlessBuffersSRV.push(ReturnedResourceBindlessBufferSRV{ im, getResourceReturnFrame() });
            });
    }

    BindlessBufferUAVOwner Device::createBindlessBufferUAV()
    {
        return BindlessBufferUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BindlessBufferUAVImplIf>(engine::make_shared<BindlessBufferUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()))) :
                static_pointer_cast<BindlessBufferUAVImplIf>(engine::make_shared<BindlessBufferUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()))),
            [&](engine::shared_ptr<BindlessBufferUAVImplIf> im)
            {
                m_returnedBindlessBuffersUAV.push(ReturnedResourceBindlessBufferUAV{ im, getResourceReturnFrame() });
            });
    }

    template<>
    bool Device::cachedDataExists<TextureSRV>(ResourceKey key) const
    {
        return m_resourceCache->cachedDataExists<TextureSRV>(key);
    }

    template<>
    bool Device::cachedDataExists<BufferSRV>(ResourceKey key) const
    {
        return m_resourceCache->cachedDataExists<BufferSRV>(key);
    }

    template<>
    bool Device::cachedDataExists<BufferIBV>(ResourceKey key) const
    {
        return m_resourceCache->cachedDataExists<BufferIBV>(key);
    }

    template<>
    bool Device::cachedDataExists<image::ImageIf>(ResourceKey key) const
    {
        return m_resourceCache->cachedDataExists<image::ImageIf>(key);
    }

    template<>
    bool Device::cachedDataExists<Mesh>(ResourceKey key) const
    {
        return m_resourceCache->cachedDataExists<Mesh>(key);
    }

	TextureOwner Device::createTexture(const TextureDescription& desc)
    {
		TextureOwner texture(
			m_impl->createTexture(*this, *m_queue, desc),
			[&](engine::shared_ptr<TextureImplIf> im)
			{
				m_returnedTextures.push(ReturnedResourceTexture{ im, getResourceReturnFrame() });
			});

        return texture;
    }

	TextureSRVOwner Device::createTextureSRV(const TextureDescription& desc)
    {
		auto texture = createTexture(desc);

		return TextureSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc)) :
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc)),
			[&](engine::shared_ptr<TextureSRVImplIf> im) {
			m_returnedTexturesSRV.push(ReturnedResourceTextureSRV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
            texture.m_returner);
    }

	TextureUAVOwner Device::createTextureUAV(const TextureDescription& desc)
    {
        auto descCopy = desc;
        if(desc.descriptor.usage != ResourceUsage::GpuRenderTargetReadWrite)
            descCopy.usage(ResourceUsage::GpuReadWrite);
		auto texture = createTexture(descCopy);

		return TextureUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, descCopy)) :
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, descCopy)),
			[&](engine::shared_ptr<TextureUAVImplIf> im) {
			m_returnedTexturesUAV.push(ReturnedResourceTextureUAV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureDSVOwner Device::createTextureDSV(const TextureDescription& desc, SubResource subResources)
    {
		auto texture = createTexture(desc);

		return TextureDSVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureDSVImplIf>(engine::make_shared<TextureDSVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc, subResources)) :
                static_pointer_cast<TextureDSVImplIf>(engine::make_shared<TextureDSVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc, subResources)),
			[&](engine::shared_ptr<TextureDSVImplIf> im) {
			m_returnedTexturesDSV.push(ReturnedResourceTextureDSV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureRTVOwner Device::createTextureRTV(const TextureDescription& desc, SubResource subResources)
    {
		auto texture = createTexture(desc);

		return TextureRTVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureRTVImplIf>(engine::make_shared<TextureRTVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc, subResources)) :
                static_pointer_cast<TextureRTVImplIf>(engine::make_shared<TextureRTVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc, subResources)),
			[&](engine::shared_ptr<TextureRTVImplIf> im) {
			m_returnedTexturesRTV.push(ReturnedResourceTextureRTV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureSRVOwner Device::createTextureSRV(const TextureOwner& texture, const TextureDescription& desc) const
    {
		return TextureSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc)) :
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc)),
			[&](engine::shared_ptr<TextureSRVImplIf> im) {
			m_returnedTexturesSRV.push(ReturnedResourceTextureSRV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureUAVOwner Device::createTextureUAV(const TextureOwner& texture, const TextureDescription& desc) const
    {
		return TextureUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc)) :
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc)),
			[&](engine::shared_ptr<TextureUAVImplIf> im) {
			m_returnedTexturesUAV.push(ReturnedResourceTextureUAV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureDSVOwner Device::createTextureDSV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources) const
    {
		return TextureDSVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureDSVImplIf>(engine::make_shared<TextureDSVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc, subResources)) :
                static_pointer_cast<TextureDSVImplIf>(engine::make_shared<TextureDSVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc, subResources)),
			[&](engine::shared_ptr<TextureDSVImplIf> im) {
			m_returnedTexturesDSV.push(ReturnedResourceTextureDSV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureRTVOwner Device::createTextureRTV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources) const
    {
		return TextureRTVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureRTVImplIf>(engine::make_shared<TextureRTVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc, subResources)) :
                static_pointer_cast<TextureRTVImplIf>(engine::make_shared<TextureRTVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc, subResources)),
			[&](engine::shared_ptr<TextureRTVImplIf> im) {
			m_returnedTexturesRTV.push(ReturnedResourceTextureRTV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureSRVOwner Device::createTextureSRV(const TextureOwner& texture) const
    {
		return TextureSRVOwner(
            (m_api == GraphicsApi::DX12) ?
            static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, TextureDescription{ texture.resource().description() })) :
            static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, TextureDescription{ texture.resource().description() })),
			[&](engine::shared_ptr<TextureSRVImplIf> im) {
			m_returnedTexturesSRV.push(ReturnedResourceTextureSRV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureUAVOwner Device::createTextureUAV(const TextureOwner& texture) const
    {
		return TextureUAVOwner(
            (m_api == GraphicsApi::DX12) ?
            static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, TextureDescription{ texture.resource().description() })) :
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, TextureDescription{ texture.resource().description() })),
			[&](engine::shared_ptr<TextureUAVImplIf> im) {
			m_returnedTexturesUAV.push(ReturnedResourceTextureUAV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureDSVOwner Device::createTextureDSV(const TextureOwner& texture, SubResource subResources) const
    {
		return TextureDSVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureDSVImplIf>(engine::make_shared<TextureDSVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)) :
                static_pointer_cast<TextureDSVImplIf>(engine::make_shared<TextureDSVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)),
			[&](engine::shared_ptr<TextureDSVImplIf> im) {
			m_returnedTexturesDSV.push(ReturnedResourceTextureDSV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureRTVOwner Device::createTextureRTV(const TextureOwner& texture, SubResource subResources) const
    {
		return TextureRTVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureRTVImplIf>(engine::make_shared<TextureRTVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)) :
                static_pointer_cast<TextureRTVImplIf>(engine::make_shared<TextureRTVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)),
			[&](engine::shared_ptr<TextureRTVImplIf> im) {
			m_returnedTexturesRTV.push(ReturnedResourceTextureRTV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureSRVOwner Device::createTextureSRV(const TextureOwner& texture, SubResource subResources) const
    {
		return TextureSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)) :
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)),
			[&](engine::shared_ptr<TextureSRVImplIf> im) {
			m_returnedTexturesSRV.push(ReturnedResourceTextureSRV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureSRVOwner Device::createTextureSRV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources) const
    {
		return TextureSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc, subResources)) :
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc, subResources)),
			[&](engine::shared_ptr<TextureSRVImplIf> im) {
			m_returnedTexturesSRV.push(ReturnedResourceTextureSRV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureUAVOwner Device::createTextureUAV(const TextureOwner& texture, SubResource subResources) const
    {
		return TextureUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)) :
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, TextureDescription{ texture.resource().description() }, subResources)),
			[&](engine::shared_ptr<TextureUAVImplIf> im) {
			m_returnedTexturesUAV.push(ReturnedResourceTextureUAV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureUAVOwner Device::createTextureUAV(const TextureOwner& texture, const TextureDescription& desc, SubResource subResources) const
    {
		return TextureUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), texture, desc, subResources)) :
                static_pointer_cast<TextureUAVImplIf>(engine::make_shared<TextureUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), texture, desc, subResources)),
			[&](engine::shared_ptr<TextureUAVImplIf> im) {
			m_returnedTexturesUAV.push(ReturnedResourceTextureUAV{ im, getResourceReturnFrame() }); },
			texture.m_implementation,
			texture.m_returner);
    }

	TextureSRVOwner Device::createTextureSRV(ResourceKey key, const TextureDescription& desc)
    {
        return m_resourceCache->createTextureSRV(key, [this, desc]()->TextureSRVOwner
        {
            return this->createTextureSRV(desc);
        });
    }

	TextureSRVOwner Device::createTextureSRV(ResourceKey key, const TextureOwner& texture, const TextureDescription& desc) const
    {
        return m_resourceCache->createTextureSRV(key, [this, texture, desc]()->TextureSRVOwner
        {
			return this->createTextureSRV(texture, desc);
        });
    }

	TextureSRVOwner Device::createTextureSRV(ResourceKey key, const TextureOwner& texture) const
    {
        return m_resourceCache->createTextureSRV(key, [this, texture]()->TextureSRVOwner
        {
            return this->createTextureSRV(texture, { texture.resource().description() });
        });
    }

    CpuTexture Device::grabTexture(TextureSRV texture)
    {
        return m_impl->grabTexture(*this, texture);
    }

    TextureSRVOwner Device::loadTexture(const CpuTexture& texture)
    {
        return m_impl->loadTexture(*this, texture);
    }

    void Device::copyTexture(const CpuTexture& texture, TextureSRV dst)
    {
        m_impl->copyTexture(*this, texture, dst);
    }

	engine::shared_ptr<image::ImageIf> Device::createImage(
        ResourceKey key,
        const string& filename,
        Format type,
        int width,
        int height,
        int slices,
        int mips,
		image::ImageType imageType)
    {
        return m_resourceCache->createImage(key, pathClean(filename), type, width, height, slices, mips, imageType);
    }

    engine::shared_ptr<image::ImageIf> Device::createImage(
        const string& filename,
        Format type,
        int width,
        int height,
        int slices,
        int mips,
		image::ImageType imageType)
    {
        auto cleanPath = pathClean(filename);
        auto key = tools::hash(cleanPath);
        return m_resourceCache->createImage(key, cleanPath, type, width, height, slices, mips, imageType);
    }

	engine::shared_ptr<SubMeshInstance> Device::createMesh(
        ResourceKey key,
        const string& filename,
        uint32_t meshIndex)
    {
        return m_resourceCache->createMesh(*this, key, pathClean(filename), meshIndex);
    }

	TextureSRVOwner Device::createTextureSRV(const TextureDSVOwner& texture) const
    {
        TextureDescription desc = { texture.resource().texture().description() };
        if(desc.descriptor.format == Format::D16_UNORM)
            desc.format(Format::R16_FLOAT);
        else if (desc.descriptor.format == Format::D32_FLOAT)
            desc.format(Format::R32_FLOAT);
        else if (desc.descriptor.format == Format::D24_UNORM_S8_UINT)
            desc.format(Format::R32_FLOAT);
        else if (desc.descriptor.format == Format::D32_FLOAT_S8X24_UINT)
            desc.format(Format::R32_FLOAT);

		TextureOwner textureOwner(
            texture.m_textureImplementation, 
            [&](engine::shared_ptr<TextureImplIf> im) 
            {
                m_returnedTextures.push(ReturnedResourceTexture{ im, getResourceReturnFrame() });
            });

		return TextureSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()), textureOwner, desc)) :
                static_pointer_cast<TextureSRVImplIf>(engine::make_shared<TextureSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()), textureOwner, desc)),
			[&](engine::shared_ptr<TextureSRVImplIf> im) {
			m_returnedTexturesSRV.push(ReturnedResourceTextureSRV{ im, getResourceReturnFrame() }); },
			textureOwner.m_implementation,
			textureOwner.m_returner);
    }

    BindlessTextureSRVOwner Device::createBindlessTextureSRV()
    {
        return BindlessTextureSRVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BindlessTextureSRVImplIf>(engine::make_shared<BindlessTextureSRVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()))) :
                static_pointer_cast<BindlessTextureSRVImplIf>(engine::make_shared<BindlessTextureSRVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()))),
            [&](engine::shared_ptr<BindlessTextureSRVImplIf> im)
            {
                m_returnedBindlessTexturesSRV.push(ReturnedResourceBindlessTextureSRV{ im, getResourceReturnFrame() });
            });
    }

    BindlessTextureUAVOwner Device::createBindlessTextureUAV()
    {
        return BindlessTextureUAVOwner(
            (m_api == GraphicsApi::DX12) ?
                static_pointer_cast<BindlessTextureUAVImplIf>(engine::make_shared<BindlessTextureUAVImplDX12>(*static_cast<const implementation::DeviceImplDX12*>(native()))) :
                static_pointer_cast<BindlessTextureUAVImplIf>(engine::make_shared<BindlessTextureUAVImplVulkan>(*static_cast<const implementation::DeviceImplVulkan*>(native()))),
            [&](engine::shared_ptr<BindlessTextureUAVImplIf> im)
            {
                m_returnedBindlessTexturesUAV.push(ReturnedResourceBindlessTextureUAV{ im, getResourceReturnFrame() });
            });
    }

    uint64_t Device::frameNumber() const
    {
        return m_frameNumber;
    }

    void Device::frameNumber(uint64_t frame)
    {
        m_frameNumber = frame;
    }

    void Device::uploadBuffer(CommandList& commandList, BufferSRV buffer, const ByteRange& data, uint32_t startElement)
    {
        m_impl->uploadBuffer(commandList, buffer, data, startElement);
    }

    void Device::uploadBuffer(CommandList& commandList, BufferCBV buffer, const tools::ByteRange& data, uint32_t startElement)
    {
        m_impl->uploadBuffer(commandList, buffer, data, startElement);
    }

    void Device::uploadBuffer(CommandList& commandList, BufferIBV buffer, const tools::ByteRange& data, uint32_t startElement)
    {
        m_impl->uploadBuffer(commandList, buffer, data, startElement);
    }

    void Device::uploadBuffer(CommandList& commandList, BufferVBV buffer, const tools::ByteRange& data, uint32_t startElement)
    {
        m_impl->uploadBuffer(commandList, buffer, data, startElement);
    }

    int Device::width() const
    {
        return m_impl->width();
    }

    int Device::height() const
    {
        return m_impl->height();
    }

    Fence Device::createFence(const char* name) const
    {
        return Fence(*this, name, m_api);
    }

    Semaphore Device::createSemaphore() const
    {
        return Semaphore(*this, m_api);
    }

    void Device::waitForIdle()
    {
        //LOG("Starting to idle device");
        m_queue->waitForIdle();
        m_copyQueue->waitForIdle();
        m_impl->waitForIdle();

        clearCommandLists();
        clearReturnedResources();

        /*clearReturnedResources();
        clearCommandLists();

        for(int i = 0; i < BackBufferCount; ++i)
        {
            auto cmd = createCommandList("wtf");
            submitBlocking(cmd);
            m_queue->waitForIdle();
            m_copyQueue->waitForIdle();
            m_impl->waitForIdle();
            processCommandLists();
            m_resolvedQuerys.clear();
            ++m_frameNumber;
        }

        auto origDestroy = m_destroy;
        m_destroy = true;
        m_waitingForClose = true;
        m_queue->waitForIdle();
        m_frameNumber += BackBufferCount;
        processCommandLists();

        m_impl->waitForIdle();
        //m_inUseCommandLists->clear();
        m_inUseCommandListsDirect->clear();
        m_inUseCommandListsCopy->clear();

        for (auto& cmdList : *m_freeCommandListsDirect) { cmdList.end(); }
        for (auto& cmdList : *m_freeCommandListsCopy) { cmdList.end(); }

        m_freeCommandListsDirect->clear();
        m_freeCommandListsCopy->clear();
        
		clearReturnedResources();

        m_waitingForClose = false;
        // TODO: maybe wait for all the command buffers?
        m_destroy = origDestroy;*/
        
		//LOG("Successfully idled the device");
    }

    void Device::clearCommandLists()
    {
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
		    // if m_destroy is not true when clearing the command lists,
		    // they're not getting deleted but leaked.
		    // they'll try to return to device, but device finds them in the free list
		    // and doesn't delete them.
		    m_destroy = true;
            m_freeCommandListsDirect->clear();
            m_inUseCommandListsDirect->clear();
            m_freeCommandListsCopy->clear();
            m_inUseCommandListsCopy->clear();
		    m_destroy = false;
            for (auto& cmdList : *m_freeCommandListsDirect) { cmdList.end(); }
            for (auto& cmdList : *m_freeCommandListsCopy) { cmdList.end(); }
        }
    }

	void Device::clearReturnedResources() const
	{
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
		while (m_returnedBuffersSRV.size() > 0) { m_returnedBuffersSRV.pop(); }
		while (m_returnedBuffersUAV.size() > 0) { m_returnedBuffersUAV.pop(); }
		while (m_returnedBuffersIBV.size() > 0) { m_returnedBuffersIBV.pop(); }
		while (m_returnedBuffersCBV.size() > 0) { m_returnedBuffersCBV.pop(); }
		while (m_returnedBuffersVBV.size() > 0) { m_returnedBuffersVBV.pop(); }
        while (m_returnedBuffers.size() > 0) { m_returnedBuffers.pop(); }

		while (m_returnedTexturesSRV.size() > 0) { m_returnedTexturesSRV.pop(); }
		while (m_returnedTexturesUAV.size() > 0) { m_returnedTexturesUAV.pop(); }
		while (m_returnedTexturesDSV.size() > 0) { m_returnedTexturesDSV.pop(); }
		while (m_returnedTexturesRTV.size() > 0) { m_returnedTexturesRTV.pop(); }
        while (m_returnedTextures.size() > 0) { m_returnedTextures.pop(); }
	}
}
