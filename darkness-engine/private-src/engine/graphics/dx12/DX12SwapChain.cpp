#include "engine/graphics/Queue.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Barrier.h"

#include "engine/graphics/dx12/DX12Queue.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12Resources.h"

#include "engine/graphics/dx12/DX12SwapChain.h"

#include "platform/window/Window.h"

#include <stdlib.h>
#include "engine/graphics/dx12/DX12Headers.h"

#include "tools/Debug.h"
#include "tools/ComPtr.h"

#ifndef _DURANGO
#include "platform/window/windows/WindowsWindow.h"
#else
#include "platform/window/durango/DurangoWindow.h"
#endif

using namespace platform;
using namespace tools;

namespace engine
{
    namespace implementation
    {
        SwapChainImplDX12::SwapChainImplDX12(
            const Device& device, 
#ifndef _DURANGO
            Queue& queue, 
#else
			Queue& /*queue*/,
#endif
			
#ifndef _DURANGO
            bool fullscreen, 
#else
			bool /*fullscreen*/,
#endif
			bool vsync,
            SwapChain* /*oldSwapChain*/)
			: m_backBufferReadySemaphore{ device.createSemaphore() }
			, m_vsync{ vsync }
            , m_needRefresh{ false }
#ifdef _DURANGO
			, m_backBufferIndex{ 0u }
#endif
        {
#ifndef _DURANGO
            ComPtr<IDXGIFactory4> factory;
#else
			ComPtr<IDXGIFactory> factory;
#endif
            ComPtr<IDXGIAdapter> adapter;
            ComPtr<IDXGIOutput> adapterOutput;

			m_size.width = device.width();
			m_size.height = device.height();

#ifndef _DURANGO
			m_format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ComPtr<IDXGISwapChain1> swapChain;
            // Create a DirectX graphics interface factory.
            auto res = CreateDXGIFactory1(DARKNESS_IID_PPV_ARGS(factory.GetAddressOf()));
			ASSERT(SUCCEEDED(res));

			// Use the factory to create an adapter for the primary graphics interface (video card).
			res = factory->EnumAdapters(0, adapter.GetAddressOf());
			ASSERT(SUCCEEDED(res));

			// Enumerate the primary adapter output (monitor).
			res = adapter->EnumOutputs(0, adapterOutput.GetAddressOf());
			ASSERT(SUCCEEDED(res));

			// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
			unsigned int numModes{ 0 };
			res = adapterOutput->GetDisplayModeList(m_format, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
			ASSERT(SUCCEEDED(res));

			// Create a list to hold all the possible display modes for this monitor/video card combination.
			engine::unique_ptr<DXGI_MODE_DESC[]> displayModeList(new DXGI_MODE_DESC[numModes]);
			ASSERT(displayModeList);

			// Now fill the display mode list structures.
			res = adapterOutput->GetDisplayModeList(m_format, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.get());
			ASSERT(SUCCEEDED(res));

			// Now go through all the display modes and find the one that matches the screen height and width.
			// When a match is found store the numerator and denominator of the refresh rate for that monitor.
			unsigned int i, numerator, denominator;
			i = numerator = denominator = 0;
			for (i = 0; i<numModes; i++)
			{
				if (displayModeList[i].Height == (unsigned int)device.height())
				{
					if (displayModeList[i].Width == (unsigned int)device.width())
					{
						numerator = displayModeList[i].RefreshRate.Numerator;
						denominator = displayModeList[i].RefreshRate.Denominator;
					}
				}
			}

			// Get the adapter (video card) description.
			DXGI_ADAPTER_DESC adapterDesc;
			res = adapter->GetDesc(&adapterDesc);
			ASSERT(SUCCEEDED(res));

			// Store the dedicated video card memory in megabytes.
			m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

			// Convert the name of the video card to a character array and store it.
			size_t stringLength;
			auto wcsres = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
			ASSERT(wcsres == 0);

			// Initialize the swap chain description.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
			memset(&swapChainDesc, 0, sizeof(swapChainDesc));

			// Set the swap chain to use double buffering.
			swapChainDesc.BufferCount = BackBufferCount;

			// Set the height and width of the back buffers in the swap chain.
			
			swapChainDesc.Height = static_cast<UINT>(m_size.height);
			swapChainDesc.Width = static_cast<UINT>(m_size.width);

			// Set a regular 32-bit surface for the back buffers.
			swapChainDesc.Format = m_format;// DXGI_FORMAT_R10G10B10A2_UNORM;

																		 // Set the usage of the back buffers to be render target outputs.
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

			// Set the swap effect to discard the previous buffer contents after swapping.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			swapChainDesc.Stereo = FALSE;

			// Turn multisampling off.
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;

			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

			// Don't set the advanced flags.
			swapChainDesc.Flags = 
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | 
				DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | 
				DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
			memset(&fullScreenDesc, 0, sizeof(fullScreenDesc));
			fullScreenDesc.Windowed = !fullscreen;

			// Finally create the swap chain using the swap chain description.
			res = factory->CreateSwapChainForHwnd(
				static_cast<QueueImplDX12*>(queue.native())->native(),
				platform::implementation::WindowImplGet::impl(static_cast<const DeviceImplDX12*>(device.native())->window()).handle(),
				&swapChainDesc,
				fullscreen ? &fullScreenDesc : nullptr,
				nullptr,
				swapChain.GetAddressOf());
			ASSERT(SUCCEEDED(res));

			// Next upgrade the IDXGISwapChain to a IDXGISwapChain3 interface and store it in a private member variable named m_swapChain.
			// This will allow us to use the newer functionality such as getting the current back buffer index.
			res = swapChain->QueryInterface(DARKNESS_IID_PPV_ARGS(m_swapChain.GetAddressOf()));
			ASSERT(SUCCEEDED(res));

			res = m_swapChain->SetMaximumFrameLatency(BackBufferCount);
			ASSERT(SUCCEEDED(res));
#else
			m_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

			ComPtr<IDXGIDevice1> dxgiDevice;
			DeviceImplGet::impl(device).m_device->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(dxgiDevice.GetAddressOf()));

			// Identify the physical adapter (GPU or card) this device is running on.
			ComPtr<IDXGIAdapter> dxgiAdapter;
			dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

			// And obtain the factory object that created it.
			ComPtr<IDXGIFactory2> dxgiFactory;
			dxgiAdapter->GetParent(IID_GRAPHICS_PPV_ARGS(dxgiFactory.GetAddressOf()));

			// Create a descriptor for the swap chain.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.Width = device.width();
			swapChainDesc.Height = device.height();
			swapChainDesc.Format = m_format;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = BackBufferCount;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			swapChainDesc.Flags = DXGIX_SWAP_CHAIN_MATCH_XBOX360_AND_PC;

			IUnknown* window = reinterpret_cast<IUnknown*>(DeviceImplGet::impl(device).window().native());

			// Create a swap chain for the window.
			auto res = dxgiFactory->CreateSwapChainForCoreWindow(
				DeviceImplGet::impl(device).m_device.Get(),
				window,
				&swapChainDesc,
				nullptr,
				m_swapChain.GetAddressOf()
			);
			ASSERT(SUCCEEDED(res));
#endif
            createSwapChainTextures(device);
        }

		SwapChainType* SwapChainImplDX12::native()
		{
			return m_swapChain.Get();
		}

		const SwapChainType* SwapChainImplDX12::native() const
		{
			return m_swapChain.Get();
		}

		void SwapChainImplDX12::releaseSwapChainTextures(Device& device)
		{
			m_swapChainTextureRTVs.clear();
			m_swapChainTextureSRVs.clear();
			m_swapChainTextures.clear();

			device.clearReturnedResources();
			device.waitForIdle();
		}

        void SwapChainImplDX12::createSwapChainTextures(const Device& device)
        {
            TextureDescription chainImageDesc;
            chainImageDesc.descriptor.append = false;
            chainImageDesc.descriptor.arraySlices = 1;
            chainImageDesc.descriptor.depth = 1;
            chainImageDesc.descriptor.dimension = ResourceDimension::Texture2D;
            chainImageDesc.descriptor.format = fromDXFormat(m_format);
            chainImageDesc.descriptor.height = m_size.height;
            chainImageDesc.descriptor.width = m_size.width;
            chainImageDesc.descriptor.mipLevels = 1;
            chainImageDesc.descriptor.name = "SwapChainImage";
            chainImageDesc.descriptor.samples = 1;
            chainImageDesc.descriptor.usage = ResourceUsage::GpuRenderTargetReadWrite;

            for (int i = 0; i < BackBufferCount; ++i)
            {
                ID3D12Resource* buffer;
                auto res = m_swapChain->GetBuffer(static_cast<UINT>(i), DARKNESS_IID_PPV_ARGS(&buffer));
                ASSERT(SUCCEEDED(res));

				m_swapChainTextures.emplace_back(TextureOwner(
					static_pointer_cast<TextureImplIf>(engine::make_shared<TextureImplDX12>(
						*static_cast<const DeviceImplDX12*>(device.native()),
						chainImageDesc,
						buffer,
						ResourceState::Present)),
					[&](engine::shared_ptr<TextureImplIf> im)
					{
						//LOG("Cursious if this just works. GPU should be empty by now");
						//m_returnedTextures.push(ReturnedResourceTexture{ im, m_submitFence.currentCPUValue() });
					}));
            }

            for (auto&& tex : m_swapChainTextures)
            {
                m_swapChainTextureRTVs.emplace_back(device.createTextureRTV(tex));
				m_swapChainTextureSRVs.emplace_back(device.createTextureSRV(tex));
            }
            
        }

        void SwapChainImplDX12::resize(Device& device, Size size)
        {
            m_size = size;
			releaseSwapChainTextures(device);
            m_swapChain->ResizeBuffers(
                BackBufferCount,
                size.width,
                size.height,
				m_format,
                DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);

            createSwapChainTextures(device);
        }

        TextureRTV SwapChainImplDX12::renderTarget(int index)
        {
            return m_swapChainTextureRTVs[index].resource();
        }

		TextureSRV SwapChainImplDX12::renderTargetSRV(int index)
		{
			return m_swapChainTextureSRVs[index].resource();
		}

		TextureRTVOwner& SwapChainImplDX12::renderTargetOwner(int index)
		{
			return m_swapChainTextureRTVs[index];
		}

        size_t SwapChainImplDX12::chainLength() const
        {
            return BackBufferCount;
        }

		Semaphore& SwapChainImplDX12::backBufferReadySemaphore()
		{
			return m_backBufferReadySemaphore;
		}

        unsigned int SwapChainImplDX12::currentBackBufferIndex() const
        {
#ifndef _DURANGO
            return m_swapChain->GetCurrentBackBufferIndex();
#else
            return m_backBufferIndex;
#endif
        }

        Size SwapChainImplDX12::size() const
        {
            return m_size;
        }

        void SwapChainImplDX12::present()
        {
			try
			{
				if (m_vsync)
				{
					auto res = m_swapChain->Present(1, 0);
					if (!SUCCEEDED(res))
					{
						m_needRefresh = true;
					}
				}
				else
				{
					auto res = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
					if (!SUCCEEDED(res))
					{
						m_needRefresh = true;
					}
				}
#ifdef _DURANGO
				m_backBufferIndex = (m_backBufferIndex + 1) % BackBufferCount;
#endif
			}
			catch (...)
			{
				
			}
        }

        bool SwapChainImplDX12::needRefresh()
        {
            auto res = m_needRefresh;
            m_needRefresh = false;
            return res;
        }
    }
}
