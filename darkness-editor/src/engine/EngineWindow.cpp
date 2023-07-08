#include "EngineWindow.h"
#include "../Tools.h"
#include <QTimer>
#include <QMouseEvent>
#include <QStyle>

EngineClientWindow::EngineClientWindow(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

QPoint EngineClientWindow::titleBarHeightAndMargin() const
{
    int titleBarMargin = style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);
    int titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight);
    return { titleBarMargin, titleBarHeight };
}

void EngineClientWindow::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    auto border = titleBarHeightAndMargin();
    border.setX(0);
    border.setY(0);
    switch (mouseEvent->button())
    {
    case Qt::MouseButton::LeftButton: { emit onMouseDoubleClick(engine::MouseButton::Left, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    case Qt::MouseButton::RightButton: { emit onMouseDoubleClick(engine::MouseButton::Right, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    case Qt::MouseButton::MiddleButton: { emit onMouseDoubleClick(engine::MouseButton::Center, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    default: break;
    }
}

void EngineClientWindow::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    emit onMouseMove(mouseEvent->pos().x(), mouseEvent->pos().y());
}

void EngineClientWindow::mousePressEvent(QMouseEvent* mouseEvent)
{
    auto border = titleBarHeightAndMargin();
    border.setX(0);
    border.setY(0);
    switch (mouseEvent->button())
    {
    case Qt::MouseButton::LeftButton: { emit onMouseDown(engine::MouseButton::Left, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    case Qt::MouseButton::RightButton: { emit onMouseDown(engine::MouseButton::Right, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    case Qt::MouseButton::MiddleButton: { emit onMouseDown(engine::MouseButton::Center, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    default: break;
    }
}

void EngineClientWindow::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    auto border = titleBarHeightAndMargin();
    border.setX(0);
    border.setY(0);
    switch (mouseEvent->button())
    {
    case Qt::MouseButton::LeftButton: { emit onMouseUp(engine::MouseButton::Left, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    case Qt::MouseButton::RightButton: { emit onMouseUp(engine::MouseButton::Right, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    case Qt::MouseButton::MiddleButton: { emit onMouseUp(engine::MouseButton::Center, mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y()); break; }
    default: break;
    }
}

void EngineClientWindow::wheelEvent(QWheelEvent *mouseEvent)
{
	auto border = titleBarHeightAndMargin();
	border.setX(0);
	border.setY(0);

	QPoint numPixels = mouseEvent->pixelDelta();
	QPoint numDegrees = mouseEvent->angleDelta() / 8;

	int delta = 0;
	if (!numPixels.isNull()) {
		delta = numPixels.y();
	}
	else if (!numDegrees.isNull()) {
		QPoint numSteps = numDegrees / 15;
		delta = numSteps.y();
	}

	mouseEvent->accept();


	emit onMouseWheel(mouseEvent->pos().x() - border.x(), mouseEvent->pos().y() - border.y(), delta);
}

void EngineClientWindow::keyPressEvent(QKeyEvent* keyEvent)
{
    if (!hasFocus())
        return;

    auto engineKey = qtKeyToEngineKey(static_cast<Qt::Key>(keyEvent->key()));
#ifdef _WIN32
    if (engineKey == engine::Key::Unknown)
    {
        auto nativeMods = interpretKeyEvent(keyEvent);

        if ((nativeMods & VK_LSHIFT) == VK_LSHIFT)
            m_modState[engine::KeyModifier::ShiftLeft] = true;

        if ((nativeMods & VK_RSHIFT) == VK_RSHIFT)
            m_modState[engine::KeyModifier::ShiftRight] = true;

        if ((nativeMods & VK_LMENU) == VK_LMENU)
            m_modState[engine::KeyModifier::AltLeft] = true;

        if ((nativeMods & VK_RMENU) == VK_RMENU)
            m_modState[engine::KeyModifier::AltRight] = true;

        if ((nativeMods & VK_LCONTROL) == VK_LCONTROL)
            m_modState[engine::KeyModifier::CtrlLeft] = true;

        if ((nativeMods & VK_RCONTROL) == VK_RCONTROL)
            m_modState[engine::KeyModifier::CtrlRight] = true;
    }
#else
    auto mods = keyEvent->modifiers();
    modState[engine::KeyModifier::ShiftLeft] = (mods & Qt::ShiftModifier) == Qt::ShiftModifier;
    modState[engine::KeyModifier::ShiftRight] = modState[engine::KeyModifier::ShiftLeft];

    modState[engine::KeyModifier::AltLeft] = (mods & Qt::AltModifier) == Qt::AltModifier;
    modState[engine::KeyModifier::AltRight] = modState[engine::KeyModifier::AltLeft];

    modState[engine::KeyModifier::CtrlLeft] = (mods & Qt::ControlModifier) == Qt::ControlModifier;
    modState[engine::KeyModifier::CtrlRight] = modState[engine::KeyModifier::CtrlLeft];
#endif

    emit onKeyDown(engineKey, m_modState);
}

void EngineClientWindow::keyReleaseEvent(QKeyEvent* keyEvent)
{
    if (!hasFocus())
        return;

    auto engineKey = qtKeyToEngineKey(static_cast<Qt::Key>(keyEvent->key()));
#ifdef _WIN32
    if (engineKey == engine::Key::Unknown)
    {
        auto nativeMods = interpretKeyEvent(keyEvent);

        if ((nativeMods & VK_LSHIFT) == VK_LSHIFT)
            m_modState[engine::KeyModifier::ShiftLeft] = false;

        if ((nativeMods & VK_RSHIFT) == VK_RSHIFT)
            m_modState[engine::KeyModifier::ShiftRight] = false;

        if ((nativeMods & VK_LMENU) == VK_LMENU)
            m_modState[engine::KeyModifier::AltLeft] = false;

        if ((nativeMods & VK_RMENU) == VK_RMENU)
            m_modState[engine::KeyModifier::AltRight] = false;

        if ((nativeMods & VK_LCONTROL) == VK_LCONTROL)
            m_modState[engine::KeyModifier::CtrlLeft] = false;

        if ((nativeMods & VK_RCONTROL) == VK_RCONTROL)
            m_modState[engine::KeyModifier::CtrlRight] = false;
    }
#else
    auto mods = keyEvent->modifiers();
    modState[engine::KeyModifier::ShiftLeft] = (mods & Qt::ShiftModifier) == Qt::ShiftModifier;
    modState[engine::KeyModifier::ShiftRight] = modState[engine::KeyModifier::ShiftLeft];

    modState[engine::KeyModifier::AltLeft] = (mods & Qt::AltModifier) == Qt::AltModifier;
    modState[engine::KeyModifier::AltRight] = modState[engine::KeyModifier::AltLeft];

    modState[engine::KeyModifier::CtrlLeft] = (mods & Qt::ControlModifier) == Qt::ControlModifier;
    modState[engine::KeyModifier::CtrlRight] = modState[engine::KeyModifier::CtrlLeft];
#endif
    emit onKeyUp(engineKey, m_modState);
}


EngineWindow::EngineWindow(QWidget* parent, Qt::WindowFlags flags, const QString& shaderRootPath)
    : QDockWidget(parent, flags)
    , m_lastSize{ this->width(), this->height() }
    , m_mainWidget{ new EngineClientWindow(this) }
    , m_mouseLeft{ false }
    , m_mouseRight{ false }
    , m_mouseCenter{ false }
{
    setWindowTitle("Scene");
    setObjectName("Scene");
    
    QObject::connect(
        m_mainWidget.get(), SIGNAL(onMouseMove(int, int)),
        this, SLOT(onMouseMove(int, int)));
    QObject::connect(
        m_mainWidget.get(), SIGNAL(onMouseDown(engine::MouseButton, int, int)),
        this, SLOT(onMouseDown(engine::MouseButton, int, int)));
    QObject::connect(
        m_mainWidget.get(), SIGNAL(onMouseUp(engine::MouseButton, int, int)),
        this, SLOT(onMouseUp(engine::MouseButton, int, int)));
    QObject::connect(
        m_mainWidget.get(), SIGNAL(onMouseDoubleClick(engine::MouseButton, int, int)),
        this, SLOT(onMouseDoubleClick(engine::MouseButton, int, int)));
	QObject::connect(
		m_mainWidget.get(), SIGNAL(onMouseWheel(int, int, int)),
		this, SLOT(onMouseWheel(int, int, int)));

    QObject::connect(
        m_mainWidget.get(), SIGNAL(onKeyDown(engine::Key, const engine::ModifierState&)),
        this, SLOT(onKeyDown(engine::Key, const engine::ModifierState&)));
    QObject::connect(
        m_mainWidget.get(), SIGNAL(onKeyUp(engine::Key, const engine::ModifierState&)),
        this, SLOT(onKeyUp(engine::Key, const engine::ModifierState&)));

    m_engineWrap = engine::make_shared<EngineWrap>(
        m_mainWidget->winId(), this->width(), this->height(), shaderRootPath);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(ping()));
    m_timer->start(1000.0f / 60.0f);

    setWidget(m_mainWidget.get());
}

Engine& EngineWindow::engine()
{
    return m_engineWrap->engine();
}

void EngineWindow::resetCameraSize()
{
    m_engineWrap->resetCameraSize();
}

void EngineWindow::playClicked(bool value)
{
    m_engineWrap->playClicked(value);
}

EngineWindow::~EngineWindow()
{
    m_timer->stop();
}

void EngineWindow::ping()
{
    QSize size = QSize(this->width(), this->height());
    if (m_lastSize != size)
    {
        m_lastSize = size;
        m_engineWrap->refresh();
    }

    auto selectedNode = m_engineWrap->grabNode();
    if (selectedNode)
        emit nodeSelected(selectedNode);
}

void EngineWindow::onMouseMove(int x, int y)
{
    m_engineWrap->onMouseMove(x, y);
}

void EngineWindow::onMouseDown(engine::MouseButton button, int x, int y)
{
    m_engineWrap->onMouseDown(button, x, y);
    if (button == engine::MouseButton::Left)
        m_mouseLeft = true;
    if (button == engine::MouseButton::Center)
        m_mouseCenter = true;
    if (button == engine::MouseButton::Right)
        m_mouseRight = true;

    emit mouseGrabbed(true);
    m_engineWrap->engine().cameraInputActive(true);
}

void EngineWindow::onMouseUp(engine::MouseButton button, int x, int y)
{
    m_engineWrap->onMouseUp(button, x, y);

    if (button == engine::MouseButton::Left)
        m_mouseLeft = false;
    if (button == engine::MouseButton::Center)
        m_mouseCenter = false;
    if (button == engine::MouseButton::Right)
        m_mouseRight = false;

    if (!m_mouseLeft && !m_mouseCenter && !m_mouseRight)
    {
        emit mouseGrabbed(false);
        m_engineWrap->engine().cameraInputActive(false);
    }
}

void EngineWindow::onMouseDoubleClick(engine::MouseButton button, int x, int y)
{
    m_engineWrap->onMouseDoubleClick(button, x, y);
}

void EngineWindow::onMouseWheel(int x, int y, int delta)
{
	m_engineWrap->onMouseWheel(x, y, delta);
}

void EngineWindow::onKeyDown(engine::Key key, const engine::ModifierState& modifierState)
{
    m_engineWrap->onKeyDown(key, modifierState);
}

void EngineWindow::onKeyUp(engine::Key key, const engine::ModifierState& modifierState)
{
    if (key == engine::Key::Delete)
    {
        emit deleteSelected();
    }
    else
        m_engineWrap->onKeyUp(key, modifierState);
}

