#include "Tools.h"

bool fileExists(engine::string filename)
{
    std::ifstream file(filename.c_str());
    return file.good();
}

void removeFile(engine::string filename)
{
    remove(filename.data());
}
