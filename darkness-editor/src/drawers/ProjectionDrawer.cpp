#include "ProjectionDrawer.h"
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include "engine/primitives/Vector3.h"

namespace engine
{
    ProjectionWidget::ProjectionWidget(
        Property& value,
        QWidget *parent,
        Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_value{ value }
        , m_layout { engine::make_unique<QHBoxLayout>() }
        , m_nameLabel{ engine::make_unique<QLabel>(this) }
        , m_values{ engine::make_unique<QComboBox>(this) }
        , m_xLabel{ engine::make_unique<QLabel>(this) }
        , m_canReceiveUpdate{ true }
        , m_valueValid{ true }
    {
        m_values->addItem("Perspective");
        m_values->addItem("Orthographic");

        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(2, 0, 2, 0));

        m_xLabel->setMaximumWidth(10);
        m_values->setMaximumWidth(60);

        m_nameLabel->sizePolicy().setHorizontalStretch(QSizePolicy::Expanding);

        m_layout->addWidget(m_nameLabel.get());
        m_layout->addWidget(m_xLabel.get());
        m_layout->addWidget(m_values.get());

        m_nameLabel->setText(value.name().data());
        m_xLabel->setText("   ");

        switch (value.value<Projection>())
        {
        case Projection::Perspective: m_values->setCurrentIndex(0); break;
        case Projection::Orthographic: m_values->setCurrentIndex(1); break;
        }

        QObject::connect(
            m_values.get(), SIGNAL(activated(int)),
            this, SLOT(onActivated(int)));

        value.registerForChangeNotification(this, [this]()
        {
            if (m_canReceiveUpdate)
            {
                switch (m_value.value<Projection>())
                {
                case Projection::Perspective: m_values->setCurrentIndex(0); break;
                case Projection::Orthographic: m_values->setCurrentIndex(1); break;
                }
            }
        });
        value.registerForRemovalNotification(this, [this]() { this->m_valueValid = false; });
    }

    ProjectionWidget::~ProjectionWidget()
    {
        if (m_valueValid)
        {
            m_value.unregisterForChangeNotification(this);
            m_value.unregisterForRemovalNotification(this);
        }
    }

    void ProjectionWidget::updateProperty()
    {
        m_canReceiveUpdate = false;
        if(m_values->currentIndex() == 0)
            m_value.value<Projection>(Projection::Perspective);
        else if (m_values->currentIndex() == 1)
            m_value.value<Projection>(Projection::Orthographic);
        m_canReceiveUpdate = true;
    }

    void ProjectionWidget::onActivated(int /*index*/)
    {
        updateProperty();
    }

    ProjectionDrawer::ProjectionDrawer(Property& value)
        : m_value{ value }
        , m_widget{ nullptr }
    {
    }

    void ProjectionDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<ProjectionWidget>(m_value, m_parent);
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

    void* ProjectionDrawer::native()
    {
        return m_widget.get();
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Projection>(Property& value)
    {
        return engine::make_shared<ProjectionDrawer>(value);
    }
}
