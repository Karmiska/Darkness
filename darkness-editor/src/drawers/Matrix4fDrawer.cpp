#include "Matrix4fDrawer.h"
#include <QStyleOption>
#include <QPainter>

namespace engine
{
    Matrix4fWidget::Matrix4fWidget(
        Property& value,
        bool matrix,
        QWidget *parent,
        Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_value{ value }
        , m_matrix{ matrix }
        , m_layout{ engine::make_unique<QHBoxLayout>() }
        , m_nameLabel{ engine::make_unique<QLabel>(this) }
        , m_xLine{ engine::make_unique<QLineEdit>(this) }
        , m_yLine{ engine::make_unique<QLineEdit>(this) }
        , m_zLine{ engine::make_unique<QLineEdit>(this) }
        , m_xLabel{ engine::make_unique<DragableLabel>(*m_xLine.get(), this) }
        , m_yLabel{ engine::make_unique<DragableLabel>(*m_yLine.get(), this) }
        , m_zLabel{ engine::make_unique<DragableLabel>(*m_zLine.get(), this) }
        , m_canReceiveUpdate{ true }
        , m_valueValid{ true }
    {
        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(2, 0, 2, 0));

        m_xLabel->setMaximumWidth(10);
        m_yLabel->setMaximumWidth(10);
        m_zLabel->setMaximumWidth(10);

        m_xLine->setMaximumWidth(60);
        m_yLine->setMaximumWidth(60);
        m_zLine->setMaximumWidth(60);

        m_layout->addWidget(m_nameLabel.get());
        m_layout->addWidget(m_xLabel.get());
        m_layout->addWidget(m_xLine.get());
        m_layout->addWidget(m_yLabel.get());
        m_layout->addWidget(m_yLine.get());
        m_layout->addWidget(m_zLabel.get());
        m_layout->addWidget(m_zLine.get());

        m_nameLabel->setText(value.name().data());
        m_xLabel->setText(" X ");
        m_yLabel->setText(" Y ");
        m_zLabel->setText(" Z ");

        Vector3f axis;
        if (m_matrix)
        {
            axis = value.value<Matrix4f>().toEulerAngles();
        }
        else
        {
            axis = value.value<Quaternionf>().toEulerAngles();
        }

        m_xLine->setText(QString::number(axis.x));
        m_yLine->setText(QString::number(axis.y));
        m_zLine->setText(QString::number(axis.z));

        QObject::connect(
            m_xLine.get(), SIGNAL(editingFinished()),
            this, SLOT(onXEditFinished()));

        QObject::connect(
            m_yLine.get(), SIGNAL(editingFinished()),
            this, SLOT(onYEditFinished()));

        QObject::connect(
            m_zLine.get(), SIGNAL(editingFinished()),
            this, SLOT(onZEditFinished()));

        value.registerForChangeNotification(this, [this]() 
        {
            if (m_canReceiveUpdate)
            {
                Vector3f axis;
                if (m_matrix)
                {
                    axis = m_value.value<Matrix4f>().toEulerAngles();
                }
                else
                {
                    axis = m_value.value<Quaternionf>().toEulerAngles();
                }

                m_xLine->setText(QString::number(axis.x));
                m_yLine->setText(QString::number(axis.y));
                m_zLine->setText(QString::number(axis.z));
            }
        });
        value.registerForRemovalNotification(this, [this]() { this->m_valueValid = false; });
    }

    Matrix4fWidget::~Matrix4fWidget()
    {
        if (m_valueValid)
        {
            m_value.unregisterForChangeNotification(this);
            m_value.unregisterForRemovalNotification(this);
        }
    }

    void Matrix4fWidget::updateProperty()
    {
        m_canReceiveUpdate = false;
        if (m_matrix)
        {
            Matrix4f rotation = Matrix4f::rotation(m_xLine->text().toFloat(), m_yLine->text().toFloat(), m_zLine->text().toFloat());
            m_value.value<Matrix4f>(rotation);
        }
        else
        {
            Quaternionf qua = Quaternionf::fromEulerAngles(
                m_xLine->text().toFloat(),
                m_yLine->text().toFloat(),
                m_zLine->text().toFloat());
            m_value.value<Quaternionf>(qua);
        }
        m_canReceiveUpdate = true;
    }

    void Matrix4fWidget::onXEditFinished()
    {
        updateProperty();
    }

    void Matrix4fWidget::onYEditFinished()
    {
        updateProperty();
    }

    void Matrix4fWidget::onZEditFinished()
    {
        updateProperty();
    }

    Matrix4fDrawer::Matrix4fDrawer(Property& value, bool matrix)
        : m_value{ value }
        , m_matrix{ matrix }
        , m_widget{ nullptr }
    {
    }

    void Matrix4fDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<Matrix4fWidget>(m_value, m_matrix, m_parent);
            m_widget->setParent(m_parent);
            m_widget->resize(m_parent->width(), 15);
            m_widget->setMaximumHeight(15);
            m_widget->setMinimumWidth(280);
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

    void* Matrix4fDrawer::native()
    {
        return m_widget.get();
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Matrix4f>(Property& value)
    {
        return engine::make_shared<Matrix4fDrawer>(value, true);
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Quaternionf>(Property& value)
    {
        return engine::make_shared<Matrix4fDrawer>(value, false);
    }
}

