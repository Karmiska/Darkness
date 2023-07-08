#pragma once

#include "engine/InputEvents.h"
#include "platform/window/Window.h"
#include "ui/DrawCommandBuffer.h"
#include "ui/UiRect.h"

#include "tools/Debug.h"

namespace ui
{
    class UiEvents
    {
    public:
		bool canReceiveMouseMessages() const { return m_canReceiveMouseEvents; }
		void canReceiveMouseMessages(bool val) { m_canReceiveMouseEvents = val; }
		bool blocksMouseMessages() const { return m_blocksMouseEvents; }
		void blocksMouseMessages(bool val) { m_blocksMouseEvents = val; }

		virtual void onMouseMove(int /*x*/, int /*y*/) {};
		virtual void onMouseDown(engine::MouseButton /*button*/, int /*x*/, int /*y*/) {};
		virtual void onMouseUp(engine::MouseButton /*button*/, int /*x*/, int /*y*/) {};
		virtual void onMouseDoubleClick(engine::MouseButton /*button*/, int /*x*/, int /*y*/) {};
		virtual void onMouseWheel(int /*x*/, int /*y*/, int /*delta*/) {};
		virtual void onMouseEnter(int /*x*/, int /*y*/) { /*LOG("Entering: %p\n", this); */ };
		virtual void onMouseLeave(int /*x*/, int /*y*/) { /*LOG("Leaving: %p\n", this); */ };
		virtual void onKeyDown(engine::Key /*key*/, const engine::ModifierState& /*modifierState*/) {};
		virtual void onKeyUp(engine::Key /*key*/, const engine::ModifierState& /*modifierState*/) {};
		virtual void onPaint(DrawCommandBuffer& /*cmd*/) {};
		virtual void onResize(int /*width*/, int /*height*/) {};
		virtual void onFrameWindowChangeBegin() {};
		virtual void onFrameWindowChangeEnd(engine::shared_ptr<platform::Window> /*newWindow*/) {};
		virtual void onMove(int /*x*/, int /*y*/) {};
	private:
		bool m_canReceiveMouseEvents = true;
		bool m_blocksMouseEvents = true;
    };
}
