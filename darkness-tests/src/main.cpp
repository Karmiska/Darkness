#include "gtest/gtest.h"
#include "GlobalTestFixture.h"
#include "tools/ArgParser.h"
#include <algorithm>

#ifdef _WIN32

int doWork();

#include <Windows.h>
#include <stdio.h>
#include <shellapi.h>
int CALLBACK WinMain(
    _In_ HINSTANCE, // hInstance,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int   //       nCmdShow
)
{
    return doWork();
}

int main(int argc, char* argv[])
{
    return doWork();
}

int doWork()
{
    engine::ArgParser argParser(__argc, __argv);

    ::testing::InitGoogleTest(&__argc, __argv);
    envPtr = new GlobalEnvironment();
    if (argParser.isSet("adapter"))
        envPtr->preferredAdapter(argParser.value("adapter"));
    ::testing::AddGlobalTestEnvironment(envPtr);

    return RUN_ALL_TESTS();
}

#endif
