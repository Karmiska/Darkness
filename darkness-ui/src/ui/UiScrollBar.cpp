#include "ui/UiScrollBar.h"
#include "ui/Theme.h"
#include "engine/graphics/Device.h"
#include "ui/UiTools.h"

namespace ui
{
	UiScrollBarHandle::UiScrollBarHandle(Frame* parent)
		: Frame{ 7, 10, parent->api(), parent }
		, m_hover{ false }
		, m_scrollValues{}
	{
		backgroundColor(ui::Theme::instance().color("scrollbar_backgroundColor").xyz());
		loadFrameImages();
		canResize(false);
		minimumSize({ 7, 25 });
		canMove(ui::AllowedMovement::Vertical);
		setMinimumPosition({ 1, 1 });
		setMaximumPosition({ 1, 10 });
	}

	void UiScrollBarHandle::setHover(bool hover)
	{
		m_hover = hover;
	}

	void UiScrollBarHandle::onMouseEnter(int /*x*/, int /*y*/)
	{
		setHover(true);
	}

	void UiScrollBarHandle::onMouseLeave(int /*x*/, int /*y*/)
	{
		setHover(false);
	}

	float UiScrollBarHandle::value() const
	{
		return m_scrollValues.value;
	}

	void UiScrollBarHandle::value(float value)
	{
		bool change = m_scrollValues.value != value;
		m_scrollValues.value = value;
		if (change)
			onScrollValueChange(value);
	}
	float UiScrollBarHandle::range() const
	{
		return m_scrollValues.range;
	}
	void UiScrollBarHandle::range(float value)
	{
		m_scrollValues.range = value;
	}
	float UiScrollBarHandle::viewRange() const
	{
		return m_scrollValues.viewSize;
	}
	void UiScrollBarHandle::viewRange(float value)
	{
		m_scrollValues.viewSize = value;
	}

	void UiScrollBarHandle::onDragMove(UiPoint pt)
	{
		float handleHeight = height();
		float scollBarHeight = static_cast<UiScrollBar*>(parent())->height();
		float drawHeight = scollBarHeight - 2;
		float emptySpace = drawHeight - handleHeight - 1;
		value(max(min(static_cast<float>(y() - 1) / emptySpace, 1.0f), 0.0f));
	}

	void UiScrollBarHandle::loadFrameImages()
	{
		m_scrollbarThemeImages = engine::make_shared<GridImage>(device(), "scroll_handle");
		m_scrollbarHoverThemeImages = engine::make_shared<GridImage>(device(), "scroll_handle_hover");
	}

	void UiScrollBarHandle::onPaint(DrawCommandBuffer& cmd)
	{
		
		if (!m_hover)
			m_scrollbarThemeImages->draw(cmd, 0, 0, width(), height(),
				GridImages::TopLeft | GridImages::TopCenter | GridImages::TopRight |
				GridImages::MiddleLeft | GridImages::MiddleCenter | GridImages::MiddleRight |
				GridImages::BottomLeft | GridImages::BottomCenter | GridImages::BottomRight);
		else
			m_scrollbarHoverThemeImages->draw(cmd, 0, 0, width(), height(),
				GridImages::TopLeft | GridImages::TopCenter | GridImages::TopRight |
				GridImages::MiddleLeft | GridImages::MiddleCenter | GridImages::MiddleRight |
				GridImages::BottomLeft | GridImages::BottomCenter | GridImages::BottomRight);
	}

	UiScrollBar::UiScrollBar(Frame* parent)
		: Frame{ 9, 10, parent->api(), parent }
		, m_handle{ engine::make_shared<UiScrollBarHandle>(this) }
	{
		backgroundColor(ui::Theme::instance().color("scrollbar_backgroundColor").xyz());
		canResize(false);
		canMove(ui::AllowedMovement::None);
		m_handle->onScrollValueChange = [this](float value) { onScrollValueChange(value); };
		updateHandle();
		addChild(m_handle);
	}

	void UiScrollBar::onResize(int width, int height)
	{
		if (width < 9)
		{
			int pint = 1;
		}
		updateHandle();
	}

	void UiScrollBar::onMouseMove(int /*x*/, int /*y*/)
	{

	}

	void UiScrollBar::onMouseWheel(int /*x*/, int /*y*/, int /*delta*/)
	{

	}

	float UiScrollBar::value() const
	{
		return m_handle->value();
	}

	void UiScrollBar::value(float value)
	{
		m_handle->value(value);
		updateHandle();
	}
	float UiScrollBar::range() const
	{
		return m_handle->range();
	}
	void UiScrollBar::range(float value)
	{
		m_handle->range(value);
		updateHandle();
	}
	float UiScrollBar::viewRange() const
	{
		return m_handle->viewRange();
	}
	void UiScrollBar::viewRange(float value)
	{
		m_handle->viewRange(value);
		updateHandle();
	}

	void UiScrollBar::updateHandle()
	{
		auto drawHeight = height() - 2;
		auto handleHeight = max(drawHeight * (viewRange() / range()), static_cast<float>(m_handle->minimumSize().y));
		auto emptySpace = drawHeight - handleHeight;
		auto position = emptySpace * value();
		//if (handleHeight < static_cast<float>(m_handle->minimumSize().y))
		//	position -= static_cast<float>(m_handle->minimumSize().y) - handleHeight;
		m_handle->position(1, 1 + position);
		m_handle->size({ width() - 2, static_cast<int>(handleHeight) });
		m_handle->setMinimumPosition({ 1, 1 });
		m_handle->setMaximumPosition({ 1, drawHeight - m_handle->size().y });
	}
}