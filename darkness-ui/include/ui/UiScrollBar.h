#pragma once

#include "ui/Frame.h"
#include "tools/image/Image.h"
#include "containers/string.h"

namespace ui
{
	class Label;
	class GridImage;

	class ScrollValueIf
	{
	public:
		virtual ~ScrollValueIf() = default;

		virtual float value() const = 0;
		virtual void value(float value) = 0;

		virtual float range() const = 0;
		virtual void range(float value) = 0;

		virtual float viewRange() const = 0;
		virtual void viewRange(float value) = 0;
	};

	struct ScrollValues
	{
		float value;
		float range;
		float viewSize;
	};

	class UiScrollBarHandle : public ui::Frame,
							  public ScrollValueIf
	{
	public:
		UiScrollBarHandle(Frame* parent);
		void setHover(bool hover);

		float value() const override;
		void value(float value) override;

		float range() const override;
		void range(float value) override;

		float viewRange() const override;
		void viewRange(float value) override;

		std::function<void(float)> onScrollValueChange;
	protected:
		void onPaint(DrawCommandBuffer& cmd) override;
		void onMouseEnter(int /*x*/, int /*y*/) override;
		void onMouseLeave(int /*x*/, int /*y*/) override;
		void onDragMove(ui::UiPoint) override;

	private:
		engine::shared_ptr<GridImage> m_scrollbarThemeImages;
		engine::shared_ptr<GridImage> m_scrollbarHoverThemeImages;
		void loadFrameImages();

		ScrollValues m_scrollValues;

		bool m_hover;
	};

	class UiScrollBar : public ui::Frame,
						public ScrollValueIf
	{
	public:
		UiScrollBar(Frame* parent);

		void onMouseMove (int /*x*/, int /*y*/) override;
		void onMouseWheel(int /*x*/, int /*y*/, int /*delta*/) override;
		void onResize(int /*width*/, int /*height*/) override;

		float value() const override;
		void value(float value) override;

		float range() const override;
		void range(float value) override;

		float viewRange() const override;
		void viewRange(float value) override;

		std::function<void(float)> onScrollValueChange;

	protected:
		
		virtual void onHandleMove(const ScrollValues&) {};
		engine::shared_ptr<UiScrollBarHandle> m_handle;

		void updateHandle();
	};
}
