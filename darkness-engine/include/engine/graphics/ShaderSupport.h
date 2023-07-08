#pragma once

#include "containers/string.h"
#include "containers/vector.h"

namespace engine
{
    namespace implementation
    {
        class ShaderSupport
        {
        public:
            ShaderSupport() = default;
            ShaderSupport(const engine::string& supportFilePath);

            engine::string binaryFile;
            engine::string executable;
            engine::string file;
            engine::string graphicsApi;
            engine::string shaderCompilerPath;
            engine::string rootPath;
            engine::string sourceFile;
            engine::vector<engine::string> includeFiles;
        };
    }
}
