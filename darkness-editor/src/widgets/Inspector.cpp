#include "Inspector.h"
#include "components/ImagePropertiesComponent.h"
#include "components/ModelPropertiesComponent.h"
#include "components/RigidBodyComponent.h"
#include "components/TerrainComponent.h"
#include "tools/AssetTools.h"
#include "../assets/AssetTools.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMainWindow>
#include <QMenu>
#include <QHeaderView>
#include <QStandardItem>
#include <QProcess>
#include <QDropEvent>
#include <QMimeData> 
#include <QPainter>
#include <QPainterPath>

using namespace engine;

void drawTextGlobal(void* propertyWidget, engine::Rect rect, const engine::string& str)
{
    reinterpret_cast<PropertyWidget*>(propertyWidget)->drawText(rect, str);
}

PropertyWidget::PropertyWidget(
    engine::PluginProperty typeProperty,
    const QString& /*contentPath*/,
    const QString& /*processedPath*/,
    QWidget *parent,
    Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_typeProperty{ typeProperty }
    , m_layout{ engine::make_unique<QHBoxLayout>() }
    , m_label{ engine::make_unique<QLabel>(this) }
    , m_lineEdit{ engine::make_unique<QLineEdit>(this) }
    , m_dragable{ engine::make_unique<engine::DragableLabel>(*m_lineEdit.get(), this) }
{
    m_drawer.reset(m_typeProperty.createDrawer(this, &drawTextGlobal));

    setLayout(m_layout.get());
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setContentsMargins(QMargins(2, 0, 2, 0));

    m_dragable->setMaximumWidth(10);
    m_lineEdit->setMaximumWidth(60);

    m_label->sizePolicy().setHorizontalStretch(QSizePolicy::Expanding);

    m_layout->addWidget(m_label.get());
    m_layout->addWidget(m_dragable.get());
    m_layout->addWidget(m_lineEdit.get());

    m_label->setText(m_typeProperty.name().data());
    m_dragable->setText("   ");

    m_lineEdit->setText(QString::number(0));

    QObject::connect(
        m_lineEdit.get(), SIGNAL(editingFinished()),
        this, SLOT(onEditFinished()));

    resize(parent->width(), 15);
    setMaximumHeight(15);
    setMinimumWidth(15);
}

void PropertyWidget::updateProperty()
{
    //m_value.value<int>(m_xLine->text().toInt());
}

void PropertyWidget::onEditFinished()
{
    updateProperty();
}

void PropertyWidget::drawText(engine::Rect /*rect*/, const engine::string& /*str*/)
{
    //int test = 1;
}

void PropertyWidget::paintEvent(QPaintEvent* paintEvent)
{
    m_typeProperty.callDraw(m_drawer.get(),
    { 
        paintEvent->rect().x(), 
        paintEvent->rect().y(), 
        paintEvent->rect().width(),
        paintEvent->rect().height()
    });
}

ComponentWidget::ComponentWidget(
    engine::shared_ptr<engine::EngineComponent> component,
    const QString& contentPath,
    const QString& processedPath,
    QWidget* /*parent*/,
    Qt::WindowFlags /*f*/)
    : m_layout{ engine::make_unique<QVBoxLayout>() }
    , m_component{ component }
    , m_componentLabel{ engine::make_unique<QLabel>() }
    , m_contentPath{ contentPath }
    , m_processedPath{ processedPath }
{

    QObject::connect(
        this, SIGNAL(refreshProperties(void)),
        this, SLOT(onRefreshProperties(void)), Qt::QueuedConnection
    );

    m_component->registerForChanges(this, [this]() {
        emit this->refreshProperties();
        //this->clear();
        //this->populate();
    });

    setLayout(m_layout.get());
    m_layout->setSpacing(4);
    m_layout->setMargin(0);
    m_layout->setContentsMargins(QMargins(4, 4, 4, 4));
    m_layout->addWidget(m_componentLabel.get());
    m_componentLabel->setText(component->name().data());

    //int countedHeight = 0;
    populate();
    //resize(QSize(parent->width(), countedHeight));
    //setMaximumHeight(countedHeight);
}

ComponentWidget::ComponentWidget(
    engine::TypeInstance& component,
    const QString& contentPath,
    const QString& processedPath,
    QWidget* /*parent*/,
    Qt::WindowFlags /*f*/)
    : m_layout{ engine::make_unique<QVBoxLayout>() }
    , m_componentLabel{ engine::make_unique<QLabel>() }
    , m_contentPath{ contentPath }
    , m_processedPath{ processedPath }
{
    m_componentInstance = engine::make_shared<engine::TypeInstance>(component);
    setLayout(m_layout.get());
    m_layout->setSpacing(4);
    m_layout->setMargin(0);
    m_layout->setContentsMargins(QMargins(4, 4, 4, 4));
    m_layout->addWidget(m_componentLabel.get());
    m_componentLabel->setText(m_componentInstance->name().data());

    for (auto&& prop : m_componentInstance->properties())
    {
        m_layout->addWidget(new PropertyWidget(prop, contentPath, processedPath, this));
    }
}

void ComponentWidget::onRefreshProperties()
{
    clear();
    populate();
}

void ComponentWidget::populate()
{
    for (auto& prop : m_component->propertyNames())
    {
        auto drawer = m_component->variant(prop).createVariantDrawer();
        drawer->setParent(this);
        drawer->setPaths(m_contentPath.toStdString().c_str(), m_processedPath.toStdString().c_str());

        m_drawers.append(drawer);

        auto newWidget = static_cast<QWidget*>(m_drawers.last()->native());
        m_layout->addWidget(newWidget);

        //countedHeight += newWidget->height();
    }
}

ComponentWidget::~ComponentWidget()
{
    m_component->unregisterForChanges(this);
    clear();
}

void ComponentWidget::clear()
{
    for (auto& drawer : m_drawers)
    {
        auto widget = static_cast<QWidget*>(drawer->native());
        if (widget)
            m_layout->removeWidget(widget);
        drawer->setParent(nullptr);
    }
    m_drawers.clear();
}

void ComponentWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRectF(0.5, 0.5, static_cast<float>(width())-3.0f, static_cast<float>(height())-3.0f), 4, 4);

    QPen pen(QColor(34, 34, 34), 1);
    p.setPen(pen);
    p.drawPath(path);

    QPainterPath innerpath;
    innerpath.addRoundedRect(QRectF(1.5, 1.5, static_cast<float>(width()) - 4.0f, static_cast<float>(height()) - 4.0f), 4, 4);

    QPen peninner(QColor(72, 72, 72), 1);
    p.setPen(peninner);
    p.fillPath(innerpath, QColor(81, 81, 81));
    p.drawPath(innerpath);
}

Inspector::Inspector(
    const Settings& settings,
    QMainWindow* mainWindow,
    QWidget* parent,
    Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
    , m_contentPath{ settings.contentPathAbsolute() }
	, m_processedAssetsPathAbsolute{ settings.processedAssetsPathAbsolute() }
    , m_mainWindow{ mainWindow }
    , m_mainWidget{ engine::make_unique<QWidget>(this) }
    , m_layout{ engine::make_unique<QVBoxLayout>() }
{
    setWindowTitle("Inspector");
    setObjectName("Inspector"); 

    resize(100, height());

    m_mainWidget->setLayout(m_layout.get());
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setContentsMargins(QMargins(0, 0, 0, 0));
    m_layout->setAlignment(Qt::AlignmentFlag::AlignTop);

    m_mainWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_mainWidget.get(), SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

    setWidget(m_mainWidget.get());
    m_mainWindow->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, this);
}

Inspector::~Inspector()
{
    m_components.clear();
}

void Inspector::nodeSelected(engine::shared_ptr<engine::SceneNode> node)
{
    m_selectedNode = node;
    m_components.clear();
    //int hackHeight = 0;
    for (int i = 0; i < node->componentCount(); ++i)
    {
        m_components.append(engine::make_shared<ComponentWidget>(node->component(i), m_contentPath, m_processedAssetsPathAbsolute, this));
        m_layout->addWidget(m_components.last().get());
        //m_layout->setAlignment(m_components.last().get(), Qt::AlignmentFlag::AlignTop);
        //hackHeight += m_components.last().get()->height();
    }
    //m_layout->addSpacing(height() - hackHeight);

    /*for (auto&& component : node->components())
    {
        m_components.append(engine::make_shared<ComponentWidget>(component, this));
        m_layout->addWidget(m_components.last().get());
    }*/
}

engine::shared_ptr<engine::EngineComponent> fileComponentFactory(
	const QString& filepath,
	const QString& contentPath,
	const QString& processedPath)
{
	QFileInfo info(filepath);
	auto ext = info.completeSuffix();
	if (engine::isImageFormat(filepath.toStdString().c_str()))
	{
		return engine::make_shared<engine::ImagePropertiesComponent>(
			assetFilePathUnderProcessed(contentPath, processedPath, filepath).toStdString().c_str());
	}
	else if (engine::isModelFormat(filepath.toStdString().c_str()))
	{
		return engine::make_shared<engine::ModelPropertiesComponent>(
			assetFilePathUnderProcessed(contentPath, processedPath, filepath).toStdString().c_str());
	}
    else if (engine::isPrefabFormat(filepath.toStdString().c_str()))
    {
        // TODO ?
    }
	ASSERT(false, "Unhandled content type");
	return nullptr;
}

void Inspector::fileSelected(const QString& filePath)
{
	m_components.clear();

    if (filePath == "")
        return;

    if ((engine::isImageFormat(filePath.toStdString().c_str())) ||
        (engine::isModelFormat(filePath.toStdString().c_str())))
    {
        m_components.append(engine::make_shared<ComponentWidget>(
            fileComponentFactory(filePath, m_contentPath, m_processedAssetsPathAbsolute),
            m_contentPath,
            m_processedAssetsPathAbsolute, this));
        m_layout->addWidget(m_components.last().get());
    }
}

void Inspector::ShowContextMenu(const QPoint& point)
{
    QPoint globalPos = m_mainWidget->mapToGlobal(point);

    QMenu myMenu;
    myMenu.addAction("RigidBody");
    myMenu.addAction("CollisionShape");
    myMenu.addAction("Terrain");

    QAction* selectedItem = myMenu.exec(globalPos);
    if (selectedItem)
    {
        if (selectedItem->text().startsWith("RigidBody"))
        {
            m_selectedNode->addComponent(engine::make_shared<engine::RigidBodyComponent>());
        }
        else if (selectedItem->text().startsWith("CollisionShape"))
        {
            m_selectedNode->addComponent(engine::make_shared<engine::CollisionShapeComponent>());
        }
        else if (selectedItem->text().startsWith("Terrain"))
        {
            m_selectedNode->addComponent(engine::make_shared<engine::TerrainComponent>());
        }
    }
    else
    {
        qDebug() << "no selection";
    }
}

