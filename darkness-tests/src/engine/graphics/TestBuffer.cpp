#include "GlobalTestFixture.h"

#include "platform/window/Window.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/CommandList.h"
#include "containers/memory.h"
#include "containers/vector.h"

using namespace engine;
using namespace platform;
using namespace engine;

class TestBuffer : public ::testing::Test
{
};

struct TestUniform
{
    float value;
    int anotherValue;
};

vector<TestUniform> generateTestData()
{
    vector<TestUniform> data;
    for(int i = 0; i < 100; ++i)
    {
        float value = static_cast<float>(i);
        data.emplace_back(TestUniform{ value + (100.0f / (value + 0.001f)), i });
    }
    return data;
}

TEST(TestBuffer, DISABLED_GeneriBufferMap)
{
    GlobalEnvironment& env = *envPtr;

    vector<TestUniform> testData = generateTestData();

    auto genericBuffer = env.device().createBuffer(BufferDescription()
        .elements(testData.size())
        .elementSize(sizeof(TestUniform))
        .name("genericBuffertest")
        .setInitialData(BufferDescription::InitialData(testData))
    );

    auto data = genericBuffer.resource().map(env.device());
    TestUniform* bufferData = reinterpret_cast<TestUniform*>(data);
    for (int i = 0; i < testData.size(); ++i)
    {
        EXPECT_EQ(bufferData[i].value, testData[i].value);
        EXPECT_EQ(bufferData[i].anotherValue, testData[i].anotherValue);
    }
    genericBuffer.resource().unmap(env.device());
}

TEST(TestBuffer, GeneriBufferCopy)
{
    GlobalEnvironment& env = *envPtr;
    vector<TestUniform> testData = generateTestData();

    auto srcBuffer = env.device().createBuffer(BufferDescription()
        .elements(testData.size())
        .elementSize(sizeof(TestUniform))
        .name("GenericBufferCopy_srcBuffer")
        .setInitialData(BufferDescription::InitialData(testData))
    );

    auto dstBuffer = env.device().createBuffer(BufferDescription()
        .usage(ResourceUsage::GpuToCpu)
        .elements(testData.size())
        .name("GenericBufferCopy_dstBuffer")
        .elementSize(sizeof(TestUniform))
    );
}
