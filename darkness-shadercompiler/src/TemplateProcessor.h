#pragma once

#include "containers/vector.h"
#include "containers/string.h"
#include "CompileSharedTypes.h"

namespace shadercompiler
{
    struct ShaderPipelineStage;
    struct ShaderPipelineConfiguration;
    class ShaderLocator;
    class TemplateProcessor
    {
    public:
        static void ProcessShaderLoadInterfaces(
            ShaderLocator& locator,
            const engine::string& originalPath,
            const engine::string& templatePath, 
            const engine::string& outputPath, 
            ShaderPipelineStage& stage,
            LogLevel logLevel);

        static void ProcessPipelineInterfaces(
            const engine::string& templatePath,
            const ShaderPipelineConfiguration& pipeline,
            const engine::string& targetPath,
            LogLevel logLevel);
    };
}
