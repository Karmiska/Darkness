#include "platform/File.h"
#include <fstream>
#include <filesystem>

using namespace std;

namespace engine
{
    bool fileExists(const engine::string& filename)
    {
        std::ifstream file(filename.c_str());
        return file.good();
    }

    bool fileCopy(const engine::string& src, const engine::string& dst, bool overWriteIfExists)
    {

        std::filesystem::copy(src.c_str(), dst.c_str(), 
            overWriteIfExists ?
            std::filesystem::copy_options::overwrite_existing :
            std::filesystem::copy_options::none);
        return true;
    }

    bool fileDelete(const engine::string& file)
    {
        std::error_code err;
        return std::filesystem::remove(file.c_str(), err);
        std::error_condition ok;
        if (err != ok)
            return false;
    }

    void fileRename(const engine::string& from, const engine::string& to)
    {
        std::filesystem::rename(from, to);
    }
}
