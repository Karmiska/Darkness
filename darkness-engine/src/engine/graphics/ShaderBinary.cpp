#include "engine/graphics/ShaderBinary.h"
#include "engine/graphics/CommonNoDep.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12ShaderBinary.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanShaderBinary.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalShaderBinary.h"
#endif

using namespace engine::implementation;

namespace engine
{
    ShaderBinary::ShaderBinary(
        const Device& device, 
        const engine::string& binaryPath, 
        const engine::string& supportPath,
        int permutationId,
        const engine::vector<engine::string>& defines,
        platform::FileWatcher& watcher,
        GraphicsApi api)
        : m_impl{}
        , m_permutationId{ permutationId }
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<ShaderBinaryImplDX12>(device, binaryPath, supportPath, permutationId, defines, watcher);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<ShaderBinaryImplVulkan>(device, binaryPath, supportPath, permutationId, defines, watcher);
    }

    void ShaderBinary::registerForChange(void* client, std::function<void(void)> change) const
    {
        m_impl->registerForChange(client, change);
    }

    void ShaderBinary::unregisterForChange(void* client) const
    {
        m_impl->unregisterForChange(client);
    }
}
