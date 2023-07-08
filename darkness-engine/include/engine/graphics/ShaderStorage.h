#pragma once

#include "ShaderBinary.h"
#include "platform/FileWatcher.h"
#include "containers/memory.h"
#include <map>
#include "containers/string.h"
#include "engine/graphics/CommonNoDep.h"

namespace engine
{
    
    class Device;
    class ShaderStorage
    {
    public:
        engine::shared_ptr<const ShaderBinary> loadShader(
            const Device& device, 
            const engine::string& path,
            const engine::string& supportPath,
            int permutationId,
            const engine::vector<engine::string>& defines,
            GraphicsApi api = GraphicsApi::DX12);
        void clear();

        platform::FileWatcher& fileWatcher();
    private:
        std::map<const engine::string, engine::shared_ptr<const ShaderBinary>> m_loaded;
        platform::FileWatcher m_fileWatcher;
    };
}
