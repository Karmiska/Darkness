#pragma once

#ifndef _DURANGO
#include <cstdint>
#include "platform/Platform.h"
#include "engine/graphics/dx12/DX12Headers.h"
#ifndef __d3d12_h__
#define __d3d12_h__
#include "GFSDK_Aftermath.h"
#undef __d3d12_h__
#else
#include "GFSDK_Aftermath.h"
#endif

#undef AFTERMATH_ENABLED

#include "containers/vector.h"

using AfterMathHandle = GFSDK_Aftermath_ContextHandle*;

class AfterMathContextContainer
{
public:
    AfterMathContextContainer();

    AfterMathHandle allocate();
    void free(AfterMathHandle handle);

    engine::vector<GFSDK_Aftermath_ContextHandle>& contextHandles()
    {
        return m_contextHandles;
    }
private:
    engine::vector<GFSDK_Aftermath_ContextHandle> m_contextHandles;
};

extern AfterMathContextContainer afterMathContext;
#endif
