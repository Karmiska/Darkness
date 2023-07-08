#include "engine/graphics/dx12/DX12Queue.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/dx12/DX12Fence.h"
#include "engine/graphics/dx12/DX12Semaphore.h"
#include "engine/graphics/dx12/DX12Conversions.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12SwapChain.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Semaphore.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        QueueImplDX12::QueueImplDX12(Device& device, CommandListType type, const char* queueName)
            : m_device{ device }
            , m_waitForClearFence{ engine::make_shared<Fence>(device.createFence("Queue wait for clear fence")) }
            , m_queueName{ queueName }
        {
            //m_waitCmdList.

            D3D12_COMMAND_QUEUE_DESC commandQueueDesc;

            // Initialize the description of the command queue.
            ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));

            // Set up the description of the command queue.
            commandQueueDesc.Type = dxCommandListType(type);
            commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            commandQueueDesc.NodeMask = 0;

            // Create the command queue.
            auto res = static_cast<DeviceImplDX12*>(device.native())->device()->CreateCommandQueue(
                &commandQueueDesc, 
                DARKNESS_IID_PPV_ARGS(&m_queue));
            ASSERT(SUCCEEDED(res));

            static WCHAR resourceName[1024] = {};
            size_t numCharacters;
            if (queueName)
                mbstowcs_s(&numCharacters, resourceName, queueName, 1024);
            m_queue->SetName(resourceName);

			if (type == CommandListType::Direct)
			{
				auto qres = m_queue->GetTimestampFrequency(&m_timeStampFrequency);
				ASSERT(SUCCEEDED(qres));
			}
        }

        QueueImplDX12::~QueueImplDX12()
        {
            if (m_queue)
            {
                m_queue->Release();
                m_queue = NULL;
            }
        }

        ID3D12CommandQueue* QueueImplDX12::native()
        {
            return m_queue;
        }

        void QueueImplDX12::submit(engine::vector<CommandList>& commandLists)
        {
            engine::vector<ID3D12CommandList*> cmdLists;
            for (auto&& cmdList : commandLists)
            {
                auto cmd = static_cast<CommandListImplDX12*>(cmdList.native());
                if (cmd->isOpen())
                {
                    cmd->applyBarriers();
                    cmd->resolveQueries();
                    cmd->end();
                }
                cmdLists.emplace_back( cmd->native() );
            }
            m_queue->ExecuteCommandLists(static_cast<UINT>(cmdLists.size()), cmdLists.data());
        }

        void QueueImplDX12::submit(CommandList& commandList)
        {
            auto cmd = static_cast<CommandListImplDX12*>(commandList.native());
            if (cmd->isOpen())
            {
                cmd->applyBarriers();
                cmd->resolveQueries();
                cmd->end();
            }
            ID3D12CommandList* cmdLists[] = { cmd->native() };
            m_queue->ExecuteCommandLists(1, cmdLists);
        }

        void QueueImplDX12::submit(engine::vector<CommandList>& commandLists, Fence& fence)
        {
            engine::vector<ID3D12CommandList*> cmdLists;
            for (auto&& cmdList : commandLists)
            {
                auto cmd = static_cast<CommandListImplDX12*>(cmdList.native());
                if (cmd->isOpen())
                {
                    cmd->applyBarriers();
                    cmd->resolveQueries();
                    cmd->end();
                }
                cmdLists.emplace_back(cmd->native());
            }
            m_queue->ExecuteCommandLists(static_cast<UINT>(cmdLists.size()), cmdLists.data());
            m_queue->Signal(static_cast<FenceImplDX12*>(fence.native())->native(), static_cast<FenceImplDX12*>(fence.native())->currentCPUValue());
        }
        void QueueImplDX12::submit(CommandList& commandList, Fence& fence)
        {
            auto cmd = static_cast<CommandListImplDX12*>(commandList.native());
            if (cmd->isOpen())
            {
                cmd->applyBarriers();
                cmd->resolveQueries();
                cmd->end();
            }
            ID3D12CommandList* cmdLists[] = { cmd->native() };
            m_queue->ExecuteCommandLists(1, cmdLists);
            m_queue->Signal(static_cast<FenceImplDX12*>(fence.native())->native(), static_cast<FenceImplDX12*>(fence.native())->currentCPUValue());
        }

		void QueueImplDX12::submit(engine::vector<CommandList>& commandLists, Semaphore& /*semaphore*/)
		{
			engine::vector<ID3D12CommandList*> cmdLists;
			for (auto&& cmdList : commandLists)
			{
                auto cmd = static_cast<CommandListImplDX12*>(cmdList.native());
                if (cmd->isOpen())
                {
                    cmd->applyBarriers();
                    cmd->resolveQueries();
                    cmd->end();
                }
				cmdLists.emplace_back(cmd->native());
			}
			m_queue->ExecuteCommandLists(static_cast<UINT>(cmdLists.size()), cmdLists.data());
		}
		void QueueImplDX12::submit(CommandList& commandList, Semaphore& /*semaphore*/)
		{
            auto cmd = static_cast<CommandListImplDX12*>(commandList.native());
            if (cmd->isOpen())
            {
                cmd->applyBarriers();
                cmd->resolveQueries();
                cmd->end();
            }
			ID3D12CommandList* cmdLists[] = { cmd->native() };
			m_queue->ExecuteCommandLists(1, cmdLists);
		}
		void QueueImplDX12::submit(CommandList& commandList, Semaphore& /*waitSemaphore*/, Semaphore& /*signalSemaphore*/)
		{
            auto cmd = static_cast<CommandListImplDX12*>(commandList.native());
            if (cmd->isOpen())
            {
                cmd->applyBarriers();
                cmd->resolveQueries();
                cmd->end();
            }
			ID3D12CommandList* cmdLists[] = { cmd->native() };
			m_queue->ExecuteCommandLists(1, cmdLists);
		}
		void QueueImplDX12::submit(CommandList& commandList, Semaphore& /*waitSemaphore*/, Semaphore& /*signalSemaphore*/, Fence& /*fence*/)
		{
            auto cmd = static_cast<CommandListImplDX12*>(commandList.native());
            if (cmd->isOpen())
            {
                cmd->applyBarriers();
                cmd->resolveQueries();
                cmd->end();
            }
			ID3D12CommandList* cmdLists[] = { cmd->native() };
			m_queue->ExecuteCommandLists(1, cmdLists);
		}
		void QueueImplDX12::submit(CommandList& commandList, Semaphore& /*semaphore*/, Fence& /*fence*/)
		{
            auto cmd = static_cast<CommandListImplDX12*>(commandList.native());
            if (cmd->isOpen())
            {
                cmd->applyBarriers();
                cmd->resolveQueries();
                cmd->end();
            }
			ID3D12CommandList* cmdLists[] = { cmd->native() };
			m_queue->ExecuteCommandLists(1, cmdLists);
		}

        void QueueImplDX12::signal(const Fence& fence, unsigned long long value)
        {
            m_queue->Signal(static_cast<const FenceImplDX12*>(fence.native())->native(), value);
        }

        void QueueImplDX12::waitForIdle() const
        {
            m_waitForClearFence->increaseCPUValue();
            m_queue->Signal(static_cast<FenceImplDX12*>(m_waitForClearFence->native())->native(), m_waitForClearFence->currentCPUValue());
            m_waitForClearFence->blockUntilSignaled();
        }

        void QueueImplDX12::signal(const Semaphore& semaphore)
        {
            static_cast<const SemaphoreImplDX12*>(semaphore.native())->native()->Signal(1);
        }

        void QueueImplDX12::present(
            Semaphore& /*signalSemaphore*/,
            SwapChain& swapChain,
            unsigned int /*chainIndex*/)
        {
            static_cast<SwapChainImplDX12*>(swapChain.native())->present();
        }

        bool QueueImplDX12::needRefresh() const
        {
            return false;
        }
    }
}
