#include "AllocatorHooks.h"

#include "platform/Platform.h"
#include "Application.h"
#include "ui/Theme.h"
#include "platform/Environment.h"
#include "tools/PathTools.h"
#include "platform/window/Window.h"

/*#include "tools/Settings.h"
#include "platform/Environment.h"
#include "tools/PathTools.h"*/

#if defined(_WIN32)
int CALLBACK WinMain(
    _In_ HINSTANCE, // hInstance,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int   //       nCmdShow
    )
{
	ui::Theme::instance().loadSettings(engine::pathClean(engine::pathJoin(engine::pathJoin(engine::pathExtractFolder(engine::getExecutableDirectory()), "../../../data/"), "ThemeConfiguration.json")));

	engine::shared_ptr<editor::Application> application = engine::make_shared<editor::Application>();
    return application->execute();
}
#endif

