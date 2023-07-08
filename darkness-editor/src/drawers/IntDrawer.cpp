#include "IntDrawer.h"
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include "engine/primitives/Vector3.h"

namespace engine
{
    IntWidget::IntWidget(
        Property& value,
        QWidget *parent,
        Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_value{ value }
        , m_layout { engine::make_unique<QHBoxLayout>() }
        , m_nameLabel{ engine::make_unique<QLabel>(this) }
        , m_xLine{ engine::make_unique<QLineEdit>(this) }
        , m_xLabel{ engine::make_unique<DragableLabel>(*m_xLine.get(), this) }
        , m_canReceiveUpdate{ true }
        , m_valueValid{ true }
    {
        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(2, 0, 2, 0));

        m_xLabel->setMaximumWidth(10);
        m_xLine->setMaximumWidth(60);

        m_nameLabel->sizePolicy().setHorizontalStretch(QSizePolicy::Expanding);

        m_layout->addWidget(m_nameLabel.get());
        m_layout->addWidget(m_xLabel.get());
        m_layout->addWidget(m_xLine.get());

        m_nameLabel->setText(value.name().data());
        m_xLabel->setText("   ");

        m_xLine->setText(QString::number(value.value<int>()));

        QObject::connect(
            m_xLine.get(), SIGNAL(editingFinished()),
            this, SLOT(onEditFinished()));

        value.registerForChangeNotification(this, [this]() { if(m_canReceiveUpdate) m_xLine->setText(QString::number(m_value.value<int>())); });
        value.registerForRemovalNotification(this, [this]() { this->m_valueValid = false; });
    }

    IntWidget::~IntWidget()
    {
        if (m_valueValid)
        {
            m_value.unregisterForChangeNotification(this);
            m_value.unregisterForRemovalNotification(this);
        }
    }

    void IntWidget::updateProperty()
    {
        m_canReceiveUpdate = false;
        m_value.value<int>(m_xLine->text().toInt());
        m_canReceiveUpdate = true;
    }

    void IntWidget::onEditFinished()
    {
        updateProperty();
    }

    IntDrawer::IntDrawer(Property& value)
        : m_value{ value }
        , m_widget{ nullptr }
    {
    }

    void IntDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<IntWidget>(m_value, m_parent);
            m_widget->setParent(m_parent);
            m_widget->resize(m_parent->width(), 15);
            m_widget->setMaximumHeight(15);
            m_widget->setMinimumWidth(15);
        }
        else
        {
            if (m_widget)
            {
                m_widget->setParent(nullptr);
                m_widget = nullptr;
            }
            m_parent = nullptr;
        }
    }

    void* IntDrawer::native()
    {
        return m_widget.get();
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<int>(Property& value)
    {
        return engine::make_shared<IntDrawer>(value);
    }
}
