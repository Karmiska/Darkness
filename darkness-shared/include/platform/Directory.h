#pragma once

#include "containers/string.h"
#include "containers/vector.h"

namespace engine
{
    class Directory
    {
        engine::string m_path;
    public:
        Directory(const engine::string& path);
        const engine::string& path() const;
        bool exists() const;
        void create() const;
        void remove(bool removeFiles = false) const;
        engine::vector<engine::string> files() const;
        engine::vector<engine::string> directories() const;
    };
}
