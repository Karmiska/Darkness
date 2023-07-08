#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Pipeline.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12Resources.h"
#include "engine/graphics/dx12/DX12Sampler.h"
#include "engine/graphics/dx12/DX12Semaphore.h"
#include "engine/graphics/dx12/DX12CommandAllocator.h"
#include "engine/graphics/dx12/DX12DescriptorHeap.h"

#include "engine/Engine.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/primitives/Color.h"
#include "shaders/ShaderTypes.h"

#include "tools/Debug.h"

constexpr int QueryBufferElements = 10000;
#undef CHECK_FOR_MULTIPLE_RESOURCE_STATE

namespace engine
{
    namespace implementation
    {
        struct pairhash {
        public:
            template <typename T, typename U>
            std::size_t operator()(const std::pair<T, U> &x) const
            {
                return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
            }
        };

        CommandListImplDX12::CommandListImplDX12(Device* device, CommandListType type, const char* name)
#ifdef AFTERMATH_ENABLED
            : m_afterMathContext{ nullptr }
            , m_device{ device }
#else
            : m_device{ device }
#endif
            , m_type{ type }
            , m_gpuMarkers{ static_cast<DeviceImplDX12*>(m_device->native())->getMarkerContainer() }
            , m_open{ false }
            , m_descriptorHeaps{ static_cast<DeviceImplDX12*>(m_device->native())->heaps() }
            , m_lastPipelineState{ nullptr }
            , m_lastTopology{ D3D_PRIMITIVE_TOPOLOGY_UNDEFINED }
            , m_resolved{ false }
            , m_lastAppliedBarrierIndex{ 0 }
            , m_pipeBound{ false }
        {
            //m_allocator = engine::make_shared<CommandAllocatorImpl>(*device);
            m_allocator = static_pointer_cast<CommandAllocatorImplDX12>(static_cast<DeviceImplDX12*>(m_device->native())->createCommandAllocator(type, name));
            auto res = static_cast<DeviceImplDX12*>(m_device->native())->device()->CreateCommandList(
                0,
                dxCommandListType(type),
                static_cast<CommandAllocatorImplDX12*>(m_allocator.get())->native(),
                NULL,
                DARKNESS_IID_PPV_ARGS(m_commandList.GetAddressOf()));
            ASSERT(SUCCEEDED(res));

#ifdef AFTERMATH_ENABLED
            m_afterMathContext = afterMathContext.allocate();
            auto afterMathResult = GFSDK_Aftermath_DX12_CreateContextHandle(
                m_commandList.Get(), 
                &(*m_afterMathContext));
            ASSERT(afterMathResult == GFSDK_Aftermath_Result_Success, "Aftermath failed to create context handle");
            LOG("Creating aftermath handle: %p -> %p", m_afterMathContext, *m_afterMathContext);
#endif

#ifdef DXR_BUILD
			auto dxrResult = m_commandList->QueryInterface(DARKNESS_IID_PPV_ARGS(m_dxrCommandList.GetAddressOf()));
			ASSERT(SUCCEEDED(res), "Could not create DXR commandlist");
#endif

            if (type != CommandListType::Copy)
            {
                setHeaps();
            }

            if (type == CommandListType::Direct)
            {
                // create query heap
                D3D12_QUERY_HEAP_DESC queryHeapDesc;
                if (m_type != CommandListType::Copy)
                    queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                else
                    queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP;
                queryHeapDesc.Count = QueryBufferElements;
                queryHeapDesc.NodeMask = 0;
                auto qheapRes = static_cast<DeviceImplDX12*>(m_device->native())->m_device->CreateQueryHeap(&queryHeapDesc, DARKNESS_IID_PPV_ARGS(m_queryHeap.GetAddressOf()));
                ASSERT(SUCCEEDED(qheapRes), "Failed to create query heap");
                
                // create timestamping query buffer
				m_queryBuffer = m_device->createBuffer(
					BufferDescription()
					.usage(ResourceUsage::GpuToCpu)
					.elementSize(sizeof(uint64_t))
					.elements(QueryBufferElements)
					.name("Timestamp buffer"));
				m_queryBufferHandle = m_queryBuffer;
            }
            m_open = true;
        }

		CommandListType CommandListImplDX12::type() const
		{
			return m_type;
		}

        void CommandListImplDX12::setHeaps()
        {
            engine::vector<ID3D12DescriptorHeap*> heaps{
                    static_cast<DeviceImplDX12*>(m_device->native())->heaps().shaderVisible_cbv_srv_uav->native(),
                    static_cast<DeviceImplDX12*>(m_device->native())->heaps().shaderVisible_sampler->native()
            };

            m_commandList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.data());
        }

        void CommandListImplDX12::applyBarriers()
        {
#ifdef CHECK_FOR_MULTIPLE_RESOURCE_STATE
            UINT count = static_cast<UINT>(m_barriers.size() - m_lastAppliedBarrierIndex);
            if (count > 0)
            {
                engine::unordered_map<std::pair<ID3D12Resource*, UINT>, engine::vector<D3D12_RESOURCE_BARRIER>, pairhash> resources;
                for (size_t i = m_lastAppliedBarrierIndex; i < m_barriers.size(); ++i)
                {
                    auto key = std::make_pair(m_barriers[i].Transition.pResource, m_barriers[i].Transition.Subresource);
                    auto res = resources.find(key);
                    if (res != resources.end())
                    {
                        (*res).second.emplace_back(m_barriers[i]);
                    }
                    else
                    {
                        engine::vector<D3D12_RESOURCE_BARRIER> vec;
                        vec.emplace_back(m_barriers[i]);
                        resources[key] = vec;
                    }
                }
                engine::vector<D3D12_RESOURCE_BARRIER> barriers;
                barriers.reserve(resources.size());
                for (auto&& keyValue : resources)
                {
                    auto bars = keyValue.second;
                    if (bars.size() > 1)
                    {
                        // same resource bound multiple times to the same operation
                        D3D12_RESOURCE_BARRIER newBarrier;
                        newBarrier.Flags = bars[0].Flags;
                        newBarrier.Type = bars[0].Type;
                        newBarrier.Transition.pResource = bars[0].Transition.pResource;
                        newBarrier.Transition.Subresource = bars[0].Transition.Subresource;
                        newBarrier.Transition.StateBefore = bars[0].Transition.StateBefore;
                        newBarrier.Transition.StateAfter = bars[0].Transition.StateAfter;

                        for(int i = 0; i < bars.size(); ++i)
                        {
                            if (bars[i].Transition.StateAfter == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
                                newBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
                        }
                        if(newBarrier.Transition.StateBefore != newBarrier.Transition.StateAfter)
                            barriers.emplace_back(newBarrier);
                    }
                    else
                    {
                        barriers.emplace_back(bars[0]);
                    }
                }

                m_commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), &barriers[0]);
                m_lastAppliedBarrierIndex = m_barriers.size();
            }
#else
			auto barrierCount = m_barriers.size() - m_lastAppliedBarrierIndex;
			if (barrierCount)
			{
				m_commandList->ResourceBarrier(static_cast<UINT>(barrierCount), &m_barriers[m_lastAppliedBarrierIndex]);
				m_lastAppliedBarrierIndex = m_barriers.size();
			}
#endif
        }

        uint32_t CommandListImplDX12::startQuery(const char* query)
        {
            auto res = m_gpuMarkers->startQuery(query);
            m_commandList->EndQuery(
                m_queryHeap.Get(),
                D3D12_QUERY_TYPE_TIMESTAMP,
                res);
            return res;
        }

        void CommandListImplDX12::stopQuery(uint32_t queryId)
        {
            auto stopId = m_gpuMarkers->stopQuery(queryId);
            m_commandList->EndQuery(
                m_queryHeap.Get(),
                D3D12_QUERY_TYPE_TIMESTAMP,
                stopId);
        }

        void CommandListImplDX12::resolveQueries()
        {
            auto queryCount = m_gpuMarkers->queryCount();
            if (queryCount == 0)
                return;

            ASSERT(!m_resolved, "Command list already resolved");
            m_resolved = true;

            transition(m_queryBufferHandle, ResourceState::CopyDest);
            m_commandList->ResolveQueryData(
                m_queryHeap.Get(),
                D3D12_QUERY_TYPE_TIMESTAMP,
                0,
                queryCount,
                static_cast<BufferImplDX12*>(m_queryBufferHandle.m_impl)->native(),
                0
            );
            transition(m_queryBufferHandle, ResourceState::GenericRead);
        }

        void insertInto(uint64_t* ptr, uint64_t freq, engine::vector<QueryResultTicks>& list, GpuMarkerContainer::MarkerItem& marker)
        {
            for (auto& item : list)
            {
                if (marker.start > item.start && marker.start < item.stop)
                {
                    insertInto(ptr, freq, item.childs, marker);
                    return;
                }
            }

            uint64_t start = *(ptr + marker.start);
            uint64_t stop = *(ptr + marker.stop);
            list.emplace_back(QueryResultTicks{
                    marker.query,
                    static_cast<float>(static_cast<double>((stop - start) * 1000) / static_cast<double>(freq)),
                    marker.start, marker.stop,
                    start, stop });
        }

        engine::vector<QueryResultTicks> CommandListImplDX12::fetchQueryResults(uint64_t freq)
        {
			if (!m_queryBufferHandle.m_impl)
				return {};
            uint64_t* ptr = reinterpret_cast<uint64_t*>(m_queryBufferHandle.m_impl->map(m_device->native()));
            engine::vector<QueryResultTicks> res;
            //for (auto& marker : m_gpuMarkers->items())
            engine::vector<GpuMarkerContainer::MarkerItem>& items = m_gpuMarkers->items();
            for(uint32_t i = 0; i < m_gpuMarkers->markerCount(); ++i)
            {
                insertInto(ptr, freq, res, items[i]);
            }
			m_queryBufferHandle.m_impl->unmap(m_device->native());
            return res;
        }

        void CommandListImplDX12::clear()
        {
            ASSERT(!m_open, "Tried to clear open list");
            m_open = true;
            m_lastTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            m_lastAppliedBarrierIndex = 0;
            m_lastPipelineState = nullptr;
            m_barriers.clear();

            m_abs.m_debugBuffers.clear();

            m_abs.m_lastSetRTVFormats.clear();
            m_abs.commandListSemaphore->reset();
            m_allocator->reset();
            m_commandList->Reset(m_allocator->native(), nullptr);
            if (m_type != CommandListType::Copy)
            {
                setHeaps();
            }

            m_gpuMarkers->reset();
            m_querys.clear();
            m_resolved = false;

#ifdef AFTERMATH_ENABLED
            // release old
            LOG("Destroying aftermath handle: %p -> %p", m_afterMathContext, *m_afterMathContext);
            auto afterMathResult = GFSDK_Aftermath_ReleaseContextHandle(*m_afterMathContext);
            ASSERT(afterMathResult == GFSDK_Aftermath_Result_Success, "Aftermath failed to release context handle");
            afterMathContext.free(m_afterMathContext);

            // allocate new
            m_afterMathContext = afterMathContext.allocate();
            auto afterMathResult2 = GFSDK_Aftermath_DX12_CreateContextHandle(
                m_commandList.Get(),
                &(*m_afterMathContext));
            ASSERT(afterMathResult2 == GFSDK_Aftermath_Result_Success, "Aftermath failed to create context handle");
            LOG("Creating aftermath handle: %p -> %p", m_afterMathContext, *m_afterMathContext);
#endif
        }

        CommandListImplDX12::~CommandListImplDX12()
        {
            end();
            clear();
            //static_cast<DeviceImplDX12*>(m_device->native())->freeCommandAllocator(m_allocator);
            for (auto&& sig : m_commandSignatures)
            {
                sig->Release();
            }
            m_commandSignatures.clear();
            static_cast<DeviceImplDX12*>(m_device->native())->returnMarkerContainer(std::move(m_gpuMarkers));

#ifdef AFTERMATH_ENABLED
            if (m_afterMathContext)
            {
                LOG("Destroying aftermath handle: %p -> %p", m_afterMathContext, *m_afterMathContext);
                auto afterMathResult = GFSDK_Aftermath_ReleaseContextHandle(*m_afterMathContext);
                ASSERT(afterMathResult == GFSDK_Aftermath_Result_Success, "Aftermath failed to release context handle");
                afterMathContext.free(m_afterMathContext);
                m_afterMathContext = nullptr;
            }
#endif
        }

        void CommandListImplDX12::reset(implementation::PipelineImplIf* pipelineState)
        {
            auto res = m_commandList->Reset(m_allocator->native(), static_cast<PipelineImplDX12*>(pipelineState)->m_pipelineState.pipeline());
            ASSERT(SUCCEEDED(res));
        }

        void CommandListImplDX12::executeBundle(CommandListImplIf* commandList)
        {
            applyBarriers();
            m_commandList->ExecuteBundle(static_cast<CommandListImplDX12*>(commandList)->native());
        }

        void CommandListImplDX12::transition(Texture resource, ResourceState state, const SubResource& subResource)
        {
            TextureImplIf* impl = resource.m_impl;

            auto localSubRes = subResource;

            if ((localSubRes.arraySliceCount == AllArraySlices) &&
                (localSubRes.mipCount == AllMipLevels))
            {
                bool forceIndividual = false;
                auto firstState = impl->state(0, 0);
                for (int slice = 0; slice < static_cast<int>(resource.arraySlices()); ++slice)
                {
                    for (int mip = 0; mip < static_cast<int>(resource.mipLevels()); ++mip)
                    {
                        if (firstState != impl->state(slice, mip))
                        {
                            forceIndividual = true;
                            break;
                        }
                    }
                    if (forceIndividual)
                        break;
                }

                if (forceIndividual)
                {
                    localSubRes.firstArraySlice = 0;
                    localSubRes.firstMipLevel = 0;
                    localSubRes.arraySliceCount = static_cast<int32_t>(resource.arraySlices());
                    localSubRes.mipCount = static_cast<int32_t>(resource.mipLevels());
                }
            }

            if ((localSubRes.arraySliceCount == AllArraySlices) &&
                (localSubRes.mipCount == AllMipLevels))
            {
                ResourceState anystate = state;
                for (int slice = 0; slice < static_cast<int>(resource.arraySlices()); ++slice)
                {
                    for (int mip = 0; mip < static_cast<int>(resource.mipLevels()); ++mip)
                    {
                        if (impl->state(slice, mip) != anystate)
                        {
                            anystate = impl->state(slice, mip);
                            break;
                        }
                    }
                }
                if (anystate != state)
                {
                    D3D12_RESOURCE_BARRIER barrier = {};
                    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barrier.Transition.pResource = static_cast<TextureImplDX12*>(impl)->native();
                    barrier.Transition.StateBefore = dxResourceStates(anystate);
                    barrier.Transition.StateAfter = dxResourceStates(state);
                    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                    m_barriers.emplace_back(barrier);

                    for (int slice = 0; slice < static_cast<int>(resource.arraySlices()); ++slice)
                    {
                        for (int mip = 0; mip < static_cast<int>(resource.mipLevels()); ++mip)
                        {
                            impl->state(slice, mip, state);
                        }
                    }
                }
            }
            else
            {
                uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
                    static_cast<uint32_t>(resource.arraySlices()) :
                    static_cast<uint32_t>(std::min(localSubRes.arraySliceCount, static_cast<int32_t>(resource.arraySlices() - static_cast<size_t>(localSubRes.firstArraySlice))));

                uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
                    static_cast<uint32_t>(resource.mipLevels()) :
                    static_cast<uint32_t>(std::min(localSubRes.mipCount, static_cast<int32_t>(resource.mipLevels() - static_cast<size_t>(localSubRes.firstMipLevel))));

                for(int slice = static_cast<int>(localSubRes.firstArraySlice); slice < static_cast<int>(localSubRes.firstArraySlice + sliceCount); ++slice)
                {
                    for (int mip = static_cast<int>(localSubRes.firstMipLevel); mip < static_cast<int>(localSubRes.firstMipLevel + mipCount); ++mip)
                    {
                        if (impl->state(slice, mip) != state)
                        {
                            D3D12_RESOURCE_BARRIER barrier = {};
                            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                            barrier.Transition.pResource = static_cast<TextureImplDX12*>(impl)->native();
                            barrier.Transition.StateBefore = dxResourceStates(impl->state(slice, mip));
                            barrier.Transition.StateAfter = dxResourceStates(state);
                            //if((barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) &&
                            //    (barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
                            barrier.Transition.Subresource = static_cast<UINT>(static_cast<size_t>(mip) + (static_cast<size_t>(slice) * resource.mipLevels()));
                            m_barriers.emplace_back(barrier);
                            impl->state(slice, mip, state);
                        }
                    }
                }
            }
        }

        void CommandListImplDX12::transition(TextureRTV resource, ResourceState state)
        {
            transition(resource.texture(), state, resource.subResource());
        }

        void CommandListImplDX12::transition(TextureSRV resource, ResourceState state)
        {
            transition(resource.texture(), state, resource.subResource());
        }

        void CommandListImplDX12::transition(TextureDSV resource, ResourceState state)
        {
            transition(resource.texture(), state, resource.subResource());
        }

        void CommandListImplDX12::transition(Buffer resource, ResourceState state)
        {
            BufferImplIf* impl = resource.m_impl;
            if (impl->state() == state)
                return;

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = static_cast<BufferImplDX12*>(impl)->native();
            barrier.Transition.StateBefore = dxResourceStates(impl->state());
            barrier.Transition.StateAfter = dxResourceStates(state);
            barrier.Transition.Subresource = 0;// D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            m_barriers.emplace_back(barrier);

            impl->state(state);
        }

        void CommandListImplDX12::transition(BufferSRV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplDX12::transition(BufferIBV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplDX12::transition(BufferCBV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplDX12::transition(BufferVBV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplDX12::setPredicate(BufferSRV buffer, uint64_t offset, PredicationOp op)
        {
            applyBarriers();

            if(buffer.valid())
                m_commandList->SetPredication(
                    static_cast<BufferImplDX12*>(buffer.buffer().m_impl)->native(),
                    offset,
                    dxPredicationOp(op));
            else
                m_commandList->SetPredication(
                    nullptr,
                    offset,
                    dxPredicationOp(op));
        }

		void CommandListImplDX12::setRenderTargets(TextureDSV target)
		{
			applyBarriers();

			engine::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(1);
			handles[0] = static_cast<TextureDSVImplDX12*>(target.m_impl)->native();
			
			m_commandList->OMSetRenderTargets(
				0,
				NULL,
				FALSE,
				handles.data());

			// we could also set these manually, but it's only a minor
			// optimization and a major pain in the ass to remember.
			// so we'll set them right away
			setViewPorts({
				engine::Viewport{
				0, 0,
				static_cast<float>(target.width()),
				static_cast<float>(target.height()),
				0.0f, 1.0f } });
			setScissorRects({ engine::Rectangle{ 0, 0,
				static_cast<int>(target.width()),
				static_cast<int>(target.height()) } });
		}

        void CommandListImplDX12::setRenderTargets(engine::vector<TextureRTV> targets)
        {
            applyBarriers();

            engine::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(targets.size());
            for (size_t i = 0; i < targets.size(); ++i)
            {
                handles[i] = static_cast<TextureRTVImplDX12*>(targets[i].m_impl)->native();
            }
            m_commandList->OMSetRenderTargets(
                static_cast<UINT>(targets.size()), 
                handles.data(), 
                FALSE, 
                NULL);

            // we could also set these manually, but it's only a minor
            // optimization and a major pain in the ass to remember.
            // so we'll set them right away
            if (targets.size() > 0)
            {
                setViewPorts({
                    engine::Viewport{
                    0, 0,
                    static_cast<float>(targets[0].width()),
                    static_cast<float>(targets[0].height()),
                    0.0f, 1.0f } });
                setScissorRects({ engine::Rectangle{ 0, 0,
                    static_cast<int>(targets[0].width()),
                    static_cast<int>(targets[0].height()) } });
            }
        }

        void CommandListImplDX12::setRenderTargets(engine::vector<TextureRTV> targets, TextureDSV dsv)
        {
            applyBarriers();
            engine::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
            for (size_t i = 0; i < targets.size(); ++i)
            {
                if(targets[i].valid())
                    handles.emplace_back(static_cast<TextureRTVImplDX12*>(targets[i].m_impl)->native());
            }
            if (handles.size() > 0)
            {
                m_commandList->OMSetRenderTargets(
                    static_cast<UINT>(handles.size()),
                    handles.data(),
                    FALSE,
                    &static_cast<TextureDSVImplDX12*>(dsv.m_impl)->native());
            }
            else
            {
                m_commandList->OMSetRenderTargets(
                    0,
                    nullptr,
                    FALSE,
                    &static_cast<TextureDSVImplDX12*>(dsv.m_impl)->native());
            }

            // we could also set these manually, but it's only a minor
            // optimization and a major pain in the ass to remember.
            // so we'll set them right away
            if (targets.size() > 0)
            {
                setViewPorts({
                    engine::Viewport{
                    0, 0,
                    static_cast<float>(targets[0].width()),
                    static_cast<float>(targets[0].height()),
                    0.0f, 1.0f } });
                setScissorRects({ engine::Rectangle{ 0, 0,
                    static_cast<int>(targets[0].width()),
                    static_cast<int>(targets[0].height()) } });
            }
            else
            {
                setViewPorts({
                    engine::Viewport{
                    0, 0,
                    static_cast<float>(dsv.texture().width()),
                    static_cast<float>(dsv.texture().height()),
                    0.0f, 1.0f } });
                setScissorRects({ engine::Rectangle{ 0, 0,
                    static_cast<int>(dsv.texture().width()),
                    static_cast<int>(dsv.texture().height()) } });
            }
        }

        void CommandListImplDX12::copyBuffer(
			Buffer srcBuffer,
            Buffer dstBuffer,
            uint64_t elements,
            size_t srcStartElement,
            size_t dstStartElement)
        {
            applyBarriers();

            auto dstElementSize = dstBuffer.description().elementSize;
            if (dstElementSize == -1)
                dstElementSize = static_cast<int32_t>(formatBytes(dstBuffer.description().format));

            auto srcElementSize = srcBuffer.description().elementSize;
            if (srcElementSize == -1)
                srcElementSize = static_cast<int32_t>(formatBytes(srcBuffer.description().format));

            ASSERT(srcStartElement >= 0, "Trying to read from outside source buffer");
            ASSERT((srcStartElement + elements) * srcElementSize <= (srcBuffer.description().elementSize * srcBuffer.description().elements), "Trying to read from outside source buffer");
            ASSERT(dstStartElement >= 0, "Trying to write outside destination buffer");
            //ASSERT((dstStartElement + elements) * dstElementSize <= (dstBuffer.description().descriptor.elementSize * dstBuffer.description().descriptor.elements), "Trying to write outside destination buffer");

            m_commandList->CopyBufferRegion(
                static_cast<BufferImplDX12*>(dstBuffer.m_impl)->native(),
                static_cast<UINT64>(dstStartElement * dstElementSize),
                static_cast<BufferImplDX12*>(srcBuffer.m_impl)->native(),
                static_cast<UINT64>(srcStartElement * srcElementSize),
                elements * srcElementSize);
        }

        void CommandListImplDX12::copyBufferBytes(Buffer srcBuffer, Buffer dstBuffer, uint64_t bytes, size_t srcStartByte, size_t dstStartByte)
        {
            applyBarriers();

            ASSERT(srcStartByte >= 0, "Trying to read from outside source buffer");
            ASSERT((srcStartByte + bytes) <= (srcBuffer.description().elementSize * srcBuffer.description().elements), "Trying to read from outside source buffer");
            ASSERT(dstStartByte >= 0, "Trying to write outside destination buffer");
            ASSERT((dstStartByte + bytes) <= (dstBuffer.description().elementSize * dstBuffer.description().elements), "Trying to write outside destination buffer");

            m_commandList->CopyBufferRegion(
                static_cast<BufferImplDX12*>(dstBuffer.m_impl)->native(),
                dstStartByte,
                static_cast<BufferImplDX12*>(srcBuffer.m_impl)->native(),
                srcStartByte,
                bytes);
        }

        static ID3D12PipelineState* lastPipelineState = nullptr;

        void CommandListImplDX12::bindPipe(
            implementation::PipelineImplIf* pipelineImpl,
            shaders::PipelineConfiguration* configuration)
        {
            applyBarriers();

			static_cast<PipelineImplDX12*>(pipelineImpl)->configure(this, configuration);

            if (!static_cast<PipelineImplDX12*>(pipelineImpl)->valid())
            {
                m_pipeBound = false;
                return;
            }

            m_pipeBound = true;

            auto pipelineState = static_cast<PipelineImplDX12*>(pipelineImpl)->m_pipelineState.pipeline();
            if(m_lastPipelineState != pipelineState)
            {
                m_lastPipelineState = pipelineState;
                m_commandList->SetPipelineState(pipelineState);

                m_rootSignature = static_cast<PipelineImplDX12*>(pipelineImpl)->m_rootSignature->signature();
                if (!configuration->hasComputeShader())
                    m_commandList->SetGraphicsRootSignature(m_rootSignature);
                else
                    m_commandList->SetComputeRootSignature(m_rootSignature);
            }

            if (!configuration->hasComputeShader())
            {
                for (auto&& table : static_cast<PipelineImplDX12*>(pipelineImpl)->resourceTablePointers())
                {
                    m_commandList->SetGraphicsRootDescriptorTable(
                        table.parameterIndex, 
                        table.handle);
                }

                auto pipeTopology = static_cast<PipelineImplDX12*>(pipelineImpl)->topology();
                if (m_lastTopology != pipeTopology)
                {
                    m_commandList->IASetPrimitiveTopology(pipeTopology);
                    m_lastTopology = pipeTopology;
                }
            }
            else
            {
                for (auto&& table : static_cast<PipelineImplDX12*>(pipelineImpl)->resourceTablePointers())
                {
                    m_commandList->SetComputeRootDescriptorTable(
                        table.parameterIndex,
                        table.handle);
                }
            }
        }

        void CommandListImplDX12::setViewPorts(const engine::vector<Viewport>& viewports)
        {
            engine::vector<D3D12_VIEWPORT> viewPorts;
            for (auto&& viewport : viewports)
            {
                D3D12_VIEWPORT vport;
                vport.Width = viewport.width;
                vport.Height = viewport.height;
                vport.MinDepth = viewport.minDepth;
                vport.MaxDepth = viewport.maxDepth;
                vport.TopLeftX = viewport.topLeftX;
                vport.TopLeftY = viewport.topLeftY;
                viewPorts.emplace_back(vport);
            }
            m_commandList->RSSetViewports(static_cast<UINT>(viewPorts.size()), viewPorts.data());
        }

        void CommandListImplDX12::setScissorRects(const engine::vector<Rectangle>& rects)
        {
            engine::vector<D3D12_RECT> scissorRects;
            for (auto&& rect : rects)
            {
                D3D12_RECT scissorRect;
                scissorRect.top = rect.top;
                scissorRect.left = rect.left;
                scissorRect.right = rect.right;
                scissorRect.bottom = rect.bottom;
                scissorRects.emplace_back(scissorRect);
            }
            m_commandList->RSSetScissorRects(static_cast<UINT>(scissorRects.size()), scissorRects.data());
        }


        void CommandListImplDX12::bindVertexBuffer(BufferVBV buffer)
        {
            applyBarriers();
            m_commandList->IASetVertexBuffers(0, 1, static_cast<BufferVBVImplDX12*>(buffer.m_impl)->view());
        }

        void CommandListImplDX12::bindIndexBuffer(BufferIBV buffer)
        {
            applyBarriers();
            m_commandList->IASetIndexBuffer(static_cast<BufferIBVImplDX12*>(buffer.m_impl)->view());
        }

        void CommandListImplDX12::clearBuffer(BufferUAV buffer, uint32_t value, size_t startElement, size_t numElements)
        {
            applyBarriers();
            const uint32_t values[4]{ value, value, value, value };
            auto& bufferUAV = *static_cast<BufferUAVImplDX12*>(buffer.m_impl);

            D3D12_RECT rect;
            rect.top = 0;
            rect.left = static_cast<LONG>(startElement);
            rect.bottom = 1;
            rect.right = static_cast<LONG>(startElement + numElements);
            
            if (static_cast<BufferUAVImplDX12*>(buffer.m_impl)->viewClearHandle().cpuHandle().ptr == 0)
            {
                static_cast<BufferUAVImplDX12*>(buffer.m_impl)->viewClearHandle() = static_cast<DeviceImplDX12*>(m_device->native())->heaps().shaderVisible_cbv_srv_uav->getDescriptor(1);
                static_cast<DeviceImplDX12*>(m_device->native())->device()->CopyDescriptorsSimple(
                    1,
                    static_cast<BufferUAVImplDX12*>(buffer.m_impl)->viewClearHandle().cpuHandle(),
                    bufferUAV.cpuHandle(),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                setHeaps();
            }

            auto cpuHandle = bufferUAV.cpuHandle();
            auto gpuHandle = static_cast<BufferUAVImplDX12*>(buffer.m_impl)->viewClearHandle().gpuHandle();

            m_commandList->ClearUnorderedAccessViewUint(
                gpuHandle,
                cpuHandle,
                static_cast<BufferImplDX12*>(bufferUAV.buffer().m_impl)->native(), 
                values, 
                1, 
                &rect);
        }

        void CommandListImplDX12::clearTextureUAV(TextureUAV /*texture*/, const Color4f& /*color*/)
        {
            LOG("DX12 CommandList clearTextureUAV is unimplemented");
			ASSERT(false, "not implemented");
        }

        void CommandListImplDX12::clearTextureDSV(TextureDSV texture, float depth, uint8_t stencil)
        {
			transition(texture, ResourceState::DepthWrite);
			applyBarriers();
			engine::vector<D3D12_RECT> rects;
			rects.emplace_back(D3D12_RECT{ 0, 0,
				static_cast<LONG>(texture.width()),
				static_cast<LONG>(texture.height()) });

			bool hasStencil = texture.format() == Format::D24_UNORM_S8_UINT;

			D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH;
			if (hasStencil)
				flags |= D3D12_CLEAR_FLAG_STENCIL;

			m_commandList->ClearDepthStencilView(
                static_cast<TextureDSVImplDX12*>(texture.m_impl)->native(),
				flags,
				depth,
				stencil,
				static_cast<UINT>(rects.size()),
				rects.data());
        }

		void CommandListImplDX12::clearTextureRTV(TextureRTV texture, const Color4f& color)
		{
			transition(texture, ResourceState::RenderTarget);
			applyBarriers();
			engine::vector<D3D12_RECT> rects;
			rects.emplace_back(D3D12_RECT{ 0, 0,
				static_cast<LONG>(texture.width()),
				static_cast<LONG>(texture.height()) });
			m_commandList->ClearRenderTargetView(
                static_cast<TextureRTVImplDX12*>(texture.m_impl)->native(),
				color.get(),
				static_cast<UINT>(rects.size()),
				rects.data());
		}

        void CommandListImplDX12::setStructureCounter(BufferUAV buffer, uint32_t value)
        {
            applyBarriers();
            static_cast<DeviceImplDX12*>(m_device->native())->uploadRawBuffer(
                this,
                buffer.buffer(),
                tools::ByteRange(
                    reinterpret_cast<uint8_t*>(&value),
                    reinterpret_cast<uint8_t*>(&value) + sizeof(uint32_t)),
                buffer.m_impl->structureCounterOffsetBytes());
        }

        void CommandListImplDX12::copyStructureCounter(BufferUAV srcBuffer, Buffer dst, uint32_t dstByteOffset)
        {
            applyBarriers();
            m_commandList->CopyBufferRegion(
                static_cast<BufferImplDX12*>(dst.m_impl)->native(),
                dstByteOffset,
                static_cast<BufferImplDX12*>(srcBuffer.buffer().m_impl)->native(),
                srcBuffer.m_impl->structureCounterOffsetBytes(), sizeof(uint32_t));
        }

        void CommandListImplDX12::draw(size_t vertexCount)
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
            m_commandList->DrawInstanced(static_cast<UINT>(vertexCount), 1, 0, 0);
        }

        void CommandListImplDX12::drawIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset)
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
            m_commandList->ExecuteIndirect(
                static_cast<DeviceImplDX12*>(m_device->native())->drawIndirectSignature(), 
                1, static_cast<BufferImplDX12*>(indirectArguments.m_impl)->native(), argumentBufferOffset, nullptr, 0);
        }

        void CommandListImplDX12::dispatch(
            size_t threadGroupCountX,
            size_t threadGroupCountY,
            size_t threadGroupCountZ)
        {
            if (!m_pipeBound)
                return;
            ASSERT(
                threadGroupCountX > 0 &&
                threadGroupCountY > 0 &&
                threadGroupCountZ > 0, "Makes no sense to launch zero dispatch. Don't do it");

            applyBarriers();
            m_commandList->Dispatch(
                static_cast<UINT>(threadGroupCountX),
                static_cast<UINT>(threadGroupCountY),
                static_cast<UINT>(threadGroupCountZ)
            );
        }

        void CommandListImplDX12::drawIndexedInstanced(
            size_t indexCount,
            size_t instanceCount,
            size_t firstIndex,
            int32_t vertexOffset,
            size_t firstInstance)
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
            m_commandList->DrawIndexedInstanced(
                static_cast<UINT>(indexCount), 
                static_cast<UINT>(instanceCount), 
                static_cast<UINT>(firstIndex), 
                vertexOffset, 
                static_cast<UINT>(firstInstance));
        }

        void CommandListImplDX12::drawIndexedIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset)
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
            m_commandList->ExecuteIndirect(
                static_cast<DeviceImplDX12*>(m_device->native())->drawIndexedIndirectSignature(), 
                1, static_cast<BufferImplDX12*>(indirectArguments.m_impl)->native(), argumentBufferOffset, nullptr, 0);
#if 0
            D3D12_INDIRECT_ARGUMENT_DESC argDesc[1];
            argDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

            D3D12_COMMAND_SIGNATURE_DESC desc;
            ZeroMemory(&desc, sizeof(D3D12_COMMAND_SIGNATURE_DESC));

            desc.pArgumentDescs = &argDesc[0];
            desc.ByteStride = sizeof(D3D12_INDIRECT_ARGUMENT_DESC);
            desc.NodeMask = 0;
            desc.NumArgumentDescs = 1;// static_cast<UINT>(m_argumentDescs.size());

                                      /*desc.ByteStride = argumentBuffer.description().descriptor.elementSize;
                                      desc.NumArgumentDescs = argumentBuffer.description().descriptor.elements;
                                      desc.pArgumentDescs = &argDesc;
                                      desc.*/

            ID3D12Resource* countBuffer = nullptr;
            UINT64 countBufferOffset = 0;
            if (indirectArguments.description().descriptor.append)
            {
                transition(indirectArguments, ResourceState::IndirectArgument);
                countBuffer = BufferImplGet::impl(indirectArguments)->native();
                countBufferOffset = roundUpToMultiple(
                    indirectArguments.description().descriptor.elements *
                    indirectArguments.description().descriptor.elementSize,
                    D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
            }

            ID3D12CommandSignature* signature;
            static_cast<DeviceImplDX12*>(m_device->native())->device()->CreateCommandSignature(
                &desc,
                m_rootSignature,
                DARKNESS_IID_PPV_ARGS(&signature));
            m_commandSignatures.emplace_back(signature);

            m_commandList->ExecuteIndirect(
                signature,                                              // ID3D12CommandSignature* pCommandSignature
                0,                                                      // UINT MaxCommandCount
                BufferImplGet::impl(indirectArguments)->native(),          // ID3D12Resource* pArgumentBuffer
                argumentBufferOffset,                                   // UINT64 ArgumentBufferOffset
                countBuffer,                                            // ID3D12Resource* pCountBuffer
                countBufferOffset                                       // UINT64 CountBufferOffset
            );
#endif
        }

        void CommandListImplDX12::drawIndexedInstancedIndirect(
            BufferIBV indexBuffer,
            Buffer indirectArguments,
            uint64_t argumentBufferOffset,
            Buffer indirectArgumentsCountBuffer,
            uint64_t countBufferOffset)
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
            m_commandList->IASetIndexBuffer(static_cast<BufferIBVImplDX12*>(indexBuffer.m_impl)->view());
            m_commandList->ExecuteIndirect(
                static_cast<DeviceImplDX12*>(m_device->native())->drawIndexedInstancedIndirectSignature(),
                static_cast<UINT>(indirectArguments.description().elements),
                static_cast<BufferImplDX12*>(indirectArguments.m_impl)->native(),
                argumentBufferOffset,
                static_cast<BufferImplDX12*>(indirectArgumentsCountBuffer.m_impl)->native(),
                countBufferOffset);
        }

        void CommandListImplDX12::executeIndexedIndirect(
            BufferIBV indexBuffer,
            Buffer indirectArguments, uint64_t argumentBufferOffset,
            Buffer countBuffer, uint64_t countBufferOffset)
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
            m_commandList->IASetIndexBuffer(static_cast<BufferIBVImplDX12*>(indexBuffer.m_impl)->view());
            m_commandList->ExecuteIndirect(
                static_cast<DeviceImplDX12*>(m_device->native())->executeIndirectClusterSignature(m_rootSignature),
                static_cast<UINT>(indirectArguments.description().elements),
                static_cast<BufferImplDX12*>(indirectArguments.m_impl)->native(),
                argumentBufferOffset,
                static_cast<BufferImplDX12*>(countBuffer.m_impl)->native(),
                countBufferOffset);
        }

        void CommandListImplDX12::dispatchIndirect(
            Buffer indirectArguments,
            uint64_t argumentBufferOffset)
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
            m_commandList->ExecuteIndirect(
                static_cast<DeviceImplDX12*>(m_device->native())->dispatchIndirectSignature(), 
                1, static_cast<BufferImplDX12*>(indirectArguments.m_impl)->native(), argumentBufferOffset, nullptr, 0);
#if 0
            /*D3D12_INDIRECT_ARGUMENT_DESC argDesc;
            argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
            m_argumentDescs.emplace_back(argDesc);*/

            D3D12_INDIRECT_ARGUMENT_DESC argDesc[1];
            argDesc[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

            D3D12_COMMAND_SIGNATURE_DESC desc;
            ZeroMemory(&desc, sizeof(D3D12_COMMAND_SIGNATURE_DESC));

            desc.pArgumentDescs = &argDesc[0];
            desc.ByteStride = sizeof(D3D12_INDIRECT_ARGUMENT_DESC);
            desc.NodeMask = 0;
            desc.NumArgumentDescs = 1;// static_cast<UINT>(m_argumentDescs.size());

            /*desc.ByteStride = argumentBuffer.description().descriptor.elementSize;
            desc.NumArgumentDescs = argumentBuffer.description().descriptor.elements;
            desc.pArgumentDescs = &argDesc;
            desc.*/

            ID3D12Resource* countBuffer = nullptr;
            UINT64 countBufferOffset = 0;
            if (indirectArguments.description().descriptor.append)
            {
                transition(indirectArguments, ResourceState::IndirectArgument);
                countBuffer = BufferImplGet::impl(indirectArguments)->native();
                countBufferOffset = roundUpToMultiple(
                    indirectArguments.description().descriptor.elements * 
                    indirectArguments.description().descriptor.elementSize, 
                    D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
            }
            
            ID3D12CommandSignature* signature;
            static_cast<DeviceImplDX12*>(m_device->native())->device()->CreateCommandSignature(
                &desc,
                m_rootSignature,
                DARKNESS_IID_PPV_ARGS(&signature));
            m_commandSignatures.emplace_back(signature);

            m_commandList->ExecuteIndirect(
                signature,                                              // ID3D12CommandSignature* pCommandSignature
                0,                                                      // UINT MaxCommandCount
                BufferImplGet::impl(indirectArguments)->native(),          // ID3D12Resource* pArgumentBuffer
                argumentBufferOffset,                                   // UINT64 ArgumentBufferOffset
                countBuffer,                                            // ID3D12Resource* pCountBuffer
                countBufferOffset                                       // UINT64 CountBufferOffset
            );
#endif
        }

        void CommandListImplDX12::dispatchMesh(
#ifdef DXR_BUILD
            size_t threadGroupCountX,
            size_t threadGroupCountY,
            size_t threadGroupCountZ
#else
            size_t,
            size_t,
            size_t
#endif
        )
        {
            if (!m_pipeBound)
                return;
            applyBarriers();
#ifdef DXR_BUILD
            m_dxrCommandList->DispatchMesh(
                threadGroupCountX,
                threadGroupCountY,
                threadGroupCountZ
            );
#endif
        }

        void CommandListImplDX12::begin()
        {
        }

        void CommandListImplDX12::end()
        {
            if (m_open)
            {
                m_commandList->Close();
                m_open = false;
            }
        }

        void CommandListImplDX12::beginRenderPass(implementation::PipelineImplIf* /*pipeline*/, int /*frameBufferIndex*/)
        {
            LOG("DX12 CommandList beginRenderPass is unimplemented");
        }

        void CommandListImplDX12::endRenderPass()
        {
            LOG("DX12 CommandList endRenderPass is unimplemented");
        }

        /*void CommandListImplDX12::transitionTexture(const Texture& image, ImageLayout from, ImageLayout to)
        {
            LOG("DX12 CommandList transitionTexture is unimplemented");
        }*/

		void CommandListImplDX12::copyTexture(TextureSRV src, TextureDSV dst)
		{
			applyBarriers();
			if (src.width() == dst.width() &&
				src.height() == dst.height())
			{
				D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
				srcLocation.pResource = static_cast<TextureImplDX12*>(src.texture().m_impl)->native();
				srcLocation.SubresourceIndex = 0;
				srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
				dstLocation.pResource = static_cast<TextureImplDX12*>(dst.texture().m_impl)->native();
				dstLocation.SubresourceIndex = 0;
				dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				D3D12_BOX box = {};
				box.left = 0;
				box.top = 0;
				box.right = static_cast<UINT>(src.width());
				box.bottom = static_cast<UINT>(src.height());
				box.front = 0;
				box.back = 1;
				m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &box);
			}
			else
			{
				D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
				srcLocation.pResource = static_cast<TextureImplDX12*>(src.texture().m_impl)->native();
				srcLocation.SubresourceIndex = 0;
				srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;


				D3D12_SUBRESOURCE_FOOTPRINT dstfprint;
				dstfprint.Depth = 1;
				dstfprint.Format = dxFormat(dst.format());
				dstfprint.Height = static_cast<UINT>(dst.height());
				dstfprint.Width = static_cast<UINT>(dst.width());
#ifndef _DURANGO
				dstfprint.RowPitch = static_cast<UINT>(roundUpToMultiple(dst.width() * formatBytes(dst.format()), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
#else
				dstfprint.RowPitch = static_cast<UINT>(roundUpToMultiple(dst.width() * formatBytes(dst.format()), D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT));
#endif
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT dstFootprint;
				dstFootprint.Footprint = dstfprint;
				dstFootprint.Offset = 0;
				D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
				dstLocation.pResource = static_cast<TextureImplDX12*>(dst.texture().m_impl)->native();
				dstLocation.PlacedFootprint = dstFootprint;
				dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;


				D3D12_BOX srcBox = {};
				srcBox.left = 0;
				srcBox.top = 0;
				srcBox.right = static_cast<UINT>(src.width());
				srcBox.bottom = static_cast<UINT>(src.height());
				srcBox.front = 0;
				srcBox.back = 1;

				UINT dstX = 0;
				UINT dstY = 0;
				UINT dstZ = 0;

				m_commandList->CopyTextureRegion(
					&dstLocation,
					dstX, dstY, dstZ,
					&srcLocation,
					&srcBox);
			}
		}

        void CommandListImplDX12::copyTexture(TextureSRV src, TextureUAV dst)
        {
            applyBarriers();
            if (src.width() == dst.width() &&
                src.height() == dst.height())
            {
                D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
                srcLocation.pResource = static_cast<TextureImplDX12*>(src.texture().m_impl)->native();
                srcLocation.SubresourceIndex = 0;
                srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
                dstLocation.pResource = static_cast<TextureImplDX12*>(dst.texture().m_impl)->native();
                dstLocation.SubresourceIndex = 0;
                dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                D3D12_BOX box = {};
                box.left = 0;
                box.top = 0;
                box.right = static_cast<UINT>(src.width());
                box.bottom = static_cast<UINT>(src.height());
                box.front = 0;
                box.back = 1;
                m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &box);
            }
            else
            {
                D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
                srcLocation.pResource = static_cast<TextureImplDX12*>(src.texture().m_impl)->native();
                srcLocation.SubresourceIndex = 0;
                srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;


                D3D12_SUBRESOURCE_FOOTPRINT dstfprint;
                dstfprint.Depth = 1;
                dstfprint.Format = dxFormat(dst.format());
                dstfprint.Height = static_cast<UINT>(dst.height());
                dstfprint.Width = static_cast<UINT>(dst.width());
#ifndef _DURANGO
                dstfprint.RowPitch = static_cast<UINT>(roundUpToMultiple(dst.width() * formatBytes(dst.format()), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
#else
				dstfprint.RowPitch = static_cast<UINT>(roundUpToMultiple(dst.width() * formatBytes(dst.format()), D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT));
#endif
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT dstFootprint;
                dstFootprint.Footprint = dstfprint;
                dstFootprint.Offset = 0;
                D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
                dstLocation.pResource = static_cast<TextureImplDX12*>(dst.texture().m_impl)->native();
                dstLocation.PlacedFootprint = dstFootprint;
                dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;


                D3D12_BOX srcBox = {};
                srcBox.left = 0;
                srcBox.top = 0;
                srcBox.right = static_cast<UINT>(src.width());
                srcBox.bottom = static_cast<UINT>(src.height());
                srcBox.front = 0;
                srcBox.back = 1;

                UINT dstX = 0;
                UINT dstY = 0;
                UINT dstZ = 0;

                m_commandList->CopyTextureRegion(
                    &dstLocation, 
                    dstX, dstY, dstZ,
                    &srcLocation, 
                    &srcBox);
            }
        }

        void CommandListImplDX12::copyTexture(TextureSRV src, TextureSRV dst)
        {
            applyBarriers();
            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = static_cast<TextureImplDX12*>(src.texture().m_impl)->native();
            srcLocation.SubresourceIndex = 0;
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = static_cast<TextureImplDX12*>(dst.texture().m_impl)->native();
            dstLocation.SubresourceIndex = 0;
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            D3D12_BOX box = {};
            box.left = 0;
            box.top = 0;
            box.right = static_cast<UINT>(src.width());
            box.bottom = static_cast<UINT>(src.height());
            box.front = 0;
            box.back = 1;
            m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &box);
        }

        void CommandListImplDX12::copyTexture(TextureSRV src, BufferUAV dst)
        {
            applyBarriers();
            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = static_cast<TextureImplDX12*>(src.texture().m_impl)->native();
            srcLocation.SubresourceIndex = 0;
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = static_cast<BufferImplDX12*>(dst.buffer().m_impl)->native();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dstLocation.PlacedFootprint.Offset = 0;
            dstLocation.PlacedFootprint.Footprint.Depth = 1;
            dstLocation.PlacedFootprint.Footprint.Format = dxFormat(dst.buffer().description().format);
            dstLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(src.width());
            dstLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(src.height());
            dstLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(dst.buffer().description().elementSize * src.width());
            m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        }

        void CommandListImplDX12::copyTexture(TextureSRV src, BufferSRV dst)
        {
            applyBarriers();
            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = static_cast<TextureImplDX12*>(src.texture().m_impl)->native();
            srcLocation.SubresourceIndex = 0;
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = static_cast<BufferImplDX12*>(dst.buffer().m_impl)->native();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dstLocation.PlacedFootprint.Offset = 0;
            dstLocation.PlacedFootprint.Footprint.Depth = 1;
            dstLocation.PlacedFootprint.Footprint.Format = dxFormat(dst.buffer().description().format);
            dstLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(src.width());
            dstLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(src.height());
            dstLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(dst.buffer().description().elementSize * roundUpToMultiple(src.width(), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
            m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        }

		void CommandListImplDX12::copyTexture(
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
			applyBarriers();

			// setup source
			D3D12_SUBRESOURCE_FOOTPRINT  srcFootprint;
			srcFootprint.Depth = 1;
			srcFootprint.Format = dxFormat(dst.format());
			srcFootprint.Width = static_cast<UINT>(srcWidth);
			srcFootprint.Height = static_cast<UINT>(srcHeight);
			srcFootprint.RowPitch = static_cast<UINT>(srcRowPitch);

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D = { 0 };
			placedTexture2D.Offset = srcOffset;
			placedTexture2D.Footprint = srcFootprint;

			D3D12_TEXTURE_COPY_LOCATION srcLocation;
			srcLocation.pResource = static_cast<BufferImplDX12*>(srcBuffer.m_impl)->native();
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcLocation.PlacedFootprint = placedTexture2D;

			D3D12_BOX srcBox;
			srcBox.left = 0;
			srcBox.top = 0;
			srcBox.right = static_cast<UINT>(srcWidth);
			srcBox.bottom = static_cast<UINT>(srcHeight);
			srcBox.front = 0;
			srcBox.back = 1;

			// setup destination
			auto subResourceIndex = dstMip + (dstSlice * dstMipCount);
			D3D12_TEXTURE_COPY_LOCATION dstLocation;
			dstLocation.pResource = static_cast<TextureImplDX12*>(dst.m_impl)->native();
			dstLocation.SubresourceIndex = static_cast<UINT>(subResourceIndex);
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			m_commandList->CopyTextureRegion(
				&dstLocation,
                static_cast<UINT>(dstX), static_cast<UINT>(dstY), 0u,
				&srcLocation,
				&srcBox);
		}

        ID3D12GraphicsCommandList* CommandListImplDX12::native()
        {
            return m_commandList.Get();
        }

        ID3D12GraphicsCommandList* CommandListImplDX12::native() const
        {
            return m_commandList.Get();
        }

#ifdef DXR_BUILD
        ID3D12GraphicsCommandList6* CommandListImplDX12::dxrCommandlist() const
		{
			return m_dxrCommandList.Get();
		}
#endif

        bool CommandListImplDX12::isOpen() const
        {
            return m_open;
        }
    }
}
