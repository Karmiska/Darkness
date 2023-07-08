#include "platform/Directory.h"

using namespace engine;
using namespace std;

#ifndef __APPLE__
#include <filesystem>

using namespace std;

namespace engine
{
    Directory::Directory(const string& path)
        : m_path{ path }
    {
    }

    const engine::string& Directory::path() const
    {
        return m_path;
    }

    bool Directory::exists() const
    {
        return filesystem::exists(m_path.c_str());
    }

    void Directory::create() const
    {
        if (!exists())
            filesystem::create_directories(m_path.c_str());
    }

    void Directory::remove(bool removeFiles) const
    {
        if (!removeFiles)
            filesystem::remove(m_path.c_str());
        else
            filesystem::remove_all(m_path.c_str());
    }

    vector<string> Directory::files() const
    {
        vector<string> result;
        for (const auto& file : filesystem::directory_iterator{ m_path.c_str() })
        {
            if (filesystem::is_regular_file(file.status()))
            {
                result.emplace_back(reinterpret_cast<const char*>(file.path().filename().u8string().c_str()));
            }
        }
        return result;
    }

    engine::vector<engine::string> Directory::directories() const
    {
        vector<string> result;
        for (const auto& file : filesystem::directory_iterator{ m_path.c_str() })
        {
            if (filesystem::is_directory(file.status()))
            {
                result.emplace_back(reinterpret_cast<const char*>(file.path().filename().u8string().c_str()));
            }
        }
        return result;
    }
}
#else
namespace engine
{
    Directory::Directory(const string& path)
    : m_path{ path }
    {
        ASSERT(false, "TODO: Need to implement Directory support for this platform");
    }
    
    bool Directory::exists() const
    {
        return false;
    }
    
    void Directory::create() const
    {
    }
    
    void Directory::remove(bool removeFiles) const
    {
    }
    
    vector<string> Directory::files() const
    {
        return {};
    }
}
#endif
