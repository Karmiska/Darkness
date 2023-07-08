#pragma once

#include "containers/memory.h"
#include "containers/string.h"

struct METAL_SHADER_BYTECODE {};

namespace engine
{
    class Device;
    namespace implementation
    {
        class ShaderBinaryImpl
        {
        public:
            ShaderBinaryImpl(
                             const Device& device,
                             const engine::string& binaryPath);
            
            METAL_SHADER_BYTECODE& native() const;
        private:
            engine::unique_ptr<char> m_buffer;
            engine::shared_ptr<METAL_SHADER_BYTECODE> m_shaderBinary;
            
            void readFile(const engine::string& path);
            void clear();
        };
    }
}

