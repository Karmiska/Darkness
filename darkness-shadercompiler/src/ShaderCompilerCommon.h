#pragma once

#include "containers/string.h"

namespace shadercompiler
{
    engine::string sha1(const engine::string& seed);

    engine::string getShaderBinaryPath(const engine::string& hlslSourcePath);
    engine::string getShaderLoadInterfaceHeaderPath();
}