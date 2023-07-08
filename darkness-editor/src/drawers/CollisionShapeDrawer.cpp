#include "CollisionShapeDrawer.h"
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include "engine/primitives/Vector3.h"

namespace engine
{
    CollisionShapeWidget::CollisionShapeWidget(
        Property& value,
        QWidget *parent,
        Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_value{ value }
        , m_layout{ engine::make_unique<QHBoxLayout>() }
        , m_nameLabel{ engine::make_unique<QLabel>(this) }
        , m_values{ engine::make_unique<QComboBox>(this) }
        , m_xLabel{ engine::make_unique<QLabel>(this) }
        , m_canReceiveUpdate{ true }
        , m_valueValid{ true }
    {
        m_values->addItem("Plane");
        m_values->addItem("Box");
        m_values->addItem("Capsule");
        m_values->addItem("Sphere");
        m_values->addItem("Cylinder");
        m_values->addItem("Cone");
        m_values->addItem("BvhTriangleMesh");
        m_values->addItem("ConvexHull");
        m_values->addItem("ConvexTriangleMesh");

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

        switch (value.value<CollisionShape>())
        {
        case CollisionShape::Plane: m_values->setCurrentIndex(0); break;
        case CollisionShape::Box: m_values->setCurrentIndex(1); break;
        case CollisionShape::Capsule: m_values->setCurrentIndex(2); break;
        case CollisionShape::Sphere: m_values->setCurrentIndex(3); break;
        case CollisionShape::Cylinder: m_values->setCurrentIndex(4); break;
        case CollisionShape::Cone: m_values->setCurrentIndex(5); break;
        case CollisionShape::BvhTriangleMesh: m_values->setCurrentIndex(6); break;
        case CollisionShape::ConvexHull: m_values->setCurrentIndex(7); break;
        case CollisionShape::ConvexTriangleMesh: m_values->setCurrentIndex(8); break;
        }

        QObject::connect(
            m_values.get(), SIGNAL(activated(int)),
            this, SLOT(onActivated(int)));

        value.registerForChangeNotification(this, [this]()
        {
            if (m_canReceiveUpdate)
            {
                switch (m_value.value<CollisionShape>())
                {
                case CollisionShape::Plane: m_values->setCurrentIndex(0); break;
                case CollisionShape::Box: m_values->setCurrentIndex(1); break;
                case CollisionShape::Capsule: m_values->setCurrentIndex(2); break;
                case CollisionShape::Sphere: m_values->setCurrentIndex(3); break;
                case CollisionShape::Cylinder: m_values->setCurrentIndex(4); break;
                case CollisionShape::Cone: m_values->setCurrentIndex(5); break;
                case CollisionShape::BvhTriangleMesh: m_values->setCurrentIndex(6); break;
                case CollisionShape::ConvexHull: m_values->setCurrentIndex(7); break;
                case CollisionShape::ConvexTriangleMesh: m_values->setCurrentIndex(8); break;
                }
            }
        });
        value.registerForRemovalNotification(this, [this]() { this->m_valueValid = false; });
    }

    CollisionShapeWidget::~CollisionShapeWidget()
    {
        if (m_valueValid)
        {
            m_value.unregisterForChangeNotification(this);
            m_value.unregisterForRemovalNotification(this);
        }
    }

    void CollisionShapeWidget::updateProperty()
    {
        m_canReceiveUpdate = false;
        if (m_values->currentIndex() == 0)
            m_value.value<CollisionShape>(CollisionShape::Plane);
        else if (m_values->currentIndex() == 1)
            m_value.value<CollisionShape>(CollisionShape::Box);
        else if (m_values->currentIndex() == 2)
            m_value.value<CollisionShape>(CollisionShape::Capsule);
        else if (m_values->currentIndex() == 3)
            m_value.value<CollisionShape>(CollisionShape::Sphere);
        else if (m_values->currentIndex() == 4)
            m_value.value<CollisionShape>(CollisionShape::Cylinder);
        else if (m_values->currentIndex() == 5)
            m_value.value<CollisionShape>(CollisionShape::Cone);
        else if (m_values->currentIndex() == 6)
            m_value.value<CollisionShape>(CollisionShape::BvhTriangleMesh);
        else if (m_values->currentIndex() == 7)
            m_value.value<CollisionShape>(CollisionShape::ConvexHull);
        else if (m_values->currentIndex() == 8)
            m_value.value<CollisionShape>(CollisionShape::ConvexTriangleMesh);
        m_canReceiveUpdate = true;
    }

    void CollisionShapeWidget::onActivated(int index)
    {
        updateProperty();
    }

    CollisionShapeDrawer::CollisionShapeDrawer(Property& value)
        : m_value{ value }
        , m_widget{ nullptr }
    {
    }

    void CollisionShapeDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<CollisionShapeWidget>(m_value, m_parent);
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

    void* CollisionShapeDrawer::native()
    {
        return m_widget.get();
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<CollisionShape>(Property& value)
    {
        return engine::make_shared<CollisionShapeDrawer>(value);
    }
}
