#pragma once

#include "containers/string.h"

namespace shadercompiler
{
    engine::string className(const engine::string& path);
    engine::string stageName(const engine::string& path);
    engine::string interfacePath(const engine::string& path);
    engine::string pipelineNameFromFilename(const engine::string& path);
}
