#pragma once

#include "ui/Frame.h"
#include "engine/Engine.h"
#include "engine/graphics/CommonNoDep.h"

namespace ui
{
	class Label;
}

namespace editor
{
	class EngineEventWrapper;
	class EngineView : public ui::Frame
	{
	public:
		EngineView(Frame* parent, GraphicsApi api, engine::shared_ptr<engine::Scene> scene, const char* name);

		EngineEventWrapper* eventWrapper() { return m_eventWrapper.get(); }

		void update();
		CameraTransform getCameraTransform() const;
		void setCameraTransform(const CameraTransform& transform);
	private:
		engine::shared_ptr<EngineEventWrapper> m_eventWrapper;
		engine::shared_ptr<ui::Label> m_label;
	};
}
