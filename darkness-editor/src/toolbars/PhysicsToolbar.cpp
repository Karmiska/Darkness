#include "PhysicsToolBar.h"
#include <QDebug>

PhysicsToolBar::PhysicsToolBar(QWidget *parent)
    : QToolBar(parent)
{
    setWindowTitle("PhysicsToolBar");
    setObjectName("PhysicsToolBar");
    setIconSize(QSize(24, 24));

    createActions();
}

void PhysicsToolBar::createActions()
{
    QAction* move = addAction(QPixmap(":/data/images/play.png"), "Play");
    move->setCheckable(true);
    connect(move, &QAction::triggered, this, &PhysicsToolBar::playClicked);
    m_playButton = (QToolButton*)this->widgetForAction(move);
}

void PhysicsToolBar::toolbarDisabled(bool disabled)
{
    for (auto&& shortcut : m_shortcuts)
    {
        shortcut->setEnabled(!disabled);
    }
}

void PhysicsToolBar::playShortcutClicked()
{
    m_playButton->click();
}

void PhysicsToolBar::playClicked()
{
    if (m_playButton->isChecked())
    {
        emit onPlayClicked(true);
    }
    else
    {
        emit onPlayClicked(false);
    }
}
