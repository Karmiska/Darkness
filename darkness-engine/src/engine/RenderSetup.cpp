#include "engine/RenderSetup.h"
#include "platform/window/Window.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/Resources.h"

using namespace engine;

namespace engine
{
    RenderSetup::RenderSetup(
        engine::shared_ptr<platform::Window> window, 
        GraphicsApi api,
        EngineMode mode,
        const char* name,
        bool createModelResources, 
        MessageCallback messageCallback)
        : m_mode{ mode }
        , m_window{ window }
        , m_messageCallback{ messageCallback }
        , m_device{ m_window, name, api }
        , m_swapChain{ mode == EngineMode::OwnThread ? m_device.createSwapChain(false, false) : nullptr }
        , m_renderSemaphore{ m_device.createSemaphore(), m_device.createSemaphore() }
        , m_presentSemaphore{ m_device.createSemaphore() }
        , m_backBufferIndex{ 0 }
    {
        createSwapChainSRVs();

		if(createModelResources)
			m_device.createModelResources();
    }

    void RenderSetup::createSwapChainSRVs()
    {
        if (m_mode == EngineMode::OwnThread)
            for (int i = 0; i < BackBufferCount; ++i) { m_swapChainSRVs.emplace_back(m_device.createTextureSRV(m_swapChain->renderTargetOwner(i))); }
        else
        {
            for (int i = 0; i < BackBufferCount; ++i)
            {
                m_rtv[i] = m_device.createTextureRTV(TextureDescription()
                    .width(m_device.width())
                    .height(m_device.height())
                    .format(Format::R8G8B8A8_UNORM)
                    .usage(ResourceUsage::GpuRenderTargetReadWrite)
                    .name("Passive engine rendertarget")
                    .dimension(ResourceDimension::Texture2D)
                    .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 0.0f })
                    .shared(true));
                m_rtvsrv[i] = m_device.createTextureSRV(m_rtv[i]);
            }
        }
    }

    void RenderSetup::releaseSwapChainSRVs()
    {
        m_swapChainSRVs.clear();
    }

	void RenderSetup::recreateSwapChain()
	{
        if (m_mode == EngineMode::OwnThread)
        {
            releaseSwapChainSRVs();
            m_swapChain = nullptr;

        }

		m_device.clearReturnedResources();
		m_device.waitForIdle();

        if (m_mode == EngineMode::OwnThread)
        {
            m_swapChain = m_device.createSwapChain(false, true);
            createSwapChainSRVs();
        }
	}

    CpuTexture RenderSetup::grabDisplay()
    {
        return m_device.grabTexture(currentRTVSRV());
    }

    TextureSRVOwner RenderSetup::createGrabTexture(const CpuTexture& textureBuffer)
    {
        return m_device.loadTexture(textureBuffer);
    }

    RenderSetup::RenderSetup(GraphicsApi api, const char* name)
        : m_window{ engine::make_shared<platform::Window>("Device test window", 1024, 768) }
        , m_device{ m_window, name, api }
        , m_swapChain{ m_device.createSwapChain(false, true) }
        , m_renderSemaphore{ m_device.createSemaphore(), m_device.createSemaphore() }
        , m_presentSemaphore{ m_device.createSemaphore() }
        , m_backBufferIndex{ 0 }
    { 
        createSwapChainSRVs();
    }

    Device& RenderSetup::device()
    {
        return m_device;
    }

    SwapChain& RenderSetup::swapChain()
    {
        if (m_mode != EngineMode::OwnThread)
            ASSERT(false, "nope");
            
        return *m_swapChain;
    }

    platform::Window& RenderSetup::window()
    {
        return *m_window;
    }

	void RenderSetup::window(engine::shared_ptr<platform::Window> newWindow)
	{
		m_window = newWindow;
	}

    void RenderSetup::submit(CommandList& commandList)
    {
        CPU_MARKER(m_device.api(), "RenderSetup submit");

        m_device.submit(commandList);
    }

    void RenderSetup::submitBlocking(CommandList& commandList)
    {
        CPU_MARKER(m_device.api(), "RenderSetup submit");

        m_device.submitBlocking(commandList);
    }

    void RenderSetup::present(bool fetchQueryResults)
    {
        if (m_window->quitSignaled())
            return;

        if (m_mode == EngineMode::OwnThread)
        {

            CPU_MARKER(m_device.api(), "RenderSetup present");
            std::lock_guard<std::mutex> lock(m_mutex);
            auto backBufferIndex = currentBackBufferIndex();
            m_device.present(m_renderSemaphore[backBufferIndex], *m_swapChain, backBufferIndex);
        }
        else
            m_device.passivePresent();

        ++m_backBufferIndex;
        if (m_backBufferIndex > BackBufferCount - 1)
            m_backBufferIndex = 0;

        m_device.processCommandLists(fetchQueryResults);
        processShaderHotreload();
    }
    
    unsigned int RenderSetup::currentBackBufferIndex()
    { 
        auto res = m_swapChain->currentBackBufferIndex();
        return res;
    }

    TextureRTV RenderSetup::currentRTV()
    {
        if (m_mode == EngineMode::OwnThread)
            return m_swapChain->renderTarget(currentBackBufferIndex());
        else
            return m_rtv[m_backBufferIndex];
    }

	TextureSRV RenderSetup::currentRTVSRV()
	{
        if (m_mode == EngineMode::OwnThread)
            return m_swapChain->renderTargetSRV(currentBackBufferIndex());
        else
            return m_rtvsrv[m_backBufferIndex];
	}

    TextureSRV RenderSetup::prevRTVSRV()
    {
        auto backBufferIndex = 0;
        if (m_mode == EngineMode::OwnThread)
            backBufferIndex = currentBackBufferIndex();
        else
            backBufferIndex = m_backBufferIndex;

        --backBufferIndex;
        if (backBufferIndex < 0)
            backBufferIndex = BackBufferCount-1;

        if (m_mode == EngineMode::OwnThread)
            return m_swapChain->renderTargetSRV(backBufferIndex);
        else
            return m_rtvsrv[backBufferIndex];
    }
    
    void RenderSetup::shutdown()
    {
        // we need to clear shaderstorage as it's a static instance
        // and when the vulkan instance dies, it's going
        // to perform a debug layer check if all resources
        // have been released.
		device().clearReturnedResources();
    }

    void RenderSetup::processShaderHotreload()
    {
        CPU_MARKER(m_device.api(), "Process shader hotreload");
        if (m_device.shaderStorage().fileWatcher().hasChanges())
        {
            device().waitForIdle();
            m_lists.clear();
            if (m_device.shaderStorage().fileWatcher().processChanges())
            {
                // we had changes, pump the compilation messages to logging
                engine::vector<engine::string>& lastMessages = m_device.shaderStorage().fileWatcher().lastMessages();
                if(m_messageCallback)
                    m_messageCallback(lastMessages);
                lastMessages.clear();
            }
        }
    }
}
