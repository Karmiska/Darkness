#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>

namespace engine
{
    class DragableLabel : public QLabel
    {
        Q_OBJECT
    public:
        DragableLabel(QLineEdit& lineEdit, QWidget* parent = Q_NULLPTR,
            Qt::WindowFlags f = Qt::WindowFlags());

    protected:
        void mousePressEvent(QMouseEvent* mouseEvent) override;
        void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
        void mouseMoveEvent(QMouseEvent* mouseEvent) override;

    private:
        QLineEdit& m_lineEdit;
        QPoint m_lastMousePosition;
    };
}
