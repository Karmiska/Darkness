#pragma once

namespace shadercompiler
{
    enum class GraphicsApi
    {
        DX12,
        Vulkan
    };

    enum class LogLevel
    {
        None,
        Error,
        Recompile,
        Progress,
        All
    };
}
