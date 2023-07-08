#pragma once

#include "engine/graphics/ShaderBinaryImplIf.h"
#include "containers/memory.h"
#include "containers/string.h"
#include "containers/unordered_map.h"
#include "platform/FileWatcher.h"
#include "engine/graphics/ShaderSupport.h"

struct D3D12_SHADER_BYTECODE;

namespace engine
{
    class Device;

    namespace implementation
    {
        class ShaderBinaryImplDX12 : public ShaderBinaryImplIf
        {
        public:
            ShaderBinaryImplDX12(
                const Device& device, 
                const engine::string& binaryPath, 
                const engine::string& supportPath,
                int permutationId,
                const engine::vector<engine::string>& defines,
                platform::FileWatcher& watcher);
            ~ShaderBinaryImplDX12();

            ShaderBinaryImplDX12(const ShaderBinaryImplDX12&);
            ShaderBinaryImplDX12(ShaderBinaryImplDX12&&);
            ShaderBinaryImplDX12& operator=(const ShaderBinaryImplDX12&);
            ShaderBinaryImplDX12& operator=(ShaderBinaryImplDX12&&);

            void registerForChange(void* client, std::function<void(void)> change) const override;
            void unregisterForChange(void* client) const override;

            D3D12_SHADER_BYTECODE& native() const;

            // for debug
            const engine::string& supportPath() const { return m_shaderSupport.file; }
        private:
            engine::unique_ptr<char[]> m_buffer;
            D3D12_SHADER_BYTECODE* m_shaderBinary;
            int m_permutationId;
            engine::vector<engine::string> m_defines;
            
            ShaderSupport m_shaderSupport;
            platform::WatchHandle m_watchHandle;
            platform::WatchHandle m_includeDependencies[128];
            size_t m_includeDependenciesIndex;
            mutable engine::unordered_map<void*, std::function<void(void)>> m_change;

            engine::string onFileChange(const engine::string& path);
            void readFile(const engine::string& path);
            void clear();
        };
    }
}

