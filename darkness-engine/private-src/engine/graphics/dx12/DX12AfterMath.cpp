#include "engine/graphics/dx12/DX12AfterMath.h"

#ifndef _DURANGO
AfterMathContextContainer afterMathContext;

AfterMathContextContainer::AfterMathContextContainer()
{
    // this is a hack. we want to avoid reallocating the vector
    // memory as we're using pointers that would invalidate
    m_contextHandles.reserve(10000);
}

AfterMathHandle AfterMathContextContainer::allocate()
{
    m_contextHandles.emplace_back(GFSDK_Aftermath_ContextHandle{});
    return &m_contextHandles.back();
}

void AfterMathContextContainer::free(AfterMathHandle handle)
{
    auto loc = std::find(m_contextHandles.begin(), m_contextHandles.end(), *handle);
    if (loc != m_contextHandles.end())
        m_contextHandles.erase(loc);
}
#endif
