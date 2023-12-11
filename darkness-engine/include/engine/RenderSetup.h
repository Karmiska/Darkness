#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/ResourcesImplIf.h"
#include "rendering/Rendering.h"

#include "containers/memory.h"
#include "containers/vector.h"
#include <mutex>

namespace platform
{
    class Window;
}

namespace engine
{
    class SwapChain;
    class TextureRTV;

    using MessageCallback = std::function<void(const engine::vector<engine::string>&)>;

    class RenderSetup
    {
    public:
        RenderSetup(
            engine::shared_ptr<platform::Window> window, 
            GraphicsApi api,
            EngineMode mode,
            const char* name,
            bool createModelResources, 
            const engine::string& preferredAdapter = "",
            MessageCallback messageCallback = [](const engine::vector<engine::string>& messages)
            {
                if (messages.size() > 0)
                {
                    for (auto&& msg : messages)
                    {
                        LOG("%s", msg.c_str());
                    }
                }
            });
        RenderSetup(
            GraphicsApi api, 
            const char* name, 
            const engine::string& preferredAdapter = "");

        Device& device();
        SwapChain& swapChain();
        platform::Window& window();
		void window(engine::shared_ptr<platform::Window> newWindow);
        void submit(CommandList& commandList);
        void submitBlocking(CommandList& commandList);
        void present(bool fetchQueryResults);
        unsigned int currentBackBufferIndex();
        TextureRTV currentRTV();
        TextureUAV currentUAV();
		TextureSRV currentRTVSRV();
        TextureSRV prevRTVSRV();

        void shutdown();

        void createSwapChainSRVs();
        void releaseSwapChainSRVs();
		void recreateSwapChain();

        CpuTexture grabDisplay();
        TextureSRVOwner createGrabTexture(const CpuTexture& textureBuffer);
    private:
        EngineMode m_mode;
        engine::shared_ptr<platform::Window> m_window;
        MessageCallback m_messageCallback;
        Device m_device;
        engine::shared_ptr<SwapChain> m_swapChain;
        Semaphore m_renderSemaphore[BackBufferCount];
        Semaphore m_presentSemaphore;
        std::mutex m_mutex;

        TextureRTVOwner m_rtv[BackBufferCount];
        TextureSRVOwner m_rtvsrv[BackBufferCount];
        int m_backBufferIndex;

        engine::vector<TextureSRVOwner> m_swapChainSRVs;

        struct CommandListExec
        {
            CommandList list;
            unsigned long long done = 0u;
        };
        engine::vector<CommandListExec> m_lists;

        void processShaderHotreload();

        BufferSRVOwner m_grabBuffer;
        engine::vector<uint8_t> m_grabVector;
        TextureSRVOwner m_grabTexture;
    };

}