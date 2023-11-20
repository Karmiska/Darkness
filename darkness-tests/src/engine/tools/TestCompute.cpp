#include "GlobalTestFixture.h"
#include "shaders/core/tools/PrefixScan.h"
#include "shaders/core/tools/PrefixScanUpSweep.h"
#include "engine/rendering/tools/PrefixSum.h"
#include "containers/vector.h"
#include <random>

TEST(DISABLED_TestCompute, PrefixScan)
{
    GlobalEnvironment& env = *envPtr;

    // Generate random input data
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(0, 10);

    constexpr int InputSize = 6000;
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
	auto outputBufferSRV = env.device().createBufferSRV(outputBuffer);

    auto readBackBuffer = env.device().createBufferSRV(engine::BufferDescription()
        .format(Format::R32_UINT)
        .elements(InputSize)
        .name("RadixSort readback data")
        .usage(engine::ResourceUsage::GpuToCpu));

#if 1
    PrefixSum prefixSum(env.device());

    {
        auto cmd = env.device().createCommandList("Exclusive Prefix scan");
        prefixSum.perform(cmd, inputBuffer, outputBuffer);
        env.submit(cmd);
    }

    {
        auto cmd = env.device().createCommandList("Exclusive Prefix scan results");
        cmd.copyBuffer(outputBufferSRV.resource().buffer(), readBackBuffer.resource().buffer(), InputSize);
        env.device().submitBlocking(cmd);
    }

    {
        auto ptr = reinterpret_cast<uint32_t*>(readBackBuffer.resource().buffer().map(env.device()));

        /*for (int i = 0; i < InputSize; ++i)
        {
            LOG("index: %i, original value: %u", i, input[i]);
        }
        for (int i = 0; i < InputSize; ++i)
        {
            LOG("index: %i, prefix sum: %u", i, *(ptr + i));
        }*/


        auto temp = ptr;
        uint32_t currentSum = 0;
        for (int i = 0; i < InputSize; ++i)
        {
            LOG("index: %i, Value: %u, Exclusive Prefix Sum: %u, GPU prefix sum: %u", i, input[i], currentSum, *temp);
            EXPECT_EQ(*temp, currentSum);

            currentSum += input[i];
            ++temp;
        }
        readBackBuffer.resource().buffer().unmap(env.device());
    }

#else
    auto outputBufferSecond = env.device().createBufferUAV(engine::BufferDescription()
        .format(Format::R32_UINT)
        .elements(InputSize)
        .name("RadixSort output 2 data")
        .usage(engine::ResourceUsage::GpuReadWrite));
    auto outputBufferSecondSRV = env.device().createBufferSRV(outputBufferSecond.buffer());

    auto workBuffer = env.device().createBufferUAV(engine::BufferDescription()
        .format(Format::R32_UINT)
        .elements(roundUpToMultiple(InputSize, 64) / 64)
        .name("RadixSort work data")
        .usage(engine::ResourceUsage::GpuReadWrite));
    auto workBufferSRV = env.device().createBufferSRV(workBuffer.buffer());

    auto workBufferSecond = env.device().createBufferUAV(engine::BufferDescription()
        .format(Format::R32_UINT)
        .elements(roundUpToMultiple(InputSize, 64) / 64)
        .name("RadixSort work 2 data")
        .usage(engine::ResourceUsage::GpuReadWrite));

    auto prefixScan = env.device().createPipeline<PrefixScan>();
    auto prefixScanUpSweep = env.device().createPipeline<PrefixScanUpSweep>();

    // EXCLUSIVE SCAN
    {
        {
            auto cmd = env.device().createCommandList("Exclusive Prefix scan");
            prefixScan.cs.src = inputBuffer;
            prefixScan.cs.dst = outputBuffer;
            prefixScan.cs.workBuffer = workBuffer;
            prefixScan.cs.outputsums = true;
            prefixScan.cs.count.x = InputSize;
            prefixScan.cs.inclusive.x = 0u;
            cmd.bindPipe(prefixScan);
            cmd.dispatch(roundUpToMultiple(InputSize, 64) / 64, 1, 1);
            env.submit(cmd);
        }

        {
            auto cmd = env.device().createCommandList("Exclusive Prefix scan");
            prefixScan.cs.src = workBufferSRV;
            prefixScan.cs.dst = outputBufferSecond;
            prefixScan.cs.workBuffer = workBufferSecond;
            prefixScan.cs.outputsums = false;
            prefixScan.cs.count.x = roundUpToMultiple(InputSize, 64) / 64;
            prefixScan.cs.inclusive.x = 0u;
            cmd.bindPipe(prefixScan);
            cmd.dispatch(roundUpToMultiple((roundUpToMultiple(InputSize, 64) / 64), 64) / 64, 1, 1);
            env.submit(cmd);
        }

        {
            auto cmd = env.device().createCommandList("Exclusive Prefix scan");
            prefixScanUpSweep.cs.dst = outputBuffer;
            prefixScanUpSweep.cs.workBuffer = outputBufferSecondSRV;
            prefixScanUpSweep.cs.count.x = InputSize;
            cmd.bindPipe(prefixScanUpSweep);
            cmd.dispatch(roundUpToMultiple(InputSize, 64) / 64, 1, 1);
            env.submit(cmd);
        }


        {
            auto cmd = env.device().createCommandList("Exclusive Prefix scan results");
            cmd.copyBuffer(outputBuffer.buffer(), readBackBuffer.buffer(), InputSize);
            env.device().submitBlocking(cmd);
        }

        {
            auto ptr = reinterpret_cast<uint32_t*>(readBackBuffer.buffer().map(env.device()));

            /*for (int i = 0; i < InputSize; ++i)
            {
                LOG("index: %i, original value: %u", i, input[i]);
            }
            for (int i = 0; i < InputSize; ++i)
            {
                LOG("index: %i, prefix sum: %u", i, *(ptr + i));
            }*/


            auto temp = ptr;
            uint32_t currentSum = 0;
            for (int i = 0; i < InputSize; ++i)
            {
                LOG("index: %i, Value: %u, Exclusive Prefix Sum: %u, GPU prefix sum: %u", i, input[i], currentSum, *temp);
                EXPECT_EQ(*temp, currentSum);

                currentSum += input[i];
                ++temp;
            }
            readBackBuffer.buffer().unmap(env.device());
        }
    }

    // INCLUSIVE SCAN
    {
        {
            auto cmd = env.device().createCommandList("Inclusive Prefix scan");
            prefixScan.cs.src = inputBuffer;
            prefixScan.cs.dst = outputBuffer;
            prefixScan.cs.workBuffer = workBuffer;
            prefixScan.cs.count.x = InputSize;
            prefixScan.cs.inclusive.x = 1u;
            cmd.bindPipe(prefixScan);
            cmd.dispatch(roundUpToMultiple(InputSize, 64 * 8) / (64 * 8), 1, 1);
            env.submit(cmd);
        }

        {
            auto cmd = env.device().createCommandList("Inclusive Prefix scan results");
            cmd.copyBuffer(outputBuffer.buffer(), readBackBuffer.buffer(), InputSize);
            env.device().submitBlocking(cmd);
        }

        {
            auto ptr = reinterpret_cast<uint32_t*>(readBackBuffer.buffer().map(env.device()));

            auto temp = ptr;
            uint32_t currentSum = 0;
            for (int i = 0; i < InputSize; ++i)
            {
                currentSum += input[i];
                LOG("index: %i, Orig: %u, Inclusive Prefix Sum: %u", i, input[i], *temp);
                EXPECT_EQ(*temp, currentSum);
                ++temp;
            }
            readBackBuffer.buffer().unmap(env.device());
        }
    }
#endif
}
