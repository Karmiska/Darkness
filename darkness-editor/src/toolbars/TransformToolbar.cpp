#include "TransformToolbar.h"
#include <QDebug>

TransformToolBar::TransformToolBar(QWidget *parent)
    : QToolBar(parent)
{
    setWindowTitle("TransformToolbar");
    setObjectName("TransformToolbar");    
    setIconSize(QSize(24, 24));
    
    createActions();
    registerShortcuts();
}

void TransformToolBar::createActions()
{
    QAction* move = addAction(QPixmap(":/data/images/icon_move_24.png"), "Move");
    move->setCheckable(true);
    connect(move, &QAction::triggered, this, &TransformToolBar::moveClicked);
    m_moveButton = (QToolButton*)this->widgetForAction(move);

    QAction* rotate = addAction(QPixmap(":/data/images/icon_rotate_24.png"), "Rotate");
    rotate->setCheckable(true);
    connect(rotate, &QAction::triggered, this, &TransformToolBar::rotateClicked);
    m_rotateButton = (QToolButton*)this->widgetForAction(rotate);

    QAction* resize = addAction(QPixmap(":/data/images/icon_resize_24.png"), "Resize");
    resize->setCheckable(true);
    connect(resize, &QAction::triggered, this, &TransformToolBar::resizeClicked);
    m_resizeButton = (QToolButton*)this->widgetForAction(resize);
}

void TransformToolBar::toolbarDisabled(bool disabled)
{
	for (auto&& shortcut : m_shortcuts)
	{
		shortcut->setEnabled(!disabled);
	}
}

void TransformToolBar::registerShortcuts()
{
    m_shortcuts.push_back(new QShortcut(QKeySequence(Qt::Key_W), this, SLOT(moveShortcutClicked())));
    m_shortcuts.push_back(new QShortcut(QKeySequence(Qt::Key_E), this, SLOT(rotateShortcutClicked())));
    m_shortcuts.push_back(new QShortcut(QKeySequence(Qt::Key_R), this, SLOT(resizeShortcutClicked())));
}

void TransformToolBar::moveShortcutClicked()
{
    m_moveButton->click();
}

void TransformToolBar::rotateShortcutClicked()
{
    m_rotateButton->click();
}

void TransformToolBar::resizeShortcutClicked()
{
    m_resizeButton->click();
}

void TransformToolBar::moveClicked()
{
    if (m_moveButton->isChecked())
    {
        disableOthers(m_moveButton);
        emit onMoveClicked(true);
    }
    else
    {
        emit onMoveClicked(false);
    }
}

void TransformToolBar::rotateClicked()
{
    if (m_rotateButton->isChecked())
    {
        disableOthers(m_rotateButton); 
        emit onRotateClicked(true);
    }
    else
    {
        emit onRotateClicked(false);
    }
}

void TransformToolBar::resizeClicked()
{
    if (m_resizeButton->isChecked())
    {
        disableOthers(m_resizeButton);
        emit onResizeClicked(true);
    }
    else
    {
        emit onResizeClicked(false);
    }
}

void TransformToolBar::disableOthers(QToolButton* active)
{
    if (active == m_moveButton)
    {
        if (m_rotateButton->isChecked()) { m_rotateButton->click(); }
        if (m_resizeButton->isChecked()) { m_resizeButton->click(); }
    }
    else if (active == m_rotateButton)
    {
        if (m_moveButton->isChecked()) { m_moveButton->click(); }
        if (m_resizeButton->isChecked()) { m_resizeButton->click(); }
    }
    else if (active == m_resizeButton)
    {
        if (m_moveButton->isChecked()) { m_moveButton->click(); }
        if (m_rotateButton->isChecked()) { m_rotateButton->click(); }
    }
}
