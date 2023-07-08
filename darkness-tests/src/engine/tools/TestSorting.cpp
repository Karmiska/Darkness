#include "GlobalTestFixture.h"
#include "shaders/core/tools/sort/RadixSort.h"

#include "containers/vector.h"
#include <random>

TEST(TestSorting, RadixSort)
{
    GlobalEnvironment& env = *envPtr;

    // Generate random input data
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(0, 1000000);

    constexpr int InputSize = 1024;
    engine::vector<uint32_t> input;
    for (int i = 0; i < InputSize; ++i)
    {
        input.emplace_back(static_cast<uint32_t>(dis(gen)));
    }

    auto inputBuffer = env.device().createBufferSRV(engine::BufferDescription()
        .format(Format::R32_UINT)
        .elements(InputSize)
        .name("RadixSort input data")
        .usage(engine::ResourceUsage::GpuRead)
        .setInitialData(input));

    auto outputBuffer = env.device().createBufferUAV(engine::BufferDescription()
        .format(Format::R32_UINT)
        .elements(InputSize)
        .name("RadixSort output data")
        .usage(engine::ResourceUsage::GpuReadWrite));

    auto readBackBuffer = env.device().createBufferSRV(engine::BufferDescription()
        .format(Format::R32_UINT)
        .elements(InputSize)
        .name("RadixSort readback data")
        .usage(engine::ResourceUsage::GpuToCpu));

    auto radixSort = env.device().createPipeline<RadixSort>();

    {
        auto cmd = env.device().createCommandList("Radix sort");
        radixSort.cs.src = inputBuffer;
        radixSort.cs.dst = outputBuffer;
        radixSort.cs.count.x = InputSize;
        cmd.bindPipe(radixSort);
        cmd.dispatch(roundUpToMultiple(InputSize, 64) / 64, 1, 1);
        env.submit(cmd);
    }

    {
        auto cmd = env.device().createCommandList("Readback radix sort results");
        cmd.copyBuffer(outputBuffer.resource().buffer(), readBackBuffer.resource().buffer(), InputSize);
        env.device().submitBlocking(cmd);
    }

    {
        auto ptr = reinterpret_cast<uint32_t*>(readBackBuffer.resource().buffer().map(env.device()));

        auto temp = ptr;
        for (int i = 0; i < InputSize; ++i)
        {
            LOG("index: %i, Val: %u", i, *temp);
            ++temp;
        }

        uint32_t currentValue = 0;
        for (int i = 0; i < InputSize; ++i)
        {
            EXPECT_GE(*ptr, currentValue);
            currentValue = *ptr;
            ++ptr;
        }
        readBackBuffer.resource().buffer().unmap(env.device());
    }
}
