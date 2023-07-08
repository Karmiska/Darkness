#include "engine/graphics/dx12/DX12ShaderBinary.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/Device.h"
#include "engine/filesystem/VirtualFilesystem.h"
#include "tools/Recompile.h"
#include "tools/StringTools.h"
#include "tools/PathTools.h"
#include "tools/Debug.h"

#include <iostream>
#include <fstream>

using namespace engine;

namespace engine
{
    namespace implementation
    {
        ShaderBinaryImplDX12::ShaderBinaryImplDX12(
            const Device&, 
            const engine::string& binaryPath, 
            const engine::string& supportPath, 
            int permutationId,
            const engine::vector<engine::string>& defines,
            platform::FileWatcher& watcher)
            : m_buffer{ nullptr }
            , m_shaderBinary{ new D3D12_SHADER_BYTECODE() }
            , m_shaderSupport{ supportPath }
            , m_watchHandle{ watcher.addWatch(m_shaderSupport.file, [this](const engine::string& changedPath)->engine::string { return this->onFileChange(changedPath); }) }
            , m_change{}
            , m_permutationId{ permutationId }
            , m_defines{ defines }
            , m_includeDependenciesIndex{ 0ull }
        {
            // add shader includes to watch
            for (auto&& depend : m_shaderSupport.includeFiles)
            {
                m_includeDependencies[m_includeDependenciesIndex] = watcher.addWatch(
                    depend,
                    [this](const engine::string& /*changedPath*/)->engine::string
                    {
                        return this->onFileChange(m_shaderSupport.file);
                    });
                ++m_includeDependenciesIndex;
            }

            // read binary
            readFile(binaryPath);
        }

        engine::string ShaderBinaryImplDX12::onFileChange(const engine::string& /*path*/)
        {
            auto res = recompile(m_shaderSupport, m_permutationId, m_defines);
            auto binFile = permutationName(m_shaderSupport.binaryFile, m_permutationId);

            readFile(binFile);
            for (auto&& change : m_change)
            {
                change.second();
            }
            return res;
        }

        void ShaderBinaryImplDX12::registerForChange(void* client, std::function<void(void)> change) const
        {
            m_change[client] = change;
        }

        void ShaderBinaryImplDX12::unregisterForChange(void* client) const
        {
            m_change.erase(client);
        }

        void ShaderBinaryImplDX12::readFile(const engine::string& path)
        {
            ASSERT(path.data());

			auto resolvedPath = resolvePath(path);

            std::ifstream shaderFile;
            shaderFile.open(resolvedPath.c_str(), std::ios::in | std::ios::binary);
            if (shaderFile.is_open())
            {
                auto begin = shaderFile.tellg();
                shaderFile.seekg(0, std::ios::end);
                auto end = shaderFile.tellg();
                size_t size = static_cast<size_t>(end - begin);
                m_buffer.reset(new char[size]);
                m_shaderBinary->pShaderBytecode = m_buffer.get();
                m_shaderBinary->BytecodeLength = size;
                shaderFile.seekg(0, std::ios::beg);
                shaderFile.read(m_buffer.get(), end - begin);
                shaderFile.close();
            }
        }

        ShaderBinaryImplDX12::~ShaderBinaryImplDX12()
        {
            if (m_shaderBinary)
            {
                delete m_shaderBinary;
                m_shaderBinary = nullptr;
            }
        }

        D3D12_SHADER_BYTECODE& ShaderBinaryImplDX12::native() const
        {
            return *m_shaderBinary;
        }

        ShaderBinaryImplDX12::ShaderBinaryImplDX12(const ShaderBinaryImplDX12& shaderBinary)
            : m_buffer{ nullptr }
            , m_shaderBinary{ new D3D12_SHADER_BYTECODE() }
        {
            if (shaderBinary.m_buffer)
            {
                m_buffer.reset(new char[shaderBinary.m_shaderBinary->BytecodeLength]);
                memcpy(m_buffer.get(), shaderBinary.m_buffer.get(), shaderBinary.m_shaderBinary->BytecodeLength);
                m_shaderBinary->BytecodeLength = shaderBinary.m_shaderBinary->BytecodeLength;
                m_shaderBinary->pShaderBytecode = m_buffer.get();
            }
        }

        ShaderBinaryImplDX12::ShaderBinaryImplDX12(ShaderBinaryImplDX12&& shaderBinary)
            : m_buffer{ nullptr }
            , m_shaderBinary{ new D3D12_SHADER_BYTECODE() }
        {
            m_buffer.swap(shaderBinary.m_buffer);
            m_shaderBinary->BytecodeLength = shaderBinary.m_shaderBinary->BytecodeLength;
            m_shaderBinary->pShaderBytecode = m_buffer.get();
            shaderBinary.m_shaderBinary->BytecodeLength = 0;
        }

        void ShaderBinaryImplDX12::clear()
        {
            m_buffer.reset(nullptr);
            m_shaderBinary->BytecodeLength = 0;
            m_shaderBinary->pShaderBytecode = nullptr;
        }

        ShaderBinaryImplDX12& ShaderBinaryImplDX12::operator=(const ShaderBinaryImplDX12& shaderBinary)
        {
            clear();

            if (shaderBinary.m_buffer)
            {
                m_buffer.reset(new char[shaderBinary.m_shaderBinary->BytecodeLength]);
                memcpy(m_buffer.get(), shaderBinary.m_buffer.get(), shaderBinary.m_shaderBinary->BytecodeLength);
                m_shaderBinary->BytecodeLength = shaderBinary.m_shaderBinary->BytecodeLength;
                m_shaderBinary->pShaderBytecode = m_buffer ? m_buffer.get() : nullptr;
            }

            return *this;
        }

        ShaderBinaryImplDX12& ShaderBinaryImplDX12::operator=(ShaderBinaryImplDX12&& shaderBinary)
        {
            clear();
            m_buffer.swap(shaderBinary.m_buffer);
            m_shaderBinary->BytecodeLength = shaderBinary.m_shaderBinary->BytecodeLength;
            m_shaderBinary->pShaderBytecode = m_buffer ? m_buffer.get() : nullptr;
            shaderBinary.m_shaderBinary->BytecodeLength = 0;

            return *this;
        }
    }
}
