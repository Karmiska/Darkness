#include "engine/graphics/metal/MetalShaderBinary.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "tools/Debug.h"

#include <iostream>
#include <fstream>

using namespace std;

namespace engine
{
    namespace implementation
    {
        ShaderBinaryImpl::ShaderBinaryImpl(const Device& device,
                                           const std::string& binaryPath)
            : m_buffer{ nullptr }
            , m_shaderBinary{ std::make_shared<METAL_SHADER_BYTECODE>() }
        {
            readFile(binaryPath);
        }

        void ShaderBinaryImpl::readFile(const std::string& path)
        {
            const char* temp = path.data();
            ASSERT(temp);
            ifstream shaderFile;
            shaderFile.open(path, ios::in | ios::binary);
            if (shaderFile.is_open())
            {
                auto begin = shaderFile.tellg();
                shaderFile.seekg(0, ios::end);
                auto end = shaderFile.tellg();
                size_t size = static_cast<size_t>(end - begin);
                m_buffer.reset(new char[size]);
                //m_shaderBinary->pShaderBytecode = m_buffer.get();
                //m_shaderBinary->BytecodeLength = size;
                shaderFile.seekg(0, ios::beg);
                shaderFile.read(m_buffer.get(), end - begin);
                shaderFile.close();
            }
        }

        METAL_SHADER_BYTECODE& ShaderBinaryImpl::native() const
        {
            return *m_shaderBinary;
        }
    }
}
