#pragma once

#include "tools/ComPtr.h"
#include "engine/graphics/dx12/DX12Headers.h"

struct ID3D12Device;

namespace engine
{
    namespace implementation
    {
        class GraphicsDebug
        {
        public:
            GraphicsDebug();
            ~GraphicsDebug();
            static void addDevice(ID3D12Device* device);
        private:
            tools::ComPtr<ID3D12Debug> m_dbgInterface;
#ifndef _DURANGO
            tools::ComPtr<ID3D12Debug1> m_debug1;
#endif
        };
    }
}
