#include "EngineWrap.h"
#include "platform/window/Window.h"
#include "containers/memory.h"

using namespace platform;
using namespace platform::implementation;
using namespace engine;

#define MousePositionScale 1

EngineWrap::EngineWrap(WId windowId, int width, int height, const QString& shaderRootPath)
    : m_shaderRootPath{ shaderRootPath.toStdString().c_str() }
    , m_engine{ engine::make_shared<Window>((WindowHandle)windowId, width, height, false), m_shaderRootPath }
{
}

Engine& EngineWrap::engine()
{
    return m_engine;
}

void EngineWrap::refresh()
{
    m_engine.refreshSize();
}

engine::shared_ptr<engine::SceneNode> EngineWrap::grabNode()
{
    return m_engine.grabSelected();
}

void EngineWrap::onMouseMove(int x, int y)
{
    m_engine.onMouseMove(x * MousePositionScale, y * MousePositionScale);
}

void EngineWrap::onMouseDown(MouseButton button, int x, int y)
{
    m_engine.onMouseDown(button, x * MousePositionScale, y * MousePositionScale);
}

void EngineWrap::onMouseUp(MouseButton button, int x, int y)
{
    m_engine.onMouseUp(button, x * MousePositionScale, y * MousePositionScale);
}

void EngineWrap::onMouseDoubleClick(MouseButton button, int x, int y)
{
    m_engine.onMouseDoubleClick(button, x * MousePositionScale, y * MousePositionScale);
}

void EngineWrap::onMouseWheel(int x, int y, int delta)
{
	m_engine.onMouseWheel(x, y, delta);
}

void EngineWrap::onKeyDown(engine::Key key, const engine::ModifierState& modifierState)
{
    m_engine.onKeyDown(key, modifierState);
}

void EngineWrap::onKeyUp(engine::Key key, const engine::ModifierState& modifierState)
{
    m_engine.onKeyUp(key, modifierState);
}

void EngineWrap::resetCameraSize()
{
    m_engine.resetCameraSize();
}

void EngineWrap::playClicked(bool value)
{
    m_engine.playClicked(value);
}
