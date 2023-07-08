#pragma once

#include "ui/Frame.h"
#include "engine/Engine.h"

namespace editor
{
	class EngineEventWrapper : public ui::Frame
	{
	public:
		EngineEventWrapper(Frame* parent, GraphicsApi api, engine::shared_ptr<engine::Scene> scene);
		void setDuplicateEventReceiver(ui::UiEvents* receiver);

		void update();
		CameraTransform getCameraTransform() const;
		void setCameraTransform(const CameraTransform& transform);
	protected:
		void onMouseMove(int x, int y) override;
		void onMouseDown(engine::MouseButton button, int x, int y) override;
		void onMouseUp(engine::MouseButton button, int x, int y) override;
		void onMouseDoubleClick(engine::MouseButton button, int x, int y) override;
		void onMouseWheel(int x, int y, int delta) override;
		void onKeyDown(engine::Key key, const engine::ModifierState& modifierState) override;
		void onKeyUp(engine::Key key, const engine::ModifierState& modifierState) override;
		void onPaint(ui::DrawCommandBuffer& cmd) override;
		void onResize(int width, int height) override;
		void onFrameWindowChangeBegin() override;
		void onFrameWindowChangeEnd(engine::shared_ptr<platform::Window> newWindow) override;

	private:
		engine::unique_ptr<Engine> m_engine;
		ui::UiPoint m_lastPosition;
		ui::UiEvents* m_receiver;
		TextureSRV m_renderTarget;

		bool m_inMouseMove;
		bool m_inMouseDown;
		bool m_inMouseUp;
		bool m_inMouseDoubleClick;
		bool m_inMouseWheel;
		bool m_inKeyDown;
		bool m_inKeyUp;
	};
}
