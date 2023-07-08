#pragma once

#include "containers/string.h"

namespace engine
{
    bool fileExists(const engine::string& filename);
    bool fileCopy(const engine::string& src, const engine::string& dst);
    bool fileDelete(const engine::string& file);
    void fileRename(const engine::string& from, const engine::string& to);
}
