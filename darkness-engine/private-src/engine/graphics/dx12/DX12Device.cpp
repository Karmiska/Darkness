#include "engine/graphics/dx12/DX12AfterMath.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Resources.h"
#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/dx12/DX12Debug.h"
#include "engine/graphics/dx12/DX12DescriptorHeap.h"
#include "engine/graphics/dx12/DX12CommandAllocator.h"
#include "engine/graphics/CommandListImplIf.h"

#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/CommandList.h"
#include "tools/Debug.h"

#include "shaders/core/shared_types/ClusterExecuteIndirect.hlsli"

#include "platform/window/Window.h"

using namespace tools;

#undef PERFORMANCE_MEASURING_MODE
namespace engine
{
    namespace implementation
    {
		bool IsDirectXRaytracingSupported(IDXGIAdapter1* /*adapter*/)
		{
			/*ComPtr<ID3D12Device> testDevice;
			D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

			return SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, DARKNESS_IID_PPV_ARGS(&testDevice)))
				&& SUCCEEDED(testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData)))
				&& featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;*/
			return false;
		};

        DeviceImplDX12::DeviceImplDX12(engine::shared_ptr<platform::Window> window)
            : m_mutex{}
            , m_device{}
			, m_graphicsQueueUploadFence{ nullptr }
            , m_copyQueueUploadFence{ nullptr }
			, m_deviceGraphicsQueue{ nullptr }
            , m_deviceCopyQueue{ nullptr }
            , m_window{ window }
            , m_currentHeap{ 0 }
			, m_currentFenceValue{ 0 }
            , m_currentCopyFenceValue{ 0 }
        {
#ifndef _DURANGO
			D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_12_1 };

            IDXGIFactory* pFactory;
            HRESULT hr = CreateDXGIFactory(DARKNESS_IID_PPV_ARGS(&pFactory));
            ASSERT(hr == S_OK, "Could not create DXGI Factory");

            UINT i = 0;
            IDXGIAdapter* pAdapter;
            engine::vector <IDXGIAdapter*> vAdapters;

			auto preferredAdapterName = L"GeForce";
			//auto preferredAdapterName = L"Radeon";
			int preferredAdapterIndex = 0;

            while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
            {
                vAdapters.push_back(pAdapter);

                DXGI_ADAPTER_DESC desc;
                pAdapter->GetDesc(&desc);

				std::wstring name = desc.Description;
				if (name.find(preferredAdapterName) != std::wstring::npos)
					preferredAdapterIndex = i;

                ++i;
            }
#endif

#ifndef _DURANGO
            auto createResult = D3D12CreateDevice(
                vAdapters[preferredAdapterIndex],
                featureLevel,
                DARKNESS_IID_PPV_ARGS(m_device.GetAddressOf()));
            ASSERT(SUCCEEDED(createResult));

#ifdef AFTERMATH_ENABLED
            auto afterMathResult = GFSDK_Aftermath_DX12_Initialize(
                GFSDK_Aftermath_Version::GFSDK_Aftermath_Version_API,
                GFSDK_Aftermath_FeatureFlags::GFSDK_Aftermath_FeatureFlags_Maximum, 
                m_device.Get());
            ASSERT(afterMathResult == GFSDK_Aftermath_Result_Success, "Aftermath failed to initialize");
#endif

#else
			// Create the DX12 API device object.
			D3D12XBOX_CREATE_DEVICE_PARAMETERS params = {};
			params.Version = D3D12_SDK_VERSION;

#if defined(_DEBUG)
			// Enable the debug layer.
			params.ProcessDebugFlags = D3D12_PROCESS_DEBUG_FLAG_DEBUG_LAYER_ENABLED;
#elif defined(PROFILE)
			// Enable the instrumented driver.
			params.ProcessDebugFlags = D3D12XBOX_PROCESS_DEBUG_FLAG_INSTRUMENTED;
#endif

			params.ProcessDebugFlags |= D3D12XBOX_PROCESS_DEBUG_FLAG_ENABLE_COMMON_STATE_PROMOTION;

			params.GraphicsCommandQueueRingSizeBytes = static_cast<UINT>(D3D12XBOX_DEFAULT_SIZE_BYTES);
			params.GraphicsScratchMemorySizeBytes = static_cast<UINT>(D3D12XBOX_DEFAULT_SIZE_BYTES);
			params.ComputeScratchMemorySizeBytes = static_cast<UINT>(D3D12XBOX_DEFAULT_SIZE_BYTES);

			auto createResult = D3D12XboxCreateDevice(
				nullptr,
				&params,
				DARKNESS_IID_PPV_ARGS(m_device.GetAddressOf())
			);
			ASSERT(SUCCEEDED(createResult));

#endif

#if defined(PERFORMANCE_MEASURING_MODE) && !defined(_DURANGO)
            m_device->SetStablePowerState(true);
#endif

#if 1
            GraphicsDebug::addDevice(m_device.Get());

            m_descriptorHeaps.cbv_srv_uav = engine::make_shared<DescriptorHeapImplDX12>(
                this,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                50000);

            m_descriptorHeaps.sampler = engine::make_shared<DescriptorHeapImplDX12>(
                this,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                1000);

            m_descriptorHeaps.rtv = engine::make_shared<DescriptorHeapImplDX12>(
                this,
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                20000);

            m_descriptorHeaps.dsv = engine::make_shared<DescriptorHeapImplDX12>(
                this,
                D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                1000);

            m_descriptorHeaps.shaderVisible_cbv_srv_uav = engine::make_shared<DescriptorHeapImplDX12>(
                this,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                40000,
                20000);

            m_descriptorHeaps.shaderVisible_sampler = engine::make_shared<DescriptorHeapImplDX12>(
                this,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                2048);
            
			m_uploadBuffer = BufferOwner(
                static_pointer_cast<BufferImplIf>(engine::make_shared<BufferImplDX12>(
					*this,
					BufferDescription()
					.usage(ResourceUsage::Upload)
					.elementSize(1)
					.elements(UploadBufferSizeBytes)
					.name("UploadBuffer"))),
				[&](engine::shared_ptr<BufferImplIf> im)
				{
					//LOG("Cursious if this just works. GPU should be empty by now");
					//m_returnedBuffers.push(ReturnedResourceBuffer{ im, m_submitFence.currentCPUValue() });
				});

			Buffer temp(m_uploadBuffer.resource());
            auto ptr = temp.m_impl->map(this);
#endif

#ifdef UPLOADBUFFER_MEMORYALLOCATOR_ALIGNEDCHUNKS
            m_allocator = MemoryAllocator(ByteRange{ static_cast<uint8_t*>(ptr), static_cast<uint8_t*>(ptr) + UploadBufferSizeBytes }, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
#endif

#ifdef UPLOADBUFFER_MEMORYALLOCATOR_RINGBUFFER
            m_allocator = RingBuffer(
                ByteRange{ 
                    static_cast<uint8_t*>(ptr), 
                    static_cast<uint8_t*>(ptr) + UploadBufferSizeBytes });
#endif

            // create indirect signatures
            {
                // create drawIndirect signature
                D3D12_INDIRECT_ARGUMENT_DESC indirectArgumentDesc;
                indirectArgumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
                D3D12_COMMAND_SIGNATURE_DESC signatureDesc;
                signatureDesc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
                signatureDesc.NodeMask = 0;
                signatureDesc.NumArgumentDescs = 1;
                signatureDesc.pArgumentDescs = &indirectArgumentDesc;
                m_device->CreateCommandSignature(&signatureDesc, nullptr, DARKNESS_IID_PPV_ARGS(m_drawSignature.GetAddressOf()));
            }
            {
                // create drawIndexedIndirect signature
                D3D12_INDIRECT_ARGUMENT_DESC indirectArgumentDesc;
                indirectArgumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
                D3D12_COMMAND_SIGNATURE_DESC signatureDesc;
                signatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
                signatureDesc.NodeMask = 0;
                signatureDesc.NumArgumentDescs = 1;
                signatureDesc.pArgumentDescs = &indirectArgumentDesc;
                m_device->CreateCommandSignature(&signatureDesc, nullptr, DARKNESS_IID_PPV_ARGS(m_drawIndexedSignature.GetAddressOf()));
            }
            {
                // create drawIndexedIndirect signature
                D3D12_INDIRECT_ARGUMENT_DESC indirectArgumentDesc;
                indirectArgumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
                D3D12_COMMAND_SIGNATURE_DESC signatureDesc;
                signatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
                signatureDesc.NodeMask = 0;
                signatureDesc.NumArgumentDescs = 1;
                signatureDesc.pArgumentDescs = &indirectArgumentDesc;
                m_device->CreateCommandSignature(&signatureDesc, nullptr, DARKNESS_IID_PPV_ARGS(m_drawIndexedInstancedSignature.GetAddressOf()));
            }
            {
                // create dispatchIndirect signature
                D3D12_INDIRECT_ARGUMENT_DESC indirectArgumentDesc;
                indirectArgumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
                D3D12_COMMAND_SIGNATURE_DESC signatureDesc;
                signatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
                signatureDesc.NodeMask = 0;
                signatureDesc.NumArgumentDescs = 1;
                signatureDesc.pArgumentDescs = &indirectArgumentDesc;
                m_device->CreateCommandSignature(&signatureDesc, nullptr, DARKNESS_IID_PPV_ARGS(m_dispatchSignature.GetAddressOf()));
            }

#ifdef DXR_BUILD
			auto dxrResult = m_device->QueryInterface(DARKNESS_IID_PPV_ARGS(m_dxrDevice.GetAddressOf()));
			ASSERT(SUCCEEDED(dxrResult), "Could not create DXR device");
#endif
        }

		void DeviceImplDX12::createFences(Device& device)
		{
            m_graphicsQueueUploadFence = engine::make_shared<Fence>(device.createFence("Graphics Upload fence"));
            m_copyQueueUploadFence = engine::make_shared<Fence>(device.createFence("Copy Upload fence"));
            m_deviceGraphicsQueue = &device.queue(CommandListType::Direct);
            m_deviceCopyQueue = &device.queue(CommandListType::Copy);

            m_currentFenceValue = m_graphicsQueueUploadFence->currentCPUValue();
            m_currentCopyFenceValue = m_copyQueueUploadFence->currentCPUValue();
		}

        DeviceImplDX12::~DeviceImplDX12()
        {
            m_graphicsQueueUploadFence = nullptr;
            m_copyQueueUploadFence = nullptr;
            m_uploadBuffer = BufferSRVOwner();

            for (auto&& p : m_executeIndirectClusterSignature)
            {
                p.second->Release();
                p.second = nullptr;
            }
            m_executeIndirectClusterSignature.clear();
        }

        ID3D12CommandSignature* DeviceImplDX12::drawIndirectSignature()
        {
            return m_drawSignature.Get();
        }

        ID3D12CommandSignature* DeviceImplDX12::drawIndexedIndirectSignature()
        {
            return m_drawIndexedSignature.Get();
        }

        ID3D12CommandSignature* DeviceImplDX12::drawIndexedInstancedIndirectSignature()
        {
            return m_drawIndexedInstancedSignature.Get();
        }

        ID3D12CommandSignature* DeviceImplDX12::dispatchIndirectSignature()
        {
            return m_dispatchSignature.Get();
        }

        ID3D12Device* DeviceImplDX12::device() const
        {
            return m_device.Get();
        }

#ifdef DXR_BUILD
		ID3D12Device5* DeviceImplDX12::dxrDevice() const
		{
			return m_dxrDevice.Get();
		}
#endif

        ID3D12CommandSignature* DeviceImplDX12::executeIndirectClusterSignature(ID3D12RootSignature* rootSignature)
        {
            auto sig = m_executeIndirectClusterSignature.find(rootSignature);
            if (sig != m_executeIndirectClusterSignature.end())
                return sig->second;

            ID3D12CommandSignature* res;
            // create execute indirect cluster signature
            D3D12_INDIRECT_ARGUMENT_DESC indirectArgumentDesc[2];
            indirectArgumentDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
            indirectArgumentDesc[0].Constant.RootParameterIndex = 0;
            indirectArgumentDesc[0].Constant.DestOffsetIn32BitValues = 0;
            indirectArgumentDesc[0].Constant.Num32BitValuesToSet = 1;
            indirectArgumentDesc[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
            D3D12_COMMAND_SIGNATURE_DESC signatureDesc;
			signatureDesc.ByteStride = sizeof(ClusterExecuteIndirectArgs);// sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) + 12;
            signatureDesc.NodeMask = 0;
            signatureDesc.NumArgumentDescs = 2;
            signatureDesc.pArgumentDescs = &indirectArgumentDesc[0];
            m_device->CreateCommandSignature(&signatureDesc, rootSignature, DARKNESS_IID_PPV_ARGS(&res));
            m_executeIndirectClusterSignature[rootSignature] = res;

            return m_executeIndirectClusterSignature[rootSignature];
        }

        engine::unique_ptr<GpuMarkerContainer> DeviceImplDX12::getMarkerContainer()
        {
            return m_gpuMarkerStorage.getNewContainer();
        }

        void DeviceImplDX12::returnMarkerContainer(engine::unique_ptr<GpuMarkerContainer>&& container)
        {
            m_gpuMarkerStorage.returnContainer(std::move(container));
        }

        engine::shared_ptr<CommandAllocatorImplIf> DeviceImplDX12::createCommandAllocator(CommandListType type, const char* name)
        {
            /*std::queue<engine::shared_ptr<CommandAllocatorImpl>>& allocatorQueue = m_commandAllocators[static_cast<int>(type)];
            engine::vector<engine::shared_ptr<CommandAllocatorImpl>>& inUseAllocatorQueue = m_inUseCommandAllocators[static_cast<int>(type)];

            if (allocatorQueue.size() > 0)
            {
                auto res = allocatorQueue.front();
                allocatorQueue.pop();
                inUseAllocatorQueue.emplace_back(res);
                return res;
            }

            inUseAllocatorQueue.emplace_back(engine::make_shared<CommandAllocatorImpl>(*this, type));
            return inUseAllocatorQueue.back();*/

            return engine::make_shared<CommandAllocatorImplDX12>(*this, type, name);
        }

        void DeviceImplDX12::freeCommandAllocator(engine::shared_ptr<CommandAllocatorImplIf> allocator)
        {
            /*engine::vector<engine::shared_ptr<CommandAllocatorImpl>>& inUseAllocatorQueue = m_inUseCommandAllocators[static_cast<int>(allocator->type())];
            engine::vector<engine::shared_ptr<CommandAllocatorImpl>>& returnedAllocators = m_returnedCommandAllocators[static_cast<int>(allocator->type())];
            inUseAllocatorQueue.erase(std::find(inUseAllocatorQueue.begin(), inUseAllocatorQueue.end(), allocator));
            returnedAllocators.emplace_back(allocator);*/
        }

        const platform::Window& DeviceImplDX12::window() const
        {
            return *m_window;
        }

		void DeviceImplDX12::window(engine::shared_ptr<platform::Window> window)
		{
			m_window = window;
		}

		//static int debugFrameCounterForMemory = 0;

        int DeviceImplDX12::width() const
        {
			/*++debugFrameCounterForMemory;

			if ((debugFrameCounterForMemory % 120) == 0)
			{
				debugPrintCurrentStatus();
			}*/


#ifndef _DURANGO
            return m_window->width();
#else
			return 1920;
#endif
        }

        int DeviceImplDX12::height() const
        {
#ifndef _DURANGO
            return m_window->height();
#else
			return 1080;
#endif
        }

        SIZE_T Align(SIZE_T uLocation, SIZE_T uAlign)
        {
            if ((0 == uAlign) || (uAlign & (uAlign - 1)))
            {
                ASSERT(false, "Could not allocate upload buffer");
            }

            return ((uLocation + (uAlign - 1)) & ~(uAlign - 1));
        }

        void DeviceImplDX12::nullResources(engine::shared_ptr<NullResources> nullResources)
        {
            m_nullResources = nullResources;
        }

        NullResources& DeviceImplDX12::nullResources()
        {
            return *m_nullResources;
        }

		void DeviceImplDX12::processUploads(engine::FenceValue value, bool force)
		{
            std::lock_guard<std::mutex> lock(m_mutex);
			if (force)
			{
				for (auto&& upload : m_uploadAllocations)
				{
					m_allocator.free(upload.ptr);
				}
				m_uploadAllocations.clear();
				return;
			}
			auto upload = m_uploadAllocations.begin();
			while(upload != m_uploadAllocations.end())
			{
				if ((*upload).value <= value)
				{
					m_allocator.free((*upload).ptr);
					upload = m_uploadAllocations.erase(upload);
				}
				else
					++upload;
			}
		}

        TextureBufferCopyDesc DeviceImplDX12::getTextureBufferCopyDesc(size_t width, size_t height, Format format)
        {
            TextureBufferCopyDesc result;
            result.elementSize = engine::formatBytes(format);
            result.elements = roundUpToMultiple(width, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * height;
            result.bufferSize = result.elementSize * result.elements;
            result.width = width;
            result.height = height;
            result.pitch = roundUpToMultiple(width, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
            result.pitchBytes = result.pitch * formatBytes(format);
            result.format = format;
            result.zeroUp = true;
            return result;
        }

        CpuTexture DeviceImplDX12::grabTexture(Device& device, TextureSRV texture)
        {
            auto elementSize = engine::formatBytes(texture.format());
            auto elements = roundUpToMultiple(texture.width(), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * texture.height();
            auto bufferSize = elements * elementSize;

            if (!m_grabBuffer.resource().valid() ||
                (m_grabBuffer.resource().desc().elements * m_grabBuffer.resource().desc().elementSize) != bufferSize)
            {
                m_grabBuffer = device.createBufferSRV(engine::BufferDescription()
                    .format(texture.format())
                    .elementSize(elementSize)
                    .elements(elements)
                    .name("Frame capture")
                    .usage(engine::ResourceUsage::GpuToCpu)
                );
            }

            CommandList texcmdb = device.createCommandList("DeviceImplDX12::createTexture");
            texcmdb.copyTexture(texture, m_grabBuffer.resource());
            device.submitBlocking(texcmdb);
            

            CpuTexture result;
            result.width = texture.width();
            result.height = texture.height();
            result.pitch = roundUpToMultiple(texture.width(), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
            result.pitchBytes = result.pitch * static_cast<int>(formatBytes(texture.format()));
            result.format = texture.format();
            result.zeroUp = true;
            result.data = engine::make_shared<vector<uint8_t>>();

            auto ptr = m_grabBuffer.resource().buffer().map(device);
            result.data->resize(bufferSize);
            memcpy(result.data->data(), ptr, bufferSize);
            m_grabBuffer.resource().buffer().unmap(device);

            return result;
        }

        void transitionCommon(CommandList& cmd, Texture tex, ResourceState state)
        {
            size_t sliceCount = tex.arraySlices();
            size_t mipCount = tex.mipLevels();

            for (size_t slice = 0; slice < sliceCount; ++slice)
            {
                for (size_t mip = 0; mip < mipCount; ++mip)
                {
                    if (tex.state(static_cast<int>(slice), static_cast<int>(mip)) != ResourceState::Common)
                        cmd.transition(tex, state, SubResource{ static_cast<uint32_t>(mip), 1, static_cast<uint32_t>(slice), 1 });
                    tex.state(static_cast<int>(slice), static_cast<int>(mip), state);
                }
            }
        }

        TextureSRVOwner DeviceImplDX12::loadTexture(Device& device, const CpuTexture& texture)
        {
            auto tex = texture.tightPack();
            if(texture.zeroUp)
                return device.createTextureSRV(TextureDescription()
                    .name("Device loadTexture srv")
                    .width(tex.width)
                    .height(tex.height)
                    .format(tex.format)
                    .arraySlices(1ull)
                    .mipLevels(1)
                    .setInitialData(TextureDescription::InitialData(
                        *tex.data,
                        tex.pitch,
                        tex.pitch * tex.height)));
            else
            {
                auto flipped = tex.flipYAxis();
                return device.createTextureSRV(TextureDescription()
                    .name("Device loadTexture srv")
                    .width(flipped.width)
                    .height(flipped.height)
                    .format(flipped.format)
                    .arraySlices(1ull)
                    .mipLevels(1)
                    .setInitialData(TextureDescription::InitialData(
                        *flipped.data,
                        flipped.pitch,
                        flipped.pitch * flipped.height)));
            }
        }

        void DeviceImplDX12::copyTexture(Device& device, const CpuTexture& tex, TextureSRV dsttex)
        {
            auto texture = tex.tightPack();
            auto width = texture.width;
            auto height = texture.height;
            auto mip = 0u;
            auto slice = 0;

            TextureDescription descmod;
            descmod
                .arraySlices(1)
                .depth(1)
                .dimension(ResourceDimension::Texture2D)
                .format(texture.format)
                .width(texture.width)
                .height(texture.height)
                .mipLevels(1)
                .name("copy tex cpu gpu")
                .optimizedClearValue({ 0.0f, 0.0f, 0.0f, 0.0f })
                .optimizedDepthClearValue(0)
                .optimizedStencilClearValue(0)
                .samples(1)
                .usage(ResourceUsage::Upload);


            if (width > 128 && height > 128)
            {
                //unsigned int subResourceIndex = mip + (slice * descmod.descriptor.mipLevels);
                uint8_t* srcData = texture.data->data();

                auto future = const_cast<Device&>(device)
                    .residencyV2().upload(
                        srcData,
                        width,
                        height,
                        dsttex.texture(),
                        0, 0,
                        mip,
                        slice,
                        descmod.descriptor.mipLevels);
                future.blockUntilUploaded();
            }
            else
            {
#if 1
                auto info = surfaceInformation(descmod.descriptor.format, width, height);

                // copy to CPU memory
                D3D12_SUBRESOURCE_FOOTPRINT  srcFootprint;
                srcFootprint.Depth = 1;
                srcFootprint.Format = dxFormat(descmod.descriptor.format);
                srcFootprint.Width = static_cast<UINT>(width);
                srcFootprint.Height = static_cast<UINT>(height);
                srcFootprint.RowPitch = static_cast<UINT>(Align(
                    info.rowBytes,
#ifndef _DURANGO
                    static_cast<size_t>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)));
#else
                    static_cast<size_t>(D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT)));
#endif

                auto uploadData = m_allocator.allocate(info.numBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                if (!uploadData.ptr)
                {
                    m_graphicsQueueUploadFence->increaseCPUValue();
                    m_deviceGraphicsQueue->signal(*m_graphicsQueueUploadFence, m_graphicsQueueUploadFence->currentCPUValue());
                    m_graphicsQueueUploadFence->blockUntilSignaled();

                    processUploads(0, true);
                    m_allocator.reset();
                    uploadData = m_allocator.allocate(info.numBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                    if (!uploadData.ptr)
                    {
                        ASSERT(false, "Ran out of memory");
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_uploadAllocations.emplace_back(UploadAllocation{ uploadData, m_currentFenceValue });
                }

                D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D = { 0 };
                placedTexture2D.Offset = m_allocator.offset(uploadData.ptr);
                placedTexture2D.Footprint = srcFootprint;

                const uint8_t* srcData = texture.data->data();
                for (int i = 0; i < static_cast<int>(info.numRows); ++i)
                {
                    UINT8* pScan = static_cast<uint8_t*>(uploadData.ptr) + (i * srcFootprint.RowPitch);
                    memcpy(pScan, srcData, info.rowBytes);
                    srcData += info.rowBytes;
                }

                unsigned int mipSlice = mip;
                unsigned int arraySlice = slice;
                unsigned int mipLevels = static_cast<unsigned int>(descmod.descriptor.mipLevels);
                unsigned int subResourceIndex = mipSlice + (arraySlice * mipLevels);

                // copy to GPU memory
                D3D12_TEXTURE_COPY_LOCATION dst;
                dst.pResource = static_cast<TextureImplDX12*>(dsttex.texture().m_impl)->native();
                dst.SubresourceIndex = subResourceIndex;
                dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

                D3D12_TEXTURE_COPY_LOCATION srcLocation;
                srcLocation.pResource = static_cast<BufferImplDX12*>(m_uploadBuffer.m_implementation.get())->native();
                srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                srcLocation.PlacedFootprint = placedTexture2D;

                D3D12_BOX srcBox;
                srcBox.left = 0;
                srcBox.top = 0;
                srcBox.right = static_cast<UINT>(width);
                srcBox.bottom = static_cast<UINT>(height);
                srcBox.front = 0;
                srcBox.back = 1;

                CommandList texcmdb = device.createCommandList("DeviceImplDX12::createTexture");

                Texture gpuTex = dsttex.texture();
                transitionCommon(texcmdb, gpuTex, ResourceState::CopyDest);

                static_cast<CommandListImplDX12*>(texcmdb.native())->native()->CopyTextureRegion(
                    &dst,
                    0, 0, 0,
                    &srcLocation,
                    &srcBox);
                device.submitBlocking(texcmdb);
                processUploads(0, true);
#endif
            }
        }

		void DeviceImplDX12::setCurrentFenceValue(CommandListType type, engine::FenceValue value)
		{
            if (type == CommandListType::Direct)
                m_currentFenceValue = value;
            else if (type == CommandListType::Copy)
                m_currentCopyFenceValue = value;
		}

        void DeviceImplDX12::uploadRawBuffer(CommandListImplIf* commandList, Buffer buffer, const ByteRange& data, size_t startBytes)
        {
			//uploadBuffer(commandList, buffer, data, startElement);

#ifdef UPLOADBUFFER_MEMORYALLOCATOR_ALIGNEDCHUNKS
			auto allocator = &m_allocator;
			engine::shared_ptr<void> uploadData = engine::shared_ptr<void>(
				m_allocator.allocate(data.size(), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT),
				[allocator](void* ptr) { allocator->free(ptr); });
			m_currentUploadBufferList.emplace_back(uploadData);
			memcpy(uploadData.get(), data.start, data.size());

			commandList.native()->CopyBufferRegion(
				BufferImplGet::impl(BufferSRVImplGet::impl(buffer)->buffer())->native(),
				startElement * BufferSRVImplGet::impl(buffer)->description().elementSize,
				m_uploadBuffer.m_implementation->native(),
				m_allocator.offset(uploadData.get()), data.size());
#endif

#ifdef UPLOADBUFFER_MEMORYALLOCATOR_RINGBUFFER
			auto uploadData = m_allocator.allocate(data.sizeBytes(), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

			if (!uploadData.ptr)
			{
                m_graphicsQueueUploadFence->increaseCPUValue();
                m_deviceGraphicsQueue->signal(*m_graphicsQueueUploadFence, m_graphicsQueueUploadFence->currentCPUValue());
                m_graphicsQueueUploadFence->blockUntilSignaled();

				processUploads(0, true);
				m_allocator.reset();
				uploadData = m_allocator.allocate(data.sizeBytes(), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
				if (!uploadData.ptr)
				{
					ASSERT(false, "Ran out of memory");
				}
			}

			memcpy(uploadData.ptr, reinterpret_cast<uint8_t*>(data.start), data.sizeBytes());
			commandList->transition(buffer, ResourceState::CopyDest);
			commandList->applyBarriers();
			static_cast<CommandListImplDX12*>(commandList)->native()->CopyBufferRegion(
				static_cast<BufferImplDX12*>(buffer.m_impl)->native(),
				startBytes,
                static_cast<BufferImplDX12*>(m_uploadBuffer.m_implementation.get())->native(),
				m_allocator.offset(uploadData.ptr), data.sizeBytes());

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_uploadAllocations.emplace_back(UploadAllocation{ uploadData, m_currentFenceValue });
            }
#endif
        }

        void DeviceImplDX12::uploadBuffer(CommandList& commandList, BufferSRV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(*static_cast<CommandListImplDX12*>(commandList.native()), buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandList& commandList, BufferUAV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(*static_cast<CommandListImplDX12*>(commandList.native()), buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandList& commandList, BufferCBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(*static_cast<CommandListImplDX12*>(commandList.native()), buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandList& commandList, BufferIBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(*static_cast<CommandListImplDX12*>(commandList.native()), buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandList& commandList, BufferVBV buffer, const ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(*static_cast<CommandListImplDX12*>(commandList.native()), buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandListImplDX12& commandList, BufferSRV buffer, const tools::ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandListImplDX12& commandList, BufferUAV buffer, const tools::ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandListImplDX12& commandList, BufferCBV buffer, const tools::ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandListImplDX12& commandList, BufferIBV buffer, const tools::ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(commandList, buffer.buffer(), data, startElement);
        }

        void DeviceImplDX12::uploadBuffer(CommandListImplDX12& commandList, BufferVBV buffer, const tools::ByteRange& data, size_t startElement)
        {
            uploadBufferInternal(commandList, buffer.buffer(), data, startElement);
        }


		void DeviceImplDX12::uploadBufferInternal(CommandListImplDX12& commandList, Buffer buffer, const tools::ByteRange& data, size_t startElement)
		{
#ifdef UPLOADBUFFER_MEMORYALLOCATOR_ALIGNEDCHUNKS
			auto allocator = &m_allocator;
			engine::shared_ptr<void> uploadData = engine::shared_ptr<void>(
				m_allocator.allocate(data.size(), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT),
				[allocator](void* ptr) { allocator->free(ptr); });
			m_currentUploadBufferList.emplace_back(uploadData);
			memcpy(uploadData.get(), data.start, data.size());

			commandList.native()->CopyBufferRegion(
				buffer.m_impl->native(),
				startElement * buffer.description().elementSize,
				m_uploadBuffer.m_implementation->native(),
				m_allocator.offset(uploadData.get()), data.size());
#endif

#ifdef UPLOADBUFFER_MEMORYALLOCATOR_RINGBUFFER
			auto uploadData = m_allocator.allocate(data.sizeBytes(), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

            if (!uploadData.ptr)
			{
                m_graphicsQueueUploadFence->increaseCPUValue();
                m_deviceGraphicsQueue->signal(*m_graphicsQueueUploadFence, m_graphicsQueueUploadFence->currentCPUValue());
                m_graphicsQueueUploadFence->blockUntilSignaled();

				processUploads(0, true);
				m_allocator.reset();
				uploadData = m_allocator.allocate(data.sizeBytes(), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
				if (!uploadData.ptr)
				{
					ASSERT(false, "Ran out of memory");
				}
			}

			memcpy(uploadData.ptr, reinterpret_cast<uint8_t*>(data.start), data.sizeBytes());
			commandList.transition(buffer, ResourceState::CopyDest);
			commandList.applyBarriers();
			commandList.native()->CopyBufferRegion(
				static_cast<BufferImplDX12*>(buffer.m_impl)->native(),
				startElement * buffer.description().elementSize,
                static_cast<BufferImplDX12*>(m_uploadBuffer.m_implementation.get())->native(),
				m_allocator.offset(uploadData.ptr), data.sizeBytes());

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_uploadAllocations.emplace_back(UploadAllocation{ uploadData, m_currentFenceValue });
            }
			
#endif
		}

		engine::shared_ptr<TextureImplIf> DeviceImplDX12::createTexture(const Device& device, Queue& queue, const TextureDescription& desc)
        {
            auto descmod = desc;
            if (isBlockCompressedFormat(descmod.descriptor.format))
            {
                if (descmod.descriptor.width < 4)
                    descmod.descriptor.width = 4;
                if (descmod.descriptor.height < 4)
                    descmod.descriptor.height = 4;
            }

            if (descmod.initialData.data.size() > 0)
            {
                auto gpuDesc = descmod;
                auto gpuTexture = createTexture(device, queue, gpuDesc
                    .usage(ResourceUsage::GpuRead)
                    .setInitialData(TextureDescription::InitialData()));
				Texture gpuTex(gpuTexture.get());

                if (descmod.initialData.data.size() > 0)
                {
                    size_t dataIndex = 0;
                    for (size_t slice = 0; slice < descmod.descriptor.arraySlices; ++slice)
                    {
                        size_t width = descmod.descriptor.width;
                        size_t height = descmod.descriptor.height;

                        for (size_t mip = 0; mip < descmod.descriptor.mipLevels; ++mip)
                        {
							auto info = surfaceInformation(descmod.descriptor.format, width, height);

							if (width > 128 && height > 128)
							{
								//unsigned int subResourceIndex = mip + (slice * descmod.descriptor.mipLevels);
								uint8_t* srcData = &descmod.initialData.data[dataIndex];

								auto future = const_cast<Device&>(device)
									.residencyV2().upload(
										srcData,
										width,
										height,
										gpuTex,
                                        0, 0,
										mip,
										slice,
										descmod.descriptor.mipLevels);
								future.blockUntilUploaded();

								dataIndex += info.numBytes;

								width /= 2;
								height /= 2;
								if (isBlockCompressedFormat(descmod.descriptor.format))
								{
									if (width < 4)
										width = 4;
									if (height < 4)
										height = 4;
								}
								else
								{
									if (width < 1)
										width = 1;
									if (height < 1)
										height = 1;
								}
							}
							else
							{
#if 1
								// copy to CPU memory
								D3D12_SUBRESOURCE_FOOTPRINT  srcFootprint;
								srcFootprint.Depth = 1;
								srcFootprint.Format = dxFormat(descmod.descriptor.format);
								srcFootprint.Width = static_cast<UINT>(width);
								srcFootprint.Height = static_cast<UINT>(height);
								srcFootprint.RowPitch = static_cast<UINT>(Align(
									info.rowBytes,
#ifndef _DURANGO
									static_cast<size_t>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)));
#else
									static_cast<size_t>(D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT)));
#endif

								auto uploadData = m_allocator.allocate(info.numBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                                if (!uploadData.ptr)
								{
                                    m_graphicsQueueUploadFence->increaseCPUValue();
                                    m_deviceGraphicsQueue->signal(*m_graphicsQueueUploadFence, m_graphicsQueueUploadFence->currentCPUValue());
                                    m_graphicsQueueUploadFence->blockUntilSignaled();

									processUploads(0, true);
									m_allocator.reset();
									uploadData = m_allocator.allocate(info.numBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
									if (!uploadData.ptr)
									{
										ASSERT(false, "Ran out of memory");
									}
								}
                                {
                                    std::lock_guard<std::mutex> lock(m_mutex);
                                    m_uploadAllocations.emplace_back(UploadAllocation{ uploadData, m_currentFenceValue });
                                }

								D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D = { 0 };
								placedTexture2D.Offset = m_allocator.offset(uploadData.ptr);
								placedTexture2D.Footprint = srcFootprint;

								const uint8_t* srcData = &descmod.initialData.data[dataIndex];
								for (int i = 0; i < static_cast<int>(info.numRows); ++i)
								{
									UINT8 *pScan = static_cast<uint8_t*>(uploadData.ptr) + (i * srcFootprint.RowPitch);
									memcpy(pScan, srcData, info.rowBytes);
									srcData += info.rowBytes;
								}
								dataIndex += info.numBytes;

								unsigned int mipSlice = static_cast<unsigned int>(mip);
								unsigned int arraySlice = static_cast<unsigned int>(slice);
								unsigned int mipLevels = static_cast<unsigned int>(descmod.descriptor.mipLevels);
								unsigned int subResourceIndex = mipSlice + (arraySlice * mipLevels);

								// copy to GPU memory
								D3D12_TEXTURE_COPY_LOCATION dst;
								dst.pResource = static_cast<TextureImplDX12*>(gpuTexture.get())->native();
								dst.SubresourceIndex = subResourceIndex;
								dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

								D3D12_TEXTURE_COPY_LOCATION srcLocation;
								srcLocation.pResource = static_cast<BufferImplDX12*>(m_uploadBuffer.m_implementation.get())->native();
								srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
								srcLocation.PlacedFootprint = placedTexture2D;

								D3D12_BOX srcBox;
								srcBox.left = 0;
								srcBox.top = 0;
								srcBox.right = static_cast<UINT>(width);
								srcBox.bottom = static_cast<UINT>(height);
								srcBox.front = 0;
								srcBox.back = 1;

								CommandList texcmdb = device.createCommandList("DeviceImplDX12::createTexture");

								//Texture gpuTex(gpuTexture.get());
								transitionCommon(texcmdb, gpuTex, ResourceState::CopyDest);

								static_cast<CommandListImplDX12*>(texcmdb.native())->native()->CopyTextureRegion(
									&dst,
									0, 0, 0,
									&srcLocation,
									&srcBox);
								Fence texfence = device.createFence("Device texture upload fence");
								texfence.increaseCPUValue();
								queue.submit(texcmdb, texfence);
								texfence.blockUntilSignaled();

								width /= 2;
								height /= 2;
								if (isBlockCompressedFormat(descmod.descriptor.format))
								{
									if (width < 4)
										width = 4;
									if (height < 4)
										height = 4;
								}
								else
								{
									if (width < 1)
										width = 1;
									if (height < 1)
										height = 1;
								}
#endif
							}
							
                        }
                    }
                }
                return gpuTexture;
            }
            else
                return engine::make_shared<TextureImplDX12>(*this, descmod);
        }

        void DeviceImplDX12::waitForIdle()
        {
            // TODO
        }

        DescriptorHeapsDX12& DeviceImplDX12::heaps()
        {
            return m_descriptorHeaps;
        }

        const DescriptorHeapsDX12& DeviceImplDX12::heaps() const
        {
            return m_descriptorHeaps;
        }
    }
}
