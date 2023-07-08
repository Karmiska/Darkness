#pragma once

#include <fstream>
#include "containers/unordered_map.h"
#include "containers/string.h"
#include "CompileSharedTypes.h"

namespace shadercompiler
{
    class ShaderLocator;
    struct ShaderPipelineStage;
    class CompileTask
    {
    public:
        CompileTask(ShaderLocator& locator, bool optimization, LogLevel logLevel);

    private:
        void preprocessAndGetResources(ShaderLocator& locator);
        void writeRootSignatures(ShaderLocator& locator) const;
        void writeRootSignature(std::ofstream& stream, const ShaderPipelineStage& stage) const;
        void compileRootSignature(const engine::string& rs, const engine::string& rso) const;
        void updateResourceRegisters(ShaderLocator& locator);
        void compile(ShaderLocator& locator, bool optimization, GraphicsApi api = GraphicsApi::DX12);
        void patchVulkanRegisters(ShaderLocator& locator);
        void createCodeInterfaces(ShaderLocator& locator);

        LogLevel m_logLevel;
        engine::unordered_map<engine::string, engine::string> m_shaderProfiles;

        bool doLog(LogLevel level) const;
    };
}
