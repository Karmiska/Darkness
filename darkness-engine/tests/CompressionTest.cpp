#include "CompressionTest.h"
#include "tools/CompressedFile.h"
#include "tools/Debug.h"
#include <iostream>
#include <fstream>
#include "containers/vector.h"

using namespace engine;

CompressionTest::CompressionTest(const engine::string& testFile, CompressionTypes type)
{
	std::ifstream in;
    in.open(testFile.c_str(), std::ios::binary);
    engine::vector<char> inBuf;
    in.seekg(0, std::ios::end);
    size_t inSize = static_cast<size_t>(in.tellg());
    inBuf.resize(inSize);
    in.seekg(0, std::ios::beg);
    in.read(&inBuf[0], static_cast<std::streamsize>(inSize));
    in.close();

    CompressedFile outCompress;
    outCompress.open(engine::string(testFile + ".compressed").c_str(), std::ios::out | std::ios::binary, type);
    outCompress.write(&inBuf[0], static_cast<std::streamsize>(inSize));
    outCompress.close();

    CompressedFile inCompress;
    inCompress.open(engine::string(testFile + ".compressed").c_str(), std::ios::in | std::ios::binary);
    engine::vector<char> resInBuf;
    inCompress.seekg(0, std::ios::end);
    size_t inCompressSize = inCompress.tellg();
    inCompress.seekg(0, std::ios::beg);
    resInBuf.resize(inCompressSize);
    inCompress.read(&resInBuf[0], static_cast<std::streamsize>(inCompressSize));
    inCompress.close();

	std::ofstream out;
    out.open(engine::string(testFile + ".result").c_str(), std::ios::binary);
    out.write(&resInBuf[0], static_cast<std::streamsize>(inCompressSize));
    out.close();

    // is the original file the same size as the resulting file
    ASSERT(inSize == inCompressSize);

    // is the content identical
    for (size_t i = 0; i < inSize; ++i)
    {
        ASSERT(inBuf[i] == resInBuf[i]);
    }
}
