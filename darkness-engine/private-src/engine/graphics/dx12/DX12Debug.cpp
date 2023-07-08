#include "engine/graphics/dx12/DX12Debug.h"
#include "engine/graphics/dx12/DX12Device.h"
#ifndef _DURANGO
#include <D3D12SDKLayers.h>
#endif
#include <map>
#include "tools/ComPtr.h"

using namespace engine;
using namespace tools;

#undef GPU_DEBUG

namespace engine
{
    namespace implementation
    {
#ifdef GPU_DEBUG
#ifndef _DURANGO
        static unordered_map<ID3D12Device*, ComPtr<ID3D12DebugDevice>> debugDevices;
#ifdef GPU_DEBUG
        static GraphicsDebug debugInitializer;
#endif
#endif
#endif

        GraphicsDebug::GraphicsDebug()
        {
#ifndef _DURANGO
#ifdef GPU_DEBUG
            auto res = D3D12GetDebugInterface(DARKNESS_IID_PPV_ARGS(m_dbgInterface.GetAddressOf()));
            ASSERT(SUCCEEDED(res));
            m_dbgInterface->EnableDebugLayer();

            m_debug1 = m_dbgInterface.As<ID3D12Debug1>();
            m_debug1->SetEnableSynchronizedCommandQueueValidation(TRUE);
            m_debug1->SetEnableGPUBasedValidation(TRUE);
#endif
#endif
        }

        GraphicsDebug::~GraphicsDebug()
        {
#ifndef _DURANGO
#ifdef GPU_DEBUG
            for (auto& pair : debugDevices)
            {
                pair.second->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
            }
#endif
#endif
        }

        void GraphicsDebug::addDevice(ID3D12Device* 
#if defined(GPU_DEBUG) && !defined(_DURANGO)
            device
#endif
        )
        {
#ifndef _DURANGO
#ifdef GPU_DEBUG
            if (debugDevices.find(device) == debugDevices.end())
            {
                ComPtr<ID3D12DebugDevice> debugDevice;
                auto res = device->QueryInterface(debugDevice.GetAddressOf());
                ASSERT(SUCCEEDED(res));
                debugDevices[device].Swap(debugDevice);
            }
#endif
#endif
        }
    }
}
