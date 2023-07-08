#pragma once

#include "UiObject.h"
#include "engine/primitives/Vector3.h"
#include "engine/graphics/CommandList.h"
#include "ui/RootFrame.h"
#include "engine/graphics/CommonNoDep.h"
#include <queue>

namespace engine
{
	class RenderSetup;
	class CommandList;
	struct Rectangle;
}

namespace platform
{
	class Window;
}

#define SINGLE_THREADED_ENGINE_FRAME

namespace ui
{
	extern Frame* lastMouseMoveFrame;

	class MessageProcessor;
	class Theme;
	class DrawCommandBuffer;
	class GridImage;

	class Frame : public UiObject
	{
	public:
		Frame(Frame* parent);
		Frame(int width, int height, engine::GraphicsApi api, Frame* parent = nullptr);
		~Frame();

		platform::Window* window();
		engine::shared_ptr<platform::Window> windowShared();

		void backgroundColor(const engine::Vector3f& color);
		void backgroundColor(const engine::Vector4f& color);
		engine::Vector4f backgroundColor() const;



		bool drawBackground() const { return m_drawBackground; }
		void drawBackground(bool val) { m_drawBackground = val; }

		void reparent(Frame* parent);
		Frame* getParent();
		Frame* getParentRootFrame();
		void invalidate();

		RootFrame* getRootFrame() { return m_rootFrame.get(); }

		void themeSet(bool value);

		void forceRootFrame(bool value, engine::GraphicsApi api);

		void onMouseDown(engine::MouseButton /*button*/, int /*x*/, int /*y*/) override;

		void canFocus(bool val) { m_canFocus = val; }
		bool canFocus() const { return m_canFocus; }
		engine::GraphicsApi api() const { return m_api; }
		void api(engine::GraphicsApi api);
	protected:
		engine::Device& device();
		engine::GraphicsApi m_api;

	protected:
		friend class MessageProcessor;
		virtual void frameMessageLoopClose();
		UiPoint getGlobalPosition() const;
		UiPoint getGlobalPositionToLastRootFrame() const;

	private:
		engine::shared_ptr<GridImage> m_frameThemeImages;
		engine::shared_ptr<GridImage> m_frameTabThemeImages;

		enum FrameImages
		{
			TopLeft = 0x1,
			TopCenter = 0x2,
			TopRight = 0x4,
			MiddleLeft = 0x8,
			MiddleCenter = 0x10,
			MiddleRight = 0x20,
			BottomLeft = 0x40,
			BottomCenter = 0x80,
			BottomRight = 0x100
		};
		
		bool m_forceRootFrame;
		engine::unique_ptr<RootFrame> m_rootFrame;
		void createRootFrame(Frame* parent, engine::GraphicsApi api);

		engine::Vector4f m_backgroundColor;
		bool m_drawBackground;

		void loadFrameImages();

		struct RenderList
		{
			DrawCommandBuffer* cmd;
			engine::CommandList apiCmd;
			engine::TextureRTV rtv;
			ui::RootFrame* rootFrame;
			ui::Frame* frame;
		};

		//void render(std::queue<RenderList>& renderList, RenderList* item, engine::Rectangle clientRect);
		void render(DrawCommandBuffer* cmd);
		void recreateWindowHandles(Frame* frame);
		void repositionSystemWindows(Frame* frame);

		bool m_themeSet;
		bool m_canFocus = false;
	};
}
