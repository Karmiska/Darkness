#include "EngineView.h"
#include "engine/graphics/Device.h"
#include "EngineEventWrapper.h"
#include "ui/Label.h"
#include "ui/UiAnchors.h"

namespace editor
{
	EngineView::EngineView(Frame* parent, GraphicsApi api, engine::shared_ptr<engine::Scene> scene, const char* name)
		: Frame{ 1024, 768, parent->api(), parent }
		, m_eventWrapper{ engine::make_shared<EngineEventWrapper>(this, api, scene) }
		, m_label{ engine::make_shared<ui::Label>(this, name) }
	{
		canResize(true);
		canFocus(true);
		themeSet(true);
		addChild(m_eventWrapper);
		m_eventWrapper->position(3, 21);
		position(100, 100);

		m_label->left(2);
		m_label->top(2);
		m_label->width(116);
		m_label->height(16);
		addChild(m_label);

		addAnchor(ui::UiAnchor{ m_eventWrapper.get(), ui::AnchorType::TopLeft, ui::AnchorType::TopLeft, { 2, 21 } });
		addAnchor(ui::UiAnchor{ m_eventWrapper.get(), ui::AnchorType::BottomRight, ui::AnchorType::BottomRight, { -2, -2 } });

	}

	void EngineView::update()
	{
		m_eventWrapper->update();
	}

	CameraTransform EngineView::getCameraTransform() const
	{
		return m_eventWrapper->getCameraTransform();
	}

	void EngineView::setCameraTransform(const CameraTransform& transform)
	{
		m_eventWrapper->setCameraTransform(transform);
	}
}
