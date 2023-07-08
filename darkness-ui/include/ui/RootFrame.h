#pragma once

#include "containers/memory.h"
#include "ui/MessageProcessorRegistration.h"
#include "ui/DrawCommandBuffer.h"
#include "engine/graphics/CommonNoDep.h"

namespace platform
{
	class Window;
}

namespace engine
{
	class RenderSetup;
	class CommandList;
}

namespace ui
{
	class Frame;
	class RootFrame
	{
	public:
		RootFrame(Frame* thisFrame, Frame* parent, engine::GraphicsApi api, int width, int height);
		RootFrame(const RootFrame&) = default;
		RootFrame(RootFrame&&) = default;
		RootFrame& operator=(const RootFrame&) = default;
		RootFrame& operator=(RootFrame&&) = default;
		~RootFrame();

		void recreateWindow(Frame* thisFrame, Frame* parent, int width, int height);

		engine::unique_ptr<MessageProcessorRegistration> registration;
		engine::shared_ptr<platform::Window> window;
		engine::shared_ptr<engine::RenderSetup> rendering;
		engine::unique_ptr<DrawCommandBuffer> cmd;
		engine::unique_ptr<engine::Pipeline<engine::shaders::DrawImage>> drawImagePipeline;
		int lastWindowPosX;
		int lastWindowPosY;
	};
}