#include "ui/Frame.h"
#include "platform/window/Window.h"
#include "engine/RenderSetup.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Swapchain.h"
#include "ui/MessageProcessor.h"
#include "tools/Debug.h"
#include "ui/Theme.h"
#include "ui/DrawCommandBuffer.h"
#include "ui/UiTools.h"

namespace ui
{
	Frame* lastMouseMoveFrame = nullptr;

	Frame::Frame(Frame* parent)
		: m_api{ parent->api() }
	{
		setParent(parent);
		area().x(0);
		area().y(0);
		area().width(0);
		area().height(0);
		clientArea().left = 0;
		clientArea().top = 0;
		clientArea().right = 0;
		clientArea().bottom = 0;
		createRootFrame(parent, m_api);
	}

	Frame::Frame(int width, int height, engine::GraphicsApi api, Frame* parent)
		: m_forceRootFrame{ false }
		, m_rootFrame{ nullptr }
		, m_drawBackground{ true }
		, m_themeSet{ false }
		, m_api{ api }
	{
		minimumSize({ 20, 20 });

		setParent(parent);
		area().x(0);
		area().y(0);
		area().width(width);
		area().height(height);

		clientArea().left = 0;
		clientArea().top = 0;
		clientArea().right = 0;
		clientArea().bottom = 0;

		createRootFrame(parent, api);
		backgroundColor(Theme::instance().color("frame_backgroundColor").xyz());
	}

	Frame::~Frame()
	{
		if (this == lastMouseMoveFrame)
			lastMouseMoveFrame = nullptr;
	}

	void Frame::themeSet(bool value)
	{
		m_themeSet = value;
	}

	engine::Device& Frame::device()
	{
		if (m_rootFrame && m_rootFrame->rendering)
			return m_rootFrame->rendering->device();
		else if (parent())
		{
			return dynamic_cast<Frame*>(parent())->device();
		}
		else
		{
			ASSERT(false, "Unattached frame! All frames need to be either root frames or childs of frames");
		}
	}

	engine::shared_ptr<platform::Window> Frame::windowShared()
	{
		if (m_rootFrame && m_rootFrame->window)
			return m_rootFrame->window;
		else if (parent())
		{
			return dynamic_cast<Frame*>(parent())->windowShared();
		}
		else
		{
			ASSERT(false, "Unattached frame! All frames need to be either root frames or childs of frames");
		}
		return {};
	}

	void Frame::forceRootFrame(bool value, engine::GraphicsApi _api)
	{
		bool change = m_forceRootFrame != value;
		m_forceRootFrame = value;
		api(_api);
		if(change)
			createRootFrame(dynamic_cast<Frame*>(m_parent), _api);
	}

	void Frame::api(engine::GraphicsApi api)
	{
		m_api = api;
		for (auto&& child : m_childs)
			dynamic_pointer_cast<Frame>(child)->api(api);
	}

	void Frame::onMouseDown(engine::MouseButton button, int x, int y)
	{
		if(m_canFocus)
			moveToTop();
		UiObject::onMouseDown(button, x, y);
	}

	void Frame::createRootFrame(Frame* parent, engine::GraphicsApi api)
	{
		onFrameWindowChangeBegin();
		m_rootFrame = nullptr;
		if(m_forceRootFrame || !parent)
			m_rootFrame = engine::make_unique<RootFrame>(this, parent, api, area().width(), area().height());

		if(m_rootFrame)
			onFrameWindowChangeEnd(m_rootFrame->window);
		else
			onFrameWindowChangeEnd(nullptr);

		if (m_rootFrame && parent)
			m_rootFrame->window->position(area().position().x, area().position().y);
	}

	platform::Window* Frame::window()
	{
		if(m_rootFrame)
			return m_rootFrame->window.get();
		return nullptr;
	}

	void Frame::backgroundColor(const engine::Vector3f& color)
	{
		m_backgroundColor = engine::Vector4f(color, 1.0f);
	}

	void Frame::backgroundColor(const engine::Vector4f& color)
	{
		m_backgroundColor = color;
	}

	engine::Vector4f Frame::backgroundColor() const
	{
		return m_backgroundColor;
	}

	void Frame::recreateWindowHandles(Frame* frame)
	{
		if (frame->window())
			frame->createRootFrame(frame->getParent(), frame->api());

		for (auto&& child : frame->childs())
			recreateWindowHandles(dynamic_cast<Frame*>(child.get()));
	}

	void Frame::reparent(Frame* parent)
	{
		UiObject::reparent(parent->shared_from_this());
		createRootFrame(parent, parent->api());

		auto findRootFrame = [](Frame* frame)->Frame*
		{
			while (frame->getParent())
				frame = frame->getParent();
			return frame;
		};

		auto rootFrame = findRootFrame(this);
		for (auto&& frame : rootFrame->childs())
			recreateWindowHandles(dynamic_cast<Frame*>(frame.get()));
	}

	Frame* Frame::getParent()
	{
		return dynamic_cast<Frame*>(m_parent);
	}

	Frame* Frame::getParentRootFrame()
	{
		if (m_rootFrame && m_rootFrame->rendering)
			return this;
		auto p = dynamic_cast<Frame*>(parent());
		while (p && !(p->m_rootFrame && p->m_rootFrame->rendering))
		{
			p = dynamic_cast<Frame*>(p->parent());
		}
		return p;
	}

	void Frame::frameMessageLoopClose()
	{
		// this happens when the window was closed with X on the corner
		if (m_rootFrame)
			m_rootFrame = nullptr;
		m_parent = nullptr;
	}

	void Frame::repositionSystemWindows(Frame* frame)
	{
		if (frame->m_rootFrame && frame->m_rootFrame->window)
		{
			ui::UiPoint gp = UiDragable::getGlobalPosition({}, frame);
			if (gp.x != frame->m_rootFrame->lastWindowPosX ||
				gp.y != frame->m_rootFrame->lastWindowPosY)
			{
				frame->m_rootFrame->window->position(gp.x, gp.y);
				frame->m_rootFrame->lastWindowPosX = gp.x;
				frame->m_rootFrame->lastWindowPosY = gp.y;
			}
		}

		for (auto&& child : frame->childs())
			repositionSystemWindows(dynamic_cast<Frame*>(child.get()));
	}

	void Frame::invalidate()
	{
		if (!m_rootFrame || !m_rootFrame->rendering)
			return;

		for(auto&& child : childs())
			repositionSystemWindows(dynamic_cast<Frame*>(child.get()));

		//auto gp = getGlobalPosition();
		//engine::Rectangle clientRect {
		//	gp.x,
		//	gp.y,
		//	gp.x + this->width(),
		//	gp.y + this->height() };
		
		//std::queue<RenderList> renderList;
		//render(renderList, nullptr, clientRect);
		render(nullptr);
		
		//int counter = 0;
		//while(renderList.size() > 0)
		//{
		//	auto list = renderList.front();
		//	list.rootFrame->rendering->submit(list.apiCmd);
		//	//if(counter > 0)
		//	list.rootFrame->rendering->present();
		//	if (list.rootFrame != m_rootFrame.get())
		//	{
		//		auto display = list.rootFrame->rendering->grabDisplay();
		//		auto displayTex = m_rootFrame->rendering->createGrabTexture(display);
		//
		//		{
		//			auto gp = list.frame->getGlobalPosition();
		//			auto rtv = m_rootFrame->rendering->currentRTV();
		//			auto cmd = m_rootFrame->rendering->device().createCommandList("Sub window blit");
		//			cmd.setRenderTargets({ rtv });
		//			cmd.setScissorRects({ engine::Rectangle{ 0, 0, rtv.width(), rtv.height() } });
		//			m_rootFrame->drawImagePipeline->vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
		//			m_rootFrame->drawImagePipeline->vs.x = gp.x;
		//			m_rootFrame->drawImagePipeline->vs.y = gp.y;
		//			m_rootFrame->drawImagePipeline->vs.width = displayTex.width();
		//			m_rootFrame->drawImagePipeline->vs.height = displayTex.height();
		//			m_rootFrame->drawImagePipeline->ps.image = displayTex;
		//			cmd.bindPipe(*m_rootFrame->drawImagePipeline);
		//			cmd.draw(4);
		//			m_rootFrame->rendering->submit(cmd);
		//		}
		//	}
		//
		//	renderList.pop();
		//	++counter;
		//}
	}

	void Frame::loadFrameImages()
	{
		m_frameThemeImages = engine::make_shared<GridImage>(device(), "frame");
		m_frameTabThemeImages = engine::make_shared<GridImage>(device(), "tab");
	}

	UiPoint Frame::getGlobalPosition() const
	{
		auto pos = area().position();
		auto p = dynamic_cast<const Frame*>(parent());
		while (p)
		{
			pos += p->area().position();
			p = dynamic_cast<const Frame*>(p->parent());
		}
		return pos;
	}

	UiPoint Frame::getGlobalPositionToLastRootFrame() const
	{
		if (m_rootFrame)
			return {};
		auto pos = area().position();
		auto p = dynamic_cast<const Frame*>(parent());
		while (p && !p->m_rootFrame)
		{
			pos += p->area().position();
			p = dynamic_cast<const Frame*>(p->parent());
		}
		return pos;
	}

	//void Frame::render(std::queue<RenderList>& renderList, RenderList* item, engine::Rectangle clientRect)
	void Frame::render(DrawCommandBuffer* cmd)
	{
		bool pushedTransform = false;
		if (m_rootFrame && m_rootFrame->rendering)
		{
			cmd = m_rootFrame->cmd.get();
			ASSERT(!cmd->isOpen(), "Trying to reuse a commandlist that is already in use");
			cmd->open();
			cmd->reset();

			auto gp = getGlobalPositionToLastRootFrame();
			cmd->pushTransform(DrawCommandBuffer::GlobalTransform{ 
				gp.x, gp.y, this->width(), this->height(),
				gp.x, gp.y, this->width(), this->height() });
			pushedTransform = true;
		}
		else if (m_rootFrame)
		{
			//auto gp = getGlobalPosition();
			//cmd->pushTransform(DrawCommandBuffer::GlobalTransform{ gp.x, gp.y, this->width(), this->height() });
		}

#if 1
		//auto gp = (m_rootFrame && !m_rootFrame->rendering) ? getGlobalPosition() : getGlobalPositionToLastRootFrame();
		//engine::Rectangle clientScissor{ 
		//	gp.x + clientArea().left, 
		//	gp.y + clientArea().top,
		//	gp.x + this->width() - clientArea().right,
		//	gp.y + this->height() - clientArea().bottom };
		//engine::Rectangle scissor{
		//	gp.x,
		//	gp.y,
		//	gp.x + this->width(),
		//	gp.y + this->height() };
		//
		//clientScissor = clientScissor.intersect(clientRect);
		//scissor = scissor.intersect(clientRect);

		if (m_themeSet)
		{
			if (!m_frameThemeImages)
				loadFrameImages();

			// frame
			m_frameThemeImages->draw(*cmd, 0, 18, area().width(), area().height() - 18,
				GridImages::TopLeft | GridImages::TopCenter | GridImages::TopRight |
				GridImages::MiddleLeft | GridImages::MiddleCenter | GridImages::MiddleRight |
				GridImages::BottomLeft | GridImages::BottomCenter | GridImages::BottomRight );

			// frame tab
			m_frameTabThemeImages->draw(*cmd, 0, 0, 120, 19,
				GridImages::TopLeft | GridImages::TopCenter | GridImages::TopRight |
				GridImages::MiddleLeft | GridImages::MiddleCenter | GridImages::MiddleRight);
		}
		else if(m_drawBackground)
		{
			cmd->drawRectangle(0, 0, area().width(), area().height(), m_backgroundColor);
		}

		onPaint(*cmd);
#endif

		for (auto&& child : m_childs)
		{
			auto frame = dynamic_pointer_cast<Frame>(child);

			cmd->pushTransform(DrawCommandBuffer::GlobalTransform{
				frame->position().x, frame->position().y, frame->width(), frame->height(),
				frame->position().x, frame->position().y, frame->width(), frame->height() });
			if (cmd->currentTransform().clipHeight != 0 &&
				cmd->currentTransform().clipWidth != 0)
			{

				frame->render(cmd);
				//#ifdef SINGLE_THREADED_ENGINE_FRAME
				if (api() == frame->api() && frame->m_rootFrame && frame->m_rootFrame->rendering)
				{
					auto parentRootFrame = getParentRootFrame();

					auto gp = frame->getGlobalPosition();
					auto rtv = m_rootFrame->rendering->currentRTVSRV();
					auto parentRtv = parentRootFrame->m_rootFrame->rendering->currentRTV();
					auto cmd = parentRootFrame->m_rootFrame->rendering->device().createCommandList("Sub window blit");
					cmd.setRenderTargets({ parentRtv });
					cmd.setScissorRects({ engine::Rectangle{ 0, 0, static_cast<int>(rtv.width()), static_cast<int>(rtv.height()) } });
					parentRootFrame->m_rootFrame->drawImagePipeline->vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
					parentRootFrame->m_rootFrame->drawImagePipeline->vs.x = gp.x;
					parentRootFrame->m_rootFrame->drawImagePipeline->vs.y = gp.y;
					parentRootFrame->m_rootFrame->drawImagePipeline->vs.width = static_cast<int>(rtv.width());
					parentRootFrame->m_rootFrame->drawImagePipeline->vs.height = static_cast<int>(rtv.height());
					parentRootFrame->m_rootFrame->drawImagePipeline->ps.image = rtv;
					cmd.bindPipe(*parentRootFrame->m_rootFrame->drawImagePipeline);
					cmd.draw(4);
					parentRootFrame->m_rootFrame->rendering->submit(cmd);
				}

#if 0
				if (api() != frame->api() && frame->m_rootFrame)
				{
					// and grab the results, draw to this command list
					auto display = frame->m_rootFrame->rendering->grabDisplay();
					auto displayTex = m_rootFrame->rendering->createGrabTexture(display);

					auto cgp = frame->getGlobalPosition();
					cmd->drawImage(
						cgp.x, cgp.y, frame->area().width(), frame->area().height(),
						displayTex);
					//{
					//	auto gp = frame->getGlobalPosition();
					//	auto rtv = m_rootFrame->rendering->currentRTV();
					//	auto cmd = m_rootFrame->rendering->device().createCommandList("Sub window blit");
					//	cmd.setRenderTargets({ rtv });
					//	cmd.setScissorRects({ engine::Rectangle{ 0, 0, rtv.width(), rtv.height() } });
					//	m_rootFrame->drawImagePipeline->vs.screenSize = { static_cast<float>(rtv.width()), static_cast<float>(rtv.height()) };
					//	m_rootFrame->drawImagePipeline->vs.x = gp.x;
					//	m_rootFrame->drawImagePipeline->vs.y = gp.y;
					//	m_rootFrame->drawImagePipeline->vs.width = displayTex.width();
					//	m_rootFrame->drawImagePipeline->vs.height = displayTex.height();
					//	m_rootFrame->drawImagePipeline->ps.image = displayTex;
					//	cmd.bindPipe(*m_rootFrame->drawImagePipeline);
					//	cmd.draw(4);
					//	m_rootFrame->rendering->submit(cmd);
					//}
				}
#endif
				//#endif
			}
			cmd->popTransform();
		}

		if (m_rootFrame && m_rootFrame->rendering)
		{
			// close and present the client commandlist
			cmd->close();
			auto apiCommandLists = cmd->recordCommands(m_rootFrame->rendering.get(), m_rootFrame->rendering->currentRTV());

			for (auto&& apiCmd : apiCommandLists)
			{
				if (apiCmd.renderer)
				{
					apiCmd.renderer->submit(apiCmd.cmdList);
					apiCmd.renderer->present(true);
				}
				else
					m_rootFrame->rendering->submitBlocking(apiCmd.cmdList);
			}

			m_rootFrame->rendering->present(false);
		}
		//else
		//	return cmd;

		//if (m_rootFrame && m_rootFrame->rendering)
		if(pushedTransform)
		{
			cmd->popTransform();
		}
		else if (m_rootFrame)
		{
			//cmd->popTransform();
		}
	}
}
