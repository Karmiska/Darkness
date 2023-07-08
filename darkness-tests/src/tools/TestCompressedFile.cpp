#include "gtest/gtest.h"
#include "Tools.h"

#include "containers/vector.h"
#include <fstream>
#include <cstdio>
#include "tools/CompressedFile.h"

using namespace engine;

class TestCompressedFile : public ::testing::Test
{
};

uint32_t fileSize(string filename)
{
    std::streampos size{ 0 };
	std::ifstream file;
    file.open(filename.c_str(), std::ios::binary);
    EXPECT_EQ(file.is_open(), true);
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size = file.tellg();
        file.close();
    }
    return static_cast<uint32_t>(size);
}

TEST(TestCompressedFile, CompressAndDecompressFileDefault)
{
	engine::vector<int> testData{ generateTestData<int>() };

    const string filename = "CompressedTestDataDefault.dat";

    if (fileExists(filename))
    {
        removeFile(filename);
    }

    {
        CompressedFile file;
        file.open(filename, std::ios::out | std::ios::binary);
        EXPECT_EQ(file.is_open(), true);
        if (file.is_open())
        {
            file.write(reinterpret_cast<const char*>(testData.data()), static_cast<std::streamsize>(testData.size() * sizeof(int)));
            file.close();
            EXPECT_EQ(file.is_open(), false);
        }
    }

    EXPECT_GT(fileSize(filename), static_cast<uint32_t>(0));

    {
        CompressedFile file;
        file.open(filename, std::ios::in | std::ios::binary);
        EXPECT_EQ(file.is_open(), true);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
			std::streampos size = file.tellg();
            EXPECT_EQ(testData.size() * sizeof(int), size);

            file.seekg(0, std::ios::beg);
			engine::vector<int> compareData(testData.size(), 0);
            file.read(reinterpret_cast<char*>(compareData.data()), size);
            file.close();

            for(int i = 0; i < testData.size(); ++i)
            {
                EXPECT_EQ(testData[i], compareData[i]);
            }
        }
    }

    removeFile(filename);
}

TEST(TestCompressedFile, CompressAndDecompressFileBZip2)
{
	engine::vector<int> testData{ generateTestData<int>() };

    const string filename = "CompressedTestDataBZip2.dat";

    if (fileExists(filename))
    {
        removeFile(filename);
    }

    {
        CompressedFile file;
        file.open(filename, std::ios::out | std::ios::binary, CompressionTypes::Bzip2);
        EXPECT_EQ(file.is_open(), true);
        if (file.is_open())
        {
            file.write(reinterpret_cast<const char*>(testData.data()), static_cast<std::streamsize>(testData.size() * sizeof(int)));
            file.close();
            EXPECT_EQ(file.is_open(), false);
        }
    }

    EXPECT_GT(fileSize(filename), static_cast<uint32_t>(0));

    {
        CompressedFile file;
        file.open(filename, std::ios::in | std::ios::binary);
        EXPECT_EQ(file.is_open(), true);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
			std::streampos size = file.tellg();
            EXPECT_EQ(testData.size() * sizeof(int), size);

            file.seekg(0, std::ios::beg);
			engine::vector<int> compareData(testData.size(), 0);
            file.read(reinterpret_cast<char*>(compareData.data()), size);
            file.close();

            for (int i = 0; i < testData.size(); ++i)
            {
                EXPECT_EQ(testData[i], compareData[i]);
            }
        }
    }

    removeFile(filename);
}

TEST(TestCompressedFile, CompressAndDecompressFileZstd)
{
	engine::vector<int> testData{ generateTestData<int>() };

    const string filename = "CompressedTestDataZstd.dat";

    if (fileExists(filename))
    {
        removeFile(filename);
    }

    {
        CompressedFile file;
        file.open(filename, std::ios::out | std::ios::binary, CompressionTypes::Zstd);
        EXPECT_EQ(file.is_open(), true);
        if (file.is_open())
        {
            file.write(reinterpret_cast<const char*>(testData.data()), static_cast<std::streamsize>(testData.size() * sizeof(int)));
            file.close();
            EXPECT_EQ(file.is_open(), false);
        }
    }

    EXPECT_GT(fileSize(filename), static_cast<uint32_t>(0));

    {
        CompressedFile file;
        file.open(filename, std::ios::in | std::ios::binary);
        EXPECT_EQ(file.is_open(), true);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
			std::streampos size = file.tellg();
            EXPECT_EQ(testData.size() * sizeof(int), size);

            file.seekg(0, std::ios::beg);
			engine::vector<int> compareData(testData.size(), 0);
            file.read(reinterpret_cast<char*>(compareData.data()), size);
            file.close();

            for (int i = 0; i < testData.size(); ++i)
            {
                EXPECT_EQ(testData[i], compareData[i]);
            }
        }
    }

    removeFile(filename);
}
