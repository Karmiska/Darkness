#include "engine/graphics/dx12/DX12CpuMarker.h"


#include "engine/graphics/dx12/DX12Common.h"
#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Common.h"
#include <WinPixEventRuntime/pix3.h>

#include <inttypes.h>


namespace engine
{
    namespace implementation
    {
        CpuMarkerImplDX12::CpuMarkerImplDX12(const char* msg)
        {
            PIXBeginEvent(0, msg);
        }

        CpuMarkerImplDX12::~CpuMarkerImplDX12()
        {
            PIXEndEvent();
        }
    }
}
