#pragma once

#include "containers/string.h"
#include <fstream>
#include "containers/vector.h"

template<typename T>
engine::vector<T> generateTestData(size_t size = 32768)
{
    engine::vector<T> data;
    for (int i = 0; i < size; ++i)
    {
        data.emplace_back(i);
    }
    return data;
}

bool fileExists(engine::string filename);
void removeFile(engine::string filename);
