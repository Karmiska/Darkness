#pragma once

#include "engine/graphics/ShaderBinaryImplIf.h"
#include "platform/FileWatcher.h"
#include "containers/string.h"

namespace engine
{
    class Device;
    enum class GraphicsApi;

    class ShaderBinary
    {
    public:
        ShaderBinary(
            const Device& device, 
            const engine::string& binaryPath, 
            const engine::string& supportPath,
            int permutationId,
            const engine::vector<engine::string>& defines,
            platform::FileWatcher& watcher,
            GraphicsApi api);

        void registerForChange(void* client, std::function<void(void)> change) const;
        void unregisterForChange(void* client) const;

        implementation::ShaderBinaryImplIf* native() { return m_impl.get(); }
        const implementation::ShaderBinaryImplIf* native() const { return m_impl.get(); }

        int permutationId() const
        {
            return m_permutationId;
        }

    private:
        engine::unique_ptr<implementation::ShaderBinaryImplIf> m_impl;
        int m_permutationId;
    };
}
