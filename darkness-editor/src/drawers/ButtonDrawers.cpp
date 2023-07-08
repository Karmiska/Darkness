#include "ButtonDrawers.h"
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include "engine/primitives/Vector3.h"

namespace engine
{
    ButtonPushWidget::ButtonPushWidget(
        Property& value,
        QWidget *parent,
        Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_value{ value }
        , m_layout{ engine::make_unique<QHBoxLayout>() }
        , m_nameLabel{ engine::make_unique<QLabel>(this) }
        , m_button{ engine::make_unique<QPushButton>(this) }
        , m_canReceiveUpdate{ true }
        , m_valueValid{ true }
    {
        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(2, 0, 2, 0));

        m_button->setMaximumWidth(60);

        m_nameLabel->sizePolicy().setHorizontalStretch(QSizePolicy::Expanding);

        m_layout->addWidget(m_nameLabel.get());
        m_layout->addWidget(m_button.get());

        //m_nameLabel->setText(value.name().data());
        m_button->setText(value.name().data());

        QObject::connect(
            m_button.get(), SIGNAL(pressed()),
            this, SLOT(onPressed()));

        value.registerForChangeNotification(this, [this]()
        {
            if (m_canReceiveUpdate)
            {
                this->m_button->setDown(static_cast<bool>(m_value.value<engine::ButtonPush>()));
            }
        });

        value.registerForRemovalNotification(this, [this]() { this->m_valueValid = false; });
    }

    ButtonPushWidget::~ButtonPushWidget()
    {
        if (m_valueValid)
        {
            m_value.unregisterForChangeNotification(this);
            m_value.unregisterForRemovalNotification(this);
        }
    }

    void ButtonPushWidget::updateProperty()
    {
        m_canReceiveUpdate = false;
        m_value.value<engine::ButtonPush>(static_cast<engine::ButtonPush>(m_button->isDown()));
        m_canReceiveUpdate = true;
    }

    void ButtonPushWidget::onPressed()
    {
        updateProperty();
    }

    ButtonToggleWidget::ButtonToggleWidget(
        Property& value,
        QWidget *parent,
        Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_value{ value }
        , m_layout{ engine::make_unique<QHBoxLayout>() }
        , m_nameLabel{ engine::make_unique<QLabel>(this) }
        , m_button{ engine::make_unique<QCheckBox>(this) }
        , m_canReceiveUpdate{ true }
    {
        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(2, 0, 2, 0));

        m_button->setMaximumWidth(60);

        m_nameLabel->sizePolicy().setHorizontalStretch(QSizePolicy::Expanding);

        m_layout->addWidget(m_nameLabel.get());
        m_layout->addWidget(m_button.get());

        m_nameLabel->setText(value.name().data());
        m_button->setChecked(static_cast<bool>(m_value.value<engine::ButtonToggle>()));

        QObject::connect(
            m_button.get(), SIGNAL(toggled(bool)),
            this, SLOT(onToggled(bool)));

        value.registerForChangeNotification(this, [this]()
        {
            if (m_canReceiveUpdate)
            {
                this->m_button->setDown(static_cast<bool>(m_value.value<engine::ButtonToggle>()));
            }
        });
    }

    ButtonToggleWidget::~ButtonToggleWidget()
    {
        m_value.unregisterForChangeNotification(this);
    }

    void ButtonToggleWidget::updateProperty(bool checked)
    {
        m_canReceiveUpdate = false;
        m_value.value<engine::ButtonToggle>(static_cast<engine::ButtonToggle>(checked));
        m_canReceiveUpdate = true;
    }

    void ButtonToggleWidget::onToggled(bool checked)
    {
        updateProperty(checked);
    }


    ButtonPushDrawer::ButtonPushDrawer(Property& value)
        : m_value{ value }
        , m_widget{ nullptr }
    {
    }

    void ButtonPushDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<ButtonPushWidget>(m_value, m_parent);
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

    void* ButtonPushDrawer::native()
    {
        return m_widget.get();
    }

    ButtonToggleDrawer::ButtonToggleDrawer(Property& value)
        : m_value{ value }
        , m_widget{ nullptr }
    {
    }

    void ButtonToggleDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<ButtonToggleWidget>(m_value, m_parent);
            m_widget->setParent(m_parent);
            m_widget->resize(m_parent->width(), 15);
            m_widget->setMaximumHeight(15);
            m_widget->setMinimumWidth(15);
        }
        else
        {
            m_widget->setParent(nullptr);
            m_widget = nullptr;
            m_parent = nullptr;
        }
    }

    void* ButtonToggleDrawer::native()
    {
        return m_widget.get();
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::ButtonPush>(Property& value)
    {
        return engine::make_shared<ButtonPushDrawer>(value);
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::ButtonToggle>(Property& value)
    {
        return engine::make_shared<ButtonToggleDrawer>(value);
    }
}
