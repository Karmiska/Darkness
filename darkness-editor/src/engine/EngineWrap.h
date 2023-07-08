#pragma once

#include <QString>
#include <QWidget>
#include "engine/Engine.h"
#include "engine/InputEvents.h"
#include "containers/memory.h"
#include "containers/string.h"

class EngineWrap
{
public:
    EngineWrap(WId windowId, int width, int height, const QString& shaderRootPath);

    void refresh();

    void onMouseMove(int x, int y);
    void onMouseDown(engine::MouseButton button, int x, int y);
    void onMouseUp(engine::MouseButton button, int x, int y);
    void onMouseDoubleClick(engine::MouseButton button, int x, int y);
	void onMouseWheel(int x, int y, int delta);

    void onKeyDown(engine::Key key, const engine::ModifierState& modifierState);
    void onKeyUp(engine::Key key, const engine::ModifierState& modifierState);

    void resetCameraSize();
    void playClicked(bool value);

    engine::shared_ptr<engine::SceneNode> grabNode();

    Engine& engine();
private:
    engine::string m_shaderRootPath;
    Engine m_engine;
};
