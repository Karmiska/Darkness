#include "platform/Platform.h"
#include "tools/AssetTools.h"
#include "tools/ArgParser.h"
#include "ResourceTask.h"
#include "tools/Debug.h"
#include "ImageTask.h"
#include "ModelTask.h"

using namespace resource_task;

#ifdef _WIN32
int CALLBACK WinMain(
    _In_ HINSTANCE, // hInstance,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int   //       nCmdShow
)
{
    LOG_INFO("Resource task started");
    engine::ArgParser argParser(__argc, __argv);

    if (argParser.isSet("ip"))
    {
        ResourceTask task(
            argParser.value("ip"),
            std::stoi(argParser.value("port").c_str()),
            argParser.value("id"),
            argParser.value("hostid"));
        return task.join();
    }
    else if (argParser.isSet("src") && argParser.isSet("dst"))
    {
        if (engine::isModelFormat(argParser.value("src")))
        {
            ModelTask modelTask;
            modelTask.process(argParser.value("src"), argParser.value("dst"));
        }
        else if (engine::isImageFormat(argParser.value("src")))
        {
            resource_task::internals::CompressonatorInitializer compressonatorInit;

            ImageTask imageTask;
            imageTask.process(argParser.value("src"), argParser.value("dst"));
        }
    }
    return 0;
}
#endif
