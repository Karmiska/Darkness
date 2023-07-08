#include "StringDrawer.h"
#include <QStyleOption>
#include <QDropEvent>
#include <QMimeData> 
#include <QPainter>
#include <QMouseEvent>
#include "engine/primitives/Vector3.h"
#include "../assets/AssetTools.h"

namespace engine
{
	DroppableLabel::DroppableLabel(QWidget* parent)
		: QLineEdit(parent)
	{
		setAcceptDrops(true);
	}

	void DroppableLabel::dragEnterEvent(QDragEnterEvent *event)
	{
		LOG("dragEnterEvent");
		//if (event->mimeData()->hasFormat("text/plain"))
		event->acceptProposedAction();
	}

	void DroppableLabel::dropEvent(QDropEvent *dropEvent)
	{
		LOG("dropEvent");
		if (dropEvent->mimeData()->hasUrls())
		{
			for(auto&& url : dropEvent->mimeData()->urls())
			{
				setText(assetFilePathUnderProcessed(
					QString::fromStdString(m_contentPath.c_str()), 
					QString::fromStdString(m_processedContentPath.c_str()),
					url.toLocalFile()));
				emit onChanged();
				break;
			}
		}
	}

	void DroppableLabel::dragLeaveEvent(QDragLeaveEvent *event)
	{
		LOG("dragLeaveEvent");
	}

	void DroppableLabel::dragMoveEvent(QDragMoveEvent *event)
	{
		LOG("dragMoveEvent");
	}

	void DroppableLabel::setPaths(const engine::string& contentPath, const engine::string& processedContentPath)
	{
		m_contentPath = contentPath;
		m_processedContentPath = processedContentPath;
	}

    StringWidget::StringWidget(
        Property& value,
        QWidget *parent,
        Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_value{ value }
        , m_layout { engine::make_unique<QHBoxLayout>() }
        , m_nameLabel{ engine::make_unique<QLabel>(this) }
        , m_xLine{ engine::make_unique<DroppableLabel>(this) }
        , m_xLabel{ engine::make_unique<DragableLabel>(*m_xLine.get(), this) }
        , m_canReceiveUpdate{ true }
        , m_valueValid{ true }
    {

        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(2, 0, 2, 0));

        m_xLabel->setMaximumWidth(150);
        m_xLine->setMaximumWidth(60);

        m_nameLabel->sizePolicy().setHorizontalStretch(QSizePolicy::Expanding);

        m_layout->addWidget(m_nameLabel.get());
        m_layout->addWidget(m_xLabel.get());
        m_layout->addWidget(m_xLine.get());

        m_nameLabel->setText(value.name().data());
        m_xLabel->setText("   ");

        m_xLine->setText(value.value<engine::string>().data());

        QObject::connect(
            m_xLine.get(), SIGNAL(editingFinished()),
            this, SLOT(onEditFinished()));

		QObject::connect(
			m_xLine.get(), SIGNAL(onChanged()),
			this, SLOT(onEditFinished()));

        value.registerForChangeNotification(this, [this]() { if (m_canReceiveUpdate) m_xLine->setText(m_value.value<engine::string>().data()); });
        value.registerForRemovalNotification(this, [this]() { this->m_valueValid = false; });
    }

    StringWidget::~StringWidget()
    {
        if (m_valueValid)
        {
            m_value.unregisterForChangeNotification(this);
            m_value.unregisterForRemovalNotification(this);
        }
    }

    void StringWidget::updateProperty()
    {
        m_canReceiveUpdate = false;
        m_value.value<engine::string>(m_xLine->text().toStdString().c_str());
        m_canReceiveUpdate = true;
    }

    void StringWidget::onEditFinished()
    {
        updateProperty();
    }

	void StringWidget::setPaths(const engine::string& contentPath, const engine::string& processedContentPath)
	{
		m_contentPath = contentPath;
		m_processedContentPath = processedContentPath;
		m_xLine->setPaths(contentPath, processedContentPath);
	}

    StringDrawer::StringDrawer(Property& value)
        : m_value{ value }
        , m_widget{ nullptr }
    {
    }

    void StringDrawer::setParent(void* parent)
    {
        if (parent)
        {
            m_parent = reinterpret_cast<QWidget*>(parent);

            m_widget = engine::make_shared<StringWidget>(m_value, m_parent);
            m_widget->setParent(m_parent);
            m_widget->resize(m_parent->width(), 15);
            m_widget->setMaximumHeight(15);
            m_widget->setMinimumWidth(15);
            std::static_pointer_cast<StringWidget>(m_widget)->setPaths(m_contentPath, m_processedContentPath);
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

	void StringDrawer::setPaths(const engine::string& contentPath, const engine::string& processedContentPath)
	{
		m_contentPath = contentPath;
		m_processedContentPath = processedContentPath;
		if (m_widget)
			std::static_pointer_cast<StringWidget>(m_widget)->setPaths(contentPath, processedContentPath);
	}

    void* StringDrawer::native()
    {
        return m_widget.get();
    }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::string>(Property& value)
    {
        return engine::make_shared<StringDrawer>(value);
    }
}
