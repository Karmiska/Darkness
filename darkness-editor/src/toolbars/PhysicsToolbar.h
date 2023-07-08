#pragma once

#include <QToolBar>
#include <QToolButton>
#include <QShortcut>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

class PhysicsToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit PhysicsToolBar(QWidget *parent);

    void createActions();
    void registerShortcuts();

    void playClicked();

public slots:
    void playShortcutClicked();

    void toolbarDisabled(bool disabled);

signals:
    void onPlayClicked(bool value);

private:
    QList<QShortcut*> m_shortcuts;
    QToolButton* m_playButton;

};
