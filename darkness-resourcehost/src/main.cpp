#include "platform/Platform.h"
#include "tools/ArgParser.h"
#include "tools/Debug.h"
#include "engine/resources/ResourceDropHandler.h"
#include "engine/resources/ResourceHost.h"

#include "containers/vector.h"
#include "containers/string.h"

using namespace engine;
using namespace engine;

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
    /*ResourceTask task(
        argParser.value("ip"),
        std::stoi(argParser.value("port")),
        std::stoi(argParser.value("magic")));*/
    //return task.join();

    ResourceDropHandler handler(
        "C:\\Users\\Aleksi Jokinen\\Documents\\TestDarknessProject\\content",
        "C:\\Users\\Aleksi Jokinen\\Documents\\TestDarknessProject\\processed");

    auto destination = "C:\\Users\\Aleksi Jokinen\\Documents\\TestDarknessProject\\content\\test";
    auto source = "C:\\work\\resources\\primitives\\uv_grid.jpg";
    //auto source2 = "C:\\work\\resources\\testimages";
    //auto source = "C:\\work\\resources\\testimages\\white_square.hdr";
    auto source2 = "C:\\work\\resources\\vehicles\\hover\\hovercraft.obj";
    auto package = handler.handleDrop({ source, source2 }, destination, static_cast<unsigned int>(Format::BC1_UNORM), true, false, false);

    package.onStarted = [](ProcessResourceItem item)
    {
        LOG("RESOURCE HOST. Task Started: %s", item.absoluteSourceFilepath.c_str());
    };
    package.onFinished = [](ProcessResourceItem item)
    {
        LOG("RESOURCE HOST. Task Finished: %s", item.absoluteSourceFilepath.c_str());
    };
    package.onProgress = [](ProcessResourceItem item, float progress)
    {
        LOG("RESOURCE HOST. Task: %s, Progress: %f", item.absoluteSourceFilepath.c_str(), progress);
    };
    package.onProgressMessage = [](ProcessResourceItem item, float progress, const engine::string& message)
    {
        LOG("RESOURCE HOST. Task: %s, Progress: %f, Message: %s", 
            item.absoluteSourceFilepath.c_str(), 
            progress,
            message.c_str());
    };

    ResourceHost resourceHost;
    resourceHost.processResources(package);

    return 0;
}
#endif
