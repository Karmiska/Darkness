#include "engine/graphics/dx12/DX12AfterMath.h"
#include "engine/graphics/dx12/DX12GpuMarker.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/CommandList.h"
#include <WinPixEventRuntime/pix3.h>

namespace engine
{
    namespace implementation
    {
        GpuMarkerImplDX12::GpuMarkerImplDX12(CommandList& cmd, const char* msg)
            : cmdList{ static_cast<CommandListImplDX12*>(cmd.native()) }
        {
            PIXBeginEvent(cmdList->native(), 0, msg);
			if(cmd.m_impl->type() == CommandListType::Direct)
				m_queryId = cmdList->startQuery(msg);

#ifdef AFTERMATH_ENABLED
            auto afterMathResult = GFSDK_Aftermath_SetEventMarker(*cmdList.afterMathContextHandle(), msg, strlen(msg));
            ASSERT(afterMathResult == GFSDK_Aftermath_Result_Success, "Aftermath failed to set an event marker");
#endif
        }

        GpuMarkerImplDX12::~GpuMarkerImplDX12()
        {
			if (cmdList->type() == CommandListType::Direct)
				cmdList->stopQuery(m_queryId);
            PIXEndEvent(cmdList->native());
        }
    }
}