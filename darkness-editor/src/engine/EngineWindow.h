#pragma once

#include <QDockWidget>
#include <QWidget>
#include "containers/memory.h"
#include "EngineWrap.h"
#include "engine/Engine.h"

QT_FORWARD_DECLARE_CLASS(QTimer)

class EngineClientWindow : public QWidget
{
    Q_OBJECT
public:
    EngineClientWindow(QWidget* parent = Q_NULLPTR);

private:
    QPoint titleBarHeightAndMargin() const;
    engine::ModifierState m_modState;

protected:
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent) override;
    void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    void mousePressEvent(QMouseEvent* mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
    void keyPressEvent(QKeyEvent* keyEvent) override;
    void keyReleaseEvent(QKeyEvent* keyEvent) override;
	void wheelEvent(QWheelEvent *mouseEvent) override;

signals:
    void onMouseMove(int x, int y);
    void onMouseDown(engine::MouseButton button, int x, int y);
    void onMouseUp(engine::MouseButton button, int x, int y);
    void onMouseDoubleClick(engine::MouseButton button, int x, int y);
	void onMouseWheel(int x, int y, int delta);

    void onKeyDown(engine::Key key, const engine::ModifierState& modifierState);
    void onKeyUp(engine::Key key, const engine::ModifierState& modifierState);
};


class EngineWindow : public QDockWidget
{
    Q_OBJECT
public:
    explicit EngineWindow(
        QWidget *parent = Q_NULLPTR, 
        Qt::WindowFlags flags = Qt::WindowFlags(),
        const QString& shaderRootPath = "");

    Engine& engine();
    ~EngineWindow();

    void resetCameraSize();
    void playClicked(bool value);
private:
    engine::shared_ptr<QWidget> m_mainWidget;
    engine::shared_ptr<EngineWrap> m_engineWrap;
    QTimer* m_timer;
    QSize m_lastSize;

signals:
    void mouseGrabbed(bool grabbed);
    void nodeSelected(engine::shared_ptr<engine::SceneNode> node);
    void deleteSelected();

public slots:
    void ping();
    void onMouseMove(int x, int y);
    void onMouseDown(engine::MouseButton button, int x, int y);
    void onMouseUp(engine::MouseButton button, int x, int y);
    void onMouseDoubleClick(engine::MouseButton button, int x, int y);
	void onMouseWheel(int x, int y, int delta);

    void onKeyDown(engine::Key key, const engine::ModifierState& modifierState);
    void onKeyUp(engine::Key key, const engine::ModifierState& modifierState);

private:
    bool m_mouseLeft;
    bool m_mouseRight;
    bool m_mouseCenter;
};
