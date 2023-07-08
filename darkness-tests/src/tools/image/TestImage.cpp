#include "gtest/gtest.h"
#include "engine/graphics/Format.h"
#include "Tools.h"

#include <fstream>
#include "containers/string.h"
#include "tools/image/Image.h"

using namespace engine;

class TestImage : public ::testing::Test
{
};

TEST(TestImage, DISABLED_WriteAndReadImage)
{
    const string filename = "ImageTest.dds";

    auto bytes = engine::imageBytes(engine::Format::BC7_UNORM, 1024, 1024, 1, 1);

    {
        auto image = engine::image::Image::createImage(filename, engine::image::ImageType::DDS, engine::Format::BC7_UNORM, 1024, 1024);
        EXPECT_EQ(image->width(), 1024);
        EXPECT_EQ(image->height(), 1024);
        image->save(reinterpret_cast<char*>(generateTestData<unsigned char>(bytes).data()), bytes * sizeof(unsigned char));
    }

    {
        auto image = engine::image::Image::createImage(filename, engine::image::ImageType::DDS);
        EXPECT_EQ(image->width(), 1024);
        EXPECT_EQ(image->height(), 1024);
        EXPECT_EQ(image->bytes(), bytes * sizeof(unsigned char));
        engine::vector<unsigned char> compareData(bytes, 0);
        memcpy(static_cast<void*>(compareData.data()), image->data(), image->bytes());

        auto data = generateTestData<unsigned char>(bytes);
        for (int i = 0; i < bytes; ++i)
        {
            EXPECT_EQ(compareData[i], data[i]);
        }
    }

    if (fileExists(filename))
    {
        removeFile(filename);
    }
}
