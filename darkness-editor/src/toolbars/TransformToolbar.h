#pragma once

#include <QToolBar>
#include <QToolButton>
#include <QShortcut>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

class TransformToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit TransformToolBar(QWidget *parent);

    void createActions();
    void registerShortcuts();

    void moveClicked();
    void rotateClicked();
    void resizeClicked();

public slots:
    void moveShortcutClicked();
    void rotateShortcutClicked();
    void resizeShortcutClicked();

	void toolbarDisabled(bool disabled);

signals:
    void onMoveClicked(bool value);
    void onRotateClicked(bool value);
    void onResizeClicked(bool value);

private:
    QList<QShortcut*> m_shortcuts;
    QToolButton* m_moveButton;
    QToolButton* m_rotateButton;
    QToolButton* m_resizeButton;

    void disableOthers(QToolButton* active);

};
