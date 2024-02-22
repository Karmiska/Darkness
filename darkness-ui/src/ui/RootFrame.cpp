#include "ui/RootFrame.h"
#include "engine/RenderSetup.h"
#include "ui/Frame.h"
#include "engine/graphics/Swapchain.h"
#include "ui/UiState.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/rendering/Rendering.h"

namespace ui
{
	RootFrame::RootFrame(Frame* thisFrame, Frame* parent, engine::GraphicsApi api, int width, int height)
	{
		recreateWindow(thisFrame, parent, width, height);
		if (!parent || parent->api() != api)
		{
			rendering = engine::make_shared<engine::RenderSetup>(window, api, engine::EngineMode::OwnThread, "RootFrame render", false);
			registration = engine::make_unique<MessageProcessorRegistration>(thisFrame);
			cmd = engine::make_unique<DrawCommandBuffer>(rendering->device());
			drawImagePipeline = engine::make_unique<engine::Pipeline<engine::shaders::DrawImage>>(rendering->device().createPipeline<engine::shaders::DrawImage>());

			engine::DepthStencilOpDescription front;
			front.StencilFailOp = engine::StencilOp::Keep;
			front.StencilDepthFailOp = engine::StencilOp::Incr;
			front.StencilPassOp = engine::StencilOp::Keep;
			front.StencilFunc = engine::ComparisonFunction::Always;

			engine::DepthStencilOpDescription back;
			back.StencilFailOp = engine::StencilOp::Keep;
			back.StencilDepthFailOp = engine::StencilOp::Decr;
			back.StencilPassOp = engine::StencilOp::Keep;
			back.StencilFunc = engine::ComparisonFunction::Always;
			drawImagePipeline->setPrimitiveTopologyType(engine::PrimitiveTopologyType::TriangleStrip);
			drawImagePipeline->setRasterizerState(engine::RasterizerDescription().cullMode(engine::CullMode::None).fillMode(engine::FillMode::Solid)); // TODO
			drawImagePipeline->setDepthStencilState(engine::DepthStencilDescription()
				.depthEnable(false)
				.depthWriteMask(engine::DepthWriteMask::All)
				.depthFunc(engine::ComparisonFunction::GreaterEqual)
				.frontFace(front)
				.backFace(back));
			drawImagePipeline->ps.imageSampler = rendering->device().createSampler(
				engine::SamplerDescription().filter(
					engine::Filter::Bilinear).textureAddressMode(engine::TextureAddressMode::Clamp));
		}
		else
		{
			drawImagePipeline = nullptr;
			cmd = nullptr;
			registration = nullptr;
			rendering = nullptr;
		}
	}

	bool getTranslationToChild(Frame* parent, Frame* child, ui::UiPoint& translation)
	{
		if (parent == child)
			return true;

		for (auto&& c : parent->childs())
		{
			auto childFrame = dynamic_cast<Frame*>(c.get());
			
			ui::UiPoint trans;
			trans = translation + childFrame->area().position();

			if (getTranslationToChild(childFrame, child, trans))
			{
				translation = trans;
				return true;
			}
		}
		return false;
	}

	Frame* translateTowardsRoot(Frame* frame, ui::UiPoint& point)
	{
		if (frame->getParent())
		{
			point += frame->area().position();
			return translateTowardsRoot(frame->getParent(), point);
		}
		else
		{
			return frame;
		}
	};

	struct TargetFramePoint
	{
		Frame* frame;
		ui::UiPoint point;
	};
	engine::vector<TargetFramePoint> translateFromRoot(Frame* frame, ui::UiPoint point, bool& blocking, ui::UiRect clientArea)
	{
		auto intersectRect = [](const ui::UiRect& rect, const ui::UiRect& rectB)->ui::UiRect
		{
			ui::UiRect res;
			res.left(rect.left()); if (rectB.left() > res.left()) res.left(rectB.left());
			res.top(rect.top()); if (rectB.top() > res.top()) res.top(rectB.top());
			res.right(rect.right()); if (rectB.right() < res.right()) res.right(rectB.right());
			res.bottom(rect.bottom()); if (rectB.bottom() < res.bottom()) res.bottom(rectB.bottom());
			if (res.right() < res.left())
				res.right(res.left());
			if (res.bottom() < res.top())
				res.bottom(res.top());
			return res;
		};

		engine::vector<TargetFramePoint> result;
		for (auto rev = frame->childs().rbegin(); rev != frame->childs().rend(); ++rev)
		{
			auto childFrame = dynamic_cast<Frame*>((*rev).get());

			auto revisedClientArea = clientArea;
			revisedClientArea = intersectRect(revisedClientArea, ui::UiRect{
				childFrame->position().x,
				childFrame->position().y,
				childFrame->width(),
				childFrame->height()
				});
			revisedClientArea = intersectRect(revisedClientArea, ui::UiRect{
				static_cast<int>(frame->clientArea().left),
				static_cast<int>(frame->clientArea().top),
				max(static_cast<int>(frame->width()) - static_cast<int>(frame->clientArea().left) - static_cast<int>(frame->clientArea().right), 0),
				max(static_cast<int>(frame->height()) - static_cast<int>(frame->clientArea().top) - static_cast<int>(frame->clientArea().bottom), 0)
				});

			revisedClientArea.left(revisedClientArea.left() - childFrame->area().position().x);
			revisedClientArea.top(revisedClientArea.top() - childFrame->area().position().y);

			auto childresult = translateFromRoot(childFrame, point - childFrame->area().position(), blocking, revisedClientArea);
			if (childresult.size() > 0)
			{
				result.reserve(result.size() + std::distance(childresult.begin(), childresult.end()));
				result.insert(result.end(), childresult.begin(), childresult.end());
			}
		}

		bool insideParentsClientArea = true;

		insideParentsClientArea |= point.x >= clientArea.left();
		if(insideParentsClientArea) insideParentsClientArea = point.y >= clientArea.top();
		if(insideParentsClientArea) insideParentsClientArea = point.x < clientArea.right();
		if(insideParentsClientArea) insideParentsClientArea = point.y < clientArea.bottom();

		if ((point >= ui::UiPoint{}) && (point < frame->area().size()) && !blocking && insideParentsClientArea)
		{
			if (frame->canReceiveMouseMessages())
			{
				result.emplace_back(TargetFramePoint{ frame, point });
			}
			if (frame->blocksMouseMessages())
				blocking = true;
		}

		return result;
	};

	engine::vector<TargetFramePoint> findTargetFrameAndTranslate(Frame* messageFrame, ui::UiPoint point)
	{
		if (ui::dragFrame != nullptr)
		{
			auto rootFrame = translateTowardsRoot(messageFrame, point);
			ui::UiPoint trans{};
			getTranslationToChild(rootFrame, dynamic_cast<Frame*>(ui::dragFrame), trans);
			return { { dynamic_cast<Frame*>(ui::dragFrame), { point.x - trans.x, point.y - trans.y } } };

		}
		else
		{
			bool blocking = false;
			auto rootFrame = translateTowardsRoot(messageFrame, point);
			auto targetFrame = translateFromRoot(rootFrame, point, blocking, 
				ui::UiRect{ 
					static_cast<int>(rootFrame->clientArea().left), 
					static_cast<int>(rootFrame->clientArea().top),
					static_cast<int>(rootFrame->width() - rootFrame->clientArea().left - rootFrame->clientArea().right),
					static_cast<int>(rootFrame->height() - rootFrame->clientArea().top - rootFrame->clientArea().bottom) });
			return targetFrame;
		}
	};


	static Frame* lastMouseDownFrame = nullptr;

	void RootFrame::recreateWindow(Frame* thisFrame, Frame* parent, int width, int height)
	{
		if (parent)
		{
			platform::Window* parentWindow = nullptr;
			Frame* tempParent = parent;
			while (!parentWindow)
			{
				if (tempParent->window())
					parentWindow = tempParent->window();
				else
					tempParent = tempParent->getParent();
			}
			window = engine::make_shared<platform::Window>(parentWindow->native(), width, height, true);
		}
		else
		{
			window = engine::make_shared<platform::Window>("Window", width, height);
		}

		window->setMouseCallbacks(
			[thisFrame](int x, int y)
			{
				auto targetTrans = findTargetFrameAndTranslate(thisFrame, { x, y });

				if (targetTrans.size() > 0)
				{
					bool lastFramePresent = false;
					for (auto&& target : targetTrans)
					{
						target.frame->onMouseMove(target.point.x, target.point.y);
						if (target.frame == lastMouseMoveFrame)
							lastFramePresent = true;
					}
					if (!lastFramePresent && lastMouseMoveFrame)
					{
						auto point = UiPoint{ x, y };
						auto rootFrame = translateTowardsRoot(lastMouseMoveFrame, point);
						ui::UiPoint trans;
						getTranslationToChild(rootFrame, lastMouseMoveFrame, trans);
						lastMouseMoveFrame->onMouseMove(point.x - trans.x, point.y - trans.y);
					}
					
					if (!lastFramePresent && lastMouseMoveFrame)
					{
						auto point = UiPoint{ x, y };
						auto rootFrame = translateTowardsRoot(lastMouseMoveFrame, point);
						ui::UiPoint trans;
						getTranslationToChild(rootFrame, lastMouseMoveFrame, trans);
						lastMouseMoveFrame->onMouseLeave(point.x - trans.x, point.y - trans.y);
					}
					for (auto&& target : targetTrans)
					{
						if (lastMouseMoveFrame != target.frame)
						{
							target.frame->onMouseEnter(target.point.x, target.point.y);
						}
					}


					lastMouseMoveFrame = targetTrans[0].frame;
				}
			},
			[thisFrame](engine::MouseButton btn, int x, int y)
			{
				auto targetTrans = findTargetFrameAndTranslate(thisFrame, { x, y });
				for (auto&& target : targetTrans)
					target.frame->onMouseDown(btn, target.point.x, target.point.y);
				if(targetTrans.size() > 0)
					lastMouseDownFrame = targetTrans[0].frame;
			},
			[thisFrame](engine::MouseButton btn, int x, int y)
			{
				auto targetTrans = findTargetFrameAndTranslate(thisFrame, { x, y });
				for (auto&& target : targetTrans)
					target.frame->onMouseUp(btn, target.point.x, target.point.y);
			},
			[thisFrame](engine::MouseButton btn, int x, int y)
			{
				auto targetTrans = findTargetFrameAndTranslate(thisFrame, { x, y });
				for (auto&& target : targetTrans)
					target.frame->onMouseDoubleClick(btn, target.point.x, target.point.y);
			},
			[thisFrame](int x, int y, int delta)
			{
				auto targetTrans = findTargetFrameAndTranslate(thisFrame, { x, y });
				for (auto&& target : targetTrans)
					target.frame->onMouseWheel(target.point.x, target.point.y, delta);
			});

		window->setKeyboardCallbacks(
			[thisFrame](engine::Key key, engine::ModifierState modState)
			{
				thisFrame->onKeyDown(key, modState);
				// propagate to children
				//for (auto&& child : thisFrame->childs())
				//{
				//	dynamic_pointer_cast<UiEvents>(child)->onKeyDown(key, modState);
				//}
				if (lastMouseDownFrame)
					lastMouseDownFrame->onKeyDown(key, modState);
			},
			[thisFrame](engine::Key key, engine::ModifierState modState)
			{
				thisFrame->onKeyUp(key, modState);
				// propagate to children
				//for (auto&& child : thisFrame->childs())
				//{
				//	dynamic_pointer_cast<UiEvents>(child)->onKeyUp(key, modState);
				//}
				if (lastMouseDownFrame)
					lastMouseDownFrame->onKeyUp(key, modState);
			}
			);

		window->setResizeCallback(
			[this, thisFrame](int width, int height)
			{
				if ((width <= 0) || (height <= 0))
					return;
				thisFrame->size(UiPoint{ width, height });
				//thisFrame->area().width(width);
				//thisFrame->area().height(height);

				if (this->rendering)
				{
					this->rendering->device().waitForIdle();

					auto weakChain = this->rendering->device().currentSwapChain();
					auto swapChain = weakChain.lock();
					if (swapChain)
					{
						this->rendering->releaseSwapChainSRVs();
						swapChain->resize(this->rendering->device(), { static_cast<uint32_t>(this->rendering->device().width()), static_cast<uint32_t>(this->rendering->device().height()) });
						this->rendering->createSwapChainSRVs();
					}
				}

				thisFrame->onResize(width, height);
				thisFrame->invalidate();
			});
	}

	RootFrame::~RootFrame()
	{
		if (rendering)
		{
			rendering->device().shutdown();
			rendering->device().waitForIdle();
			rendering->device().resourceCache().clear();
			rendering->shutdown();
			
			drawImagePipeline = nullptr;
			cmd = nullptr;
			registration = nullptr;
			rendering = nullptr;
		}
	}
}