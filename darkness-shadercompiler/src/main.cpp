#include "platform/Platform.h"
#include "tools/ArgParser.h"
#include "ShaderLocator.h"
#include "CompileTask.h"
#include "tools/Debug.h"
#include "tools/PathTools.h"
#include "platform/Environment.h"

#include <chrono>

void DoWork(engine::ArgParser& args)
{
    auto now = std::chrono::high_resolution_clock::now();

    if (args.flag("help"))
    {
        LOG_PURE("Usage:");
        LOG_PURE("darknessshadercompiler.exe                        # will check for changes and compile all changed");
        LOG_PURE("darknessshadercompiler.exe -force                 # will compile all shaders and interfaces");
        LOG_PURE("darknessshadercompiler.exe -input=C:\\work\\darkness\\darkness-engine\\shaders\\core\\cull\\ClustersToIndexExpand.cs.hlsl     # will compile a single file");
        LOG_PURE("darknessshadercompiler.exe -log=none              # disable all logging");
        LOG_PURE("darknessshadercompiler.exe -log=error             # show only error messages");
        LOG_PURE("darknessshadercompiler.exe -log=progress          # show compilation progress");
        LOG_PURE("darknessshadercompiler.exe -log=all               # show all messages");
        LOG_PURE("darknessshadercompiler.exe -optimization=release  # compile release binaries (default)");
        LOG_PURE("darknessshadercompiler.exe -optimization=debug    # compile debug binaries");
        return;
    }

    shadercompiler::ShaderLocatorParameters params{ 
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\shaders")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\data\\shaders\\dx12")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\data\\shaders\\vulkan")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\include\\shaders")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\data\\shaders\\dx12\\core")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\tools\\codegen\\ShaderLoadInterfaceTemplate.h")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\tools\\codegen\\ShaderLoadInterfaceTemplate.cpp")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\tools\\codegen\\ShaderPipelineTemplate.h")),
        engine::pathClean(engine::pathJoin(engine::getExecutableDirectory(), "..\\..\\..\\..\\..\\darkness-engine\\tools\\codegen\\ShaderPipelineTemplate.cpp"))
    };
            //"C:\\work\\darkness\\darkness-engine\\shaders",
            //"C:\\work\\darkness\\darkness-engine\\data\\shaders\\dx12",
            //"C:\\work\\darkness\\darkness-engine\\data\\shaders\\vulkan",
            //"C:\\work\\darkness\\darkness-engine\\include\\shaders",
            //"C:\\work\\darkness\\darkness-engine\\data\\shaders\\dx12\\core",
            //"C:\\work\\darkness\\darkness-engine\\tools\\codegen\\ShaderLoadInterfaceTemplate.h",
            //"C:\\work\\darkness\\darkness-engine\\tools\\codegen\\ShaderLoadInterfaceTemplate.cpp",
            //"C:\\work\\darkness\\darkness-engine\\tools\\codegen\\ShaderPipelineTemplate.h",
            //"C:\\work\\darkness\\darkness-engine\\tools\\codegen\\ShaderPipelineTemplate.cpp" };

    shadercompiler::LogLevel logLevel = shadercompiler::LogLevel::Progress;
    if (args.value("log") == "none") logLevel = shadercompiler::LogLevel::None;
    if (args.value("log") == "error") logLevel = shadercompiler::LogLevel::Error;
    if (args.value("log") == "recompile") logLevel = shadercompiler::LogLevel::Recompile;
    if (args.value("log") == "progress") logLevel = shadercompiler::LogLevel::Progress;
    if (args.value("log") == "all") logLevel = shadercompiler::LogLevel::All;

    bool releaseBinaries = true;
    if (args.value("optimization") == "debug") releaseBinaries = false;

    // compile All shaders
    if(args.value("input").empty())
    {
        if(static_cast<int>(logLevel) >= static_cast<int>(shadercompiler::LogLevel::Progress)) LOG_PURE("Locating shaders and checking changes (1/8)");
        params.forceCompileAll = args.flag("force");
        shadercompiler::ShaderLocator locator(params);
        if (locator.pipelines().size() == 0)
        {
            if (static_cast<int>(logLevel) >= static_cast<int>(shadercompiler::LogLevel::Progress)) LOG_PURE("No changes. Exiting.");
        }
        else
            shadercompiler::CompileTask task(locator, releaseBinaries, logLevel);
    }
    else
    {
        if (static_cast<int>(logLevel) >= static_cast<int>(shadercompiler::LogLevel::Progress)) LOG_PURE("Single file compile");
        params.singleFile = args.value("input");
        shadercompiler::ShaderLocator locator(params);
        if (locator.pipelines().size() == 0)
        {
            if (static_cast<int>(logLevel) >= static_cast<int>(shadercompiler::LogLevel::Progress)) LOG_PURE("No changes. Exiting.");
        }
        else
            shadercompiler::CompileTask task(locator, releaseBinaries, logLevel);
    }

    auto after = std::chrono::high_resolution_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(after - now);
    if (static_cast<int>(logLevel) >= static_cast<int>(shadercompiler::LogLevel::Progress)) LOG_PURE("Compile took %i ms", static_cast<int>(milliseconds.count()));
}

int main(int argc, char* argv[])
{
    engine::ArgParser argParser(__argc, __argv);
    DoWork(argParser);
    return 0;
}

int CALLBACK WinMain(
    _In_ HINSTANCE, // hInstance,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int   //       nCmdShow
)
{
    engine::ArgParser argParser(__argc, __argv);
    DoWork(argParser);
    return 0;
}
