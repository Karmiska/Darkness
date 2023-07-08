#pragma once

#include "tools/CompressedFile.h"
#include "containers/string.h"

class CompressionTest
{
public:
    CompressionTest(const engine::string& testFile, CompressionTypes type = CompressionTypes::Zstd);
};
