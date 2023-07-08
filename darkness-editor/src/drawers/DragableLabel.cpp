#include "DragableLabel.h"
#include <QMouseEvent>

namespace engine
{
    DragableLabel::DragableLabel(QLineEdit& lineEdit, QWidget* parent,
        Qt::WindowFlags f)
        : QLabel(parent, f)
        , m_lineEdit{ lineEdit }
    {
        setCursor(Qt::SizeHorCursor);
    }

    void DragableLabel::mousePressEvent(QMouseEvent* mouseEvent)
    {
        m_lastMousePosition = mouseEvent->pos();
        grabMouse();
    }

    void DragableLabel::mouseReleaseEvent(QMouseEvent* mouseEvent)
    {
        releaseMouse();
    }

    void DragableLabel::mouseMoveEvent(QMouseEvent* mouseEvent)
    {
        float currentValue = m_lineEdit.text().toFloat();
        QPoint distance = mouseEvent->pos() - m_lastMousePosition;
        currentValue += static_cast<float>(distance.x()) / 10.0f;
        m_lineEdit.setText(QString::number(currentValue));
        m_lastMousePosition = mouseEvent->pos();
        emit m_lineEdit.editingFinished();
    }
}
