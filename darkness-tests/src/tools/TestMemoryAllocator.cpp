#include "gtest/gtest.h"
#include "tools/MemoryAllocator.h"
#include "tools/ByteRange.h"
#include "tools/Debug.h"
#include "tools/Measure.h"
#include <random>

using namespace engine;
using namespace tools;

template<typename T>
engine::string debugRange(T& range, int index)
{
    return "[alignment: " + engine::string(std::to_string(indexToAlign(index)).c_str()) + ", count: " + engine::string(std::to_string(range.size()).c_str()) + "]";
};

TEST(TestMemoryAllocator, MemoryAllocator_PerformanceTest)
{
    engine::vector<uint8_t> heap(3ull * 1024ull * 1024ull * 1024ull);
    ByteRange range(heap);
    tools::MemoryAllocator allocator(range, 65536);

    // test alignment from 1 to 2^64
    ASSERT(alignToIndex(1) == 0, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(2) == 1, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(4) == 2, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(8) == 3, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(16) == 4, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(32) == 5, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(64) == 6, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(128) == 7, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(256) == 8, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(512) == 9, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(1024) == 10, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(2048) == 11, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(4096) == 12, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(8192) == 13, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(16384) == 14, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(32768) == 15, "assignToIndex returned invalid value");
    ASSERT(alignToIndex(65536) == 16, "assignToIndex returned invalid value");

    /*for(int i = 1; i < 64; ++i)
    {
        uint64_t alignment = 1 << i;
        ASSERT(alignToIndex(alignment) == i, "assignToIndex returned invalid value");
    }*/
    
    

    auto test = allocator.allocate(32, 4);
    allocator.free(test);


    auto printAllocStatus = [&]()
    {
        engine::string allocStatus = "";
        int index = 0;
        for (auto&& range : allocator.freeAllocations())
        {
            allocStatus += debugRange(range, index);
            ++index;
        }
        LOG("free: %s", allocStatus.c_str());

        allocStatus = "";
        index = 0;
        for (auto&& range : allocator.reservedAllocations())
        {
            allocStatus += debugRange(range, index);
            ++index;
        }
        LOG("rese: %s", allocStatus.c_str());
    };


    engine::vector<void*> allocations;
    constexpr int TestCount = 800;
    size_t spaceAllocated = 0;
    Measure measure;
    {
        measure.here(std::to_string(TestCount * 15).c_str()+engine::string(" allocations"));
        for (int i = 1; i < 16; ++i)
        {
            auto alignment = 1 << i;
            std::mt19937 gen(456); //Standard mersenne_twister_engine seeded with rd()
            std::uniform_int_distribution<> dis(1, 1024);
            for (int a = 0; a < TestCount; ++a)
            {
                if (a == 95)
                {
                	int here = 1;
                }
                auto bytes = dis(gen);
                allocations.emplace_back(allocator.allocate(bytes, alignment));
                spaceAllocated += bytes;
            }
        }
    }

    {
        measure.here(std::to_string(allocations.size()).c_str()+engine::string(" frees"));
        std::mt19937 gen(456); //Standard mersenne_twister_engine seeded with rd()
        size_t allocationsSize = allocations.size();
        for (int i = 0; i < allocationsSize; ++i)
        {
            std::uniform_int_distribution<> bdis(0, static_cast<int>(allocations.size()) - 1);
            auto indexToRemove = bdis(gen);
            allocator.free(allocations[indexToRemove]);
            allocations.erase(allocations.begin() + indexToRemove);
        }
    }

    for (auto&& list : allocator.reservedAllocations())
    {
        ASSERT(list.size() == 0, "unfreed allocations!");
    }
}