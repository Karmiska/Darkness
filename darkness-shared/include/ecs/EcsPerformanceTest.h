#pragma once

#include <Windows.h>
#undef min
#undef max
#include <cmath>

namespace ecs
{
    class EcsPerformanceTest
    {
    public:
        EcsPerformanceTest();

        void runTinyTest();
        void runSmallTest();
        void runLargeTest();

    private:
        LARGE_INTEGER freq;
        LARGE_INTEGER prewarm;
        LARGE_INTEGER start;
        LARGE_INTEGER populated;
        LARGE_INTEGER simulated;
        LARGE_INTEGER erased;

        void performTest(size_t count);
        void printResults(size_t count) const;
    };
}
