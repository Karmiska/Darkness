#include "engine/graphics/ShaderStorage.h"
#include "tools/Debug.h"
#include "engine/graphics/CommonNoDep.h"

namespace engine
{
    engine::shared_ptr<const ShaderBinary> ShaderStorage::loadShader(
        const Device& device, 
        const engine::string& path,
        const engine::string& supportPath,
        int permutationId,
        const engine::vector<engine::string>& defines,
        GraphicsApi api)
    {
        auto loaded = m_loaded.find(path);
        if (loaded != m_loaded.end())
            return m_loaded[path];

        m_loaded[path] = engine::make_shared<const ShaderBinary>(device, path, supportPath, permutationId, defines, m_fileWatcher, api);

        return m_loaded[path];
    }

    void ShaderStorage::clear()
    {
        m_loaded.clear();
    }

    platform::FileWatcher& ShaderStorage::fileWatcher()
    {
        return m_fileWatcher;
    }
}
