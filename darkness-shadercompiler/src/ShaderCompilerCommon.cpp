#include "ShaderCompilerCommon.h"
#include "hash/sha1.h"

namespace shadercompiler
{
    engine::string sha1(const engine::string& seed)
    {
        Chocobo1::SHA1 sha1;
        sha1.addData(seed.data(), seed.length());
        sha1.finalize();
        return sha1.toString();
    }

    engine::string getShaderBinaryPath(const engine::string& hlslSourcePath)
    {
        return "";
    }

    engine::string getShaderLoadInterfaceHeaderPath()
    {
        return "";
    }
}
