#include "TextureTypeDrawer.h"
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include "engine/primitives/Vector3.h"

namespace engine
{
    TextureTypeWidget::TextureTypeWidget(
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
        m_values->addItem("Albedo");
        m_values->addItem("Roughness");
        m_values->addItem("Ambient");
        m_values->addItem("Emissive");
        m_values->addItem("Height");
        m_values->addItem("Normal");
        m_values->addItem("Shininess");
        m_values->addItem("Opacity");
        m_values->addItem("Displacement");
        m_values->addItem("Lightmap");
        m_values->addItem("Reflection");
        m_values->addItem("Metalness");
        m_values->addItem("Occlusion");
        m_values->addItem("Hdr");
        
        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(2, 0, 2, 0));
        
        m_xLabel->setMaximumWidth(10);
        //m_values->setMaximumWidth(60);
        
        m_nameLabel->sizePolicy().setHorizontalStretch(QSizePolicy::Expanding);
        
        m_layout->addWidget(m_nameLabel.get());
        m_layout->addWidget(m_xLabel.get());
        m_layout->addWidget(m_values.get());
        
        m_nameLabel->setText(value.name().data());
        m_xLabel->setText("   ");
        
        m_values->setCurrentIndex(static_cast<int>(value.value<TextureType>()));
        
        QObject::connect(
            m_values.get(), SIGNAL(activated(int)),
            this, SLOT(onActivated(int)));
        
        value.registerForChangeNotification(this, [this]() { if (m_canReceiveUpdate) m_values->setCurrentIndex(static_cast<int>(m_value.value<TextureType>())); });
        value.registerForRemovalNotification(this, [this]() { this->m_valueValid = false; });
    }

    TextureTypeWidget::~TextureTypeWidget()
    {
        if (m_valueValid)
        {
            m_value.unregisterForChangeNotification(this);
            m_value.unregisterForRemovalNotification(this);
        }
    }

    void TextureTypeWidget::updateProperty()
    {
        m_canReceiveUpdate = false;
        m_value.value<TextureType>(static_cast<TextureType>(m_values->currentIndex()));
        m_canReceiveUpdate = true;
    }

    void TextureTypeWidget::onActivated(int index)
    {
        updateProperty();
    }

    TextureTypeDrawer::TextureTypeDrawer(Property& value)
        : m_value{ value }
        , m_widget{ nullptr }
    {
    }

    void TextureTypeDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<TextureTypeWidget>(m_value, m_parent);
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

    void* TextureTypeDrawer::native()
    {
        return m_widget.get();
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<TextureType>(Property& value)
    {
        return engine::make_shared<TextureTypeDrawer>(value);
    }
}
