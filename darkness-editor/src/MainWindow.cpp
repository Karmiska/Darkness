#include "MainWindow.h"
#include "ColorSwatch.h"
#include "engine/EngineWindow.h"
#include "widgets/Browser.h"
#include "widgets/Hierarchy.h"
#include "widgets/Inspector.h"
#include "toolbars/TransformToolbar.h"
#include "toolbars/PhysicsToolbar.h"
#include "platform/File.h"

#include <QAction>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSignalMapper>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QDebug>
#include <QDockWidget>
#include <QFileInfo>
#include <QDebug>

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

MainWindow::MainWindow(const CustomSizeHintMap& /*customSizeHints*/, const QString& projectPath,
    QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_projectPath{ projectPath }
    , m_settings{ projectPath }
    , m_assetImporter{ engine::make_unique<AssetImporter>(m_settings.contentPathAbsolute(), m_settings.processedAssetsPathAbsolute()) }
    , m_engineWindow{ engine::make_unique<EngineWindow>(this, Qt::WindowFlags(), m_settings.shaderPathAbsolute()) }
    , m_browser{ engine::make_unique<Browser>(m_settings, this, this) }
    , m_hierarchy{ engine::make_unique<Hierarchy>(m_settings, m_engineWindow->engine(), this, this) }
    , m_inspector{ engine::make_unique<Inspector>(m_settings, this, this) }
    , m_decodePopup{ engine::make_unique<AssetDecodePopup>(this) }
{
    setObjectName("MainWindow");
    setWindowTitle("Darkness Editor");
    setTabPosition(Qt::DockWidgetArea::TopDockWidgetArea, QTabWidget::TabPosition::North);
    setTabPosition(Qt::DockWidgetArea::BottomDockWidgetArea, QTabWidget::TabPosition::North);
    setTabPosition(Qt::DockWidgetArea::LeftDockWidgetArea, QTabWidget::TabPosition::North);
    setTabPosition(Qt::DockWidgetArea::RightDockWidgetArea, QTabWidget::TabPosition::North);

    QFile file(":/data/DarknessEditor.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    if (file.isOpen())
    {
        QString styleSheet = QLatin1String(file.readAll());
        setStyleSheet(styleSheet);
    }

    qDebug() << projectPath;
    qDebug() << m_settings.shaderPathAbsolute();

    /*EngineWindow *center = new EngineWindow(
        this, Qt::WindowFlags(),
        m_settings.shaderPathAbsolute());*/

    m_engineWindow->setMinimumSize(100, 100);
    setCentralWidget(m_engineWindow.get());

    //m_dockWidgets.push_back(new Browser(m_settings, this, this));

    QObject::connect(
        m_assetImporter.get(), SIGNAL(assetWorkStarted(QString)),
        this, SLOT(onAssetWorkStarted(QString)));
    QObject::connect(
        m_assetImporter.get(), SIGNAL(assetWorkStatusChange(QString, QString)),
        this, SLOT(onAssetWorkStatusChange(QString, QString)));
    QObject::connect(
        m_assetImporter.get(), SIGNAL(assetWorkProgress(QString, float)),
        this, SLOT(onAssetWorkProgress(QString, float)));
    QObject::connect(
        m_assetImporter.get(), SIGNAL(assetWorkStopped(QString)),
        this, SLOT(onAssetWorkStopped(QString)));
    

    QObject::connect(
        m_browser.get(), SIGNAL(processDroppedItems(const QList<QString>&, const QString&, const engine::Vector3f&, const engine::Quaternionf&, const engine::string&, bool, bool, bool)),
        m_assetImporter.get(), SLOT(processItems(const QList<QString>&, const QString&, const engine::Vector3f&, const engine::Quaternionf&, const engine::string&, bool, bool, bool))
    );

    QObject::connect(
        m_browser.get(), SIGNAL(createCubemap(const QList<QString>&, const QList<QString>&)),
        m_assetImporter.get(), SLOT(createCubemap(const QList<QString>&, const QList<QString>&))
    );

    QObject::connect(
        m_hierarchy.get(), SIGNAL(nodeSelected(engine::shared_ptr<engine::SceneNode>)),
        m_inspector.get(), SLOT(nodeSelected(engine::shared_ptr<engine::SceneNode>))
    );

    QObject::connect(
        m_engineWindow.get(), SIGNAL(nodeSelected(engine::shared_ptr<engine::SceneNode>)),
        m_hierarchy.get(), SLOT(onNodeSelected(engine::shared_ptr<engine::SceneNode>))
    );

    QObject::connect(
        m_engineWindow.get(), SIGNAL(deleteSelected()),
        m_hierarchy.get(), SLOT(deleteSelected())
    );

    QObject::connect(
        m_browser.get(), SIGNAL(fileSelected(const QString&)),
        m_inspector.get(), SLOT(fileSelected(const QString&))
    );

    //m_dockWidgets.push_back(new Hierarchy(m_settings, this, this));

    //m_dockWidgets.push_back(new Inspector(m_settings, this, this));

    setupToolBar();
    setupMenuBar();
    /*setupDockWidgets(customSizeHints);

    statusBar()->showMessage(tr("Status Bar"));*/

    m_decodePopup->move(100, 100);

    auto lastLoadedScenePath = m_settings.lastLoadedScene();
    if (lastLoadedScenePath != "")
    {
        if (engine::fileExists(lastLoadedScenePath.toStdString().c_str()))
        {
            m_hierarchy->beginModelReset();
            m_engineWindow->engine().loadScene(lastLoadedScenePath.toStdString().c_str());
            m_hierarchy->endModelReset();
        }
    }
}

void MainWindow::onAssetWorkStarted(QString sourceFilePath) const
{
    m_decodePopup->addItem(sourceFilePath);
}

void MainWindow::onAssetWorkStatusChange(QString sourceFilePath, QString msg) const
{
    m_decodePopup->setItemStatus(sourceFilePath, msg);
}

void MainWindow::onAssetWorkProgress(QString sourceFilePath, float progress) const
{
    m_decodePopup->setItemProgress(sourceFilePath, progress);
}

void MainWindow::onAssetWorkStopped(QString sourceFilePath) const
{
    m_decodePopup->removeItem(sourceFilePath);
}


/*
void MainWindow::actionTriggered(QAction *action)
{
    qDebug("action '%s' triggered", action->text().toLocal8Bit().data());
}*/

void MainWindow::setupToolBar()
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    auto transformToolbar = new TransformToolBar(this);
    m_toolBars.append(transformToolbar);
    addToolBar(transformToolbar);

    connect(transformToolbar, SIGNAL(onMoveClicked(bool)), this, SLOT(moveClick(bool)));
    connect(transformToolbar, SIGNAL(onRotateClicked(bool)), this, SLOT(rotateClick(bool)));
    connect(transformToolbar, SIGNAL(onResizeClicked(bool)), this, SLOT(resizeClick(bool)));

    connect(m_engineWindow.get(), SIGNAL(mouseGrabbed(bool)), transformToolbar, SLOT(toolbarDisabled(bool)));

    auto physicsToolbar = new PhysicsToolBar(this);
    m_toolBars.append(physicsToolbar);
    addToolBar(physicsToolbar);

    connect(physicsToolbar, SIGNAL(onPlayClicked(bool)), this, SLOT(playClick(bool)));

    /*for (int i = 0; i < 1; ++i) {
        ToolBar *tb = new ToolBar(QString::fromLatin1("Tool Bar %1").arg(i + 1), this);
        m_toolBars.append(tb);
        addToolBar(tb);
    }*/
}

void MainWindow::moveClick(bool value)
{
    if(value)
        qDebug() << "move is now: on";
    else
        qDebug() << "move is now: off";
}

void MainWindow::rotateClick(bool value)
{
    if (value)
        qDebug() << "rotate is now: on";
    else
        qDebug() << "rotate is now: off";
}

void MainWindow::resizeClick(bool value)
{
    if (value)
        qDebug() << "resize is now: on";
    else
        qDebug() << "resize is now: off";
}

void MainWindow::playClick(bool value)
{
    m_engineWindow->playClicked(value);
}

void MainWindow::setupMenuBar()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(tr("New scene"), this, &MainWindow::newScene);
    menu->addAction(tr("Save scene"), this, &MainWindow::saveScene);
    menu->addAction(tr("Load scene"), this, &MainWindow::loadScene);
    menu->addAction(tr("&Quit"), this, &QWidget::close);

    //menu->addAction(tr("Save layout..."), this, &MainWindow::saveLayout);
    //menu->addAction(tr("Load layout..."), this, &MainWindow::loadLayout);
    //menu->addAction(tr("Switch layout direction"), this, &MainWindow::switchLayoutDirection);

    //menu->addSeparator();
    

    /*mainWindowMenu = menuBar()->addMenu(tr("Main window"));

    QAction *action = mainWindowMenu->addAction(tr("Animated docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AnimatedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = mainWindowMenu->addAction(tr("Allow nested docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AllowNestedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = mainWindowMenu->addAction(tr("Allow tabbed docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & AllowTabbedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = mainWindowMenu->addAction(tr("Force tabbed docks"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & ForceTabbedDocks);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = mainWindowMenu->addAction(tr("Vertical tabs"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & VerticalTabs);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    action = mainWindowMenu->addAction(tr("Grouped dragging"));
    action->setCheckable(true);
    action->setChecked(dockOptions() & GroupedDragging);
    connect(action, &QAction::toggled, this, &MainWindow::setDockOptions);

    QMenu *toolBarMenu = menuBar()->addMenu(tr("Tool bars"));
    for (int i = 0; i < toolBars.count(); ++i)
        toolBarMenu->addMenu(toolBars.at(i)->toolbarMenu());

#ifdef Q_OS_OSX
    toolBarMenu->addSeparator();

    action = toolBarMenu->addAction(tr("Unified"));
    action->setCheckable(true);
    action->setChecked(unifiedTitleAndToolBarOnMac());
    connect(action, &QAction::toggled, this, &QMainWindow::setUnifiedTitleAndToolBarOnMac);
#endif

    dockWidgetMenu = menuBar()->addMenu(tr("&Dock Widgets"));*/
}

void MainWindow::newScene()
{
    m_hierarchy->beginModelReset();
    m_engineWindow->engine().scene().clear();
    m_engineWindow->resetCameraSize();
    m_hierarchy->endModelReset();
}

void MainWindow::saveScene()
{
    QString fileName
        = QFileDialog::getSaveFileName(
            this, 
            tr("Save scene"),
            m_settings.contentPathAbsolute());
    if (fileName.isEmpty())
        return;

    m_engineWindow->engine().scene().saveTo(fileName.toStdString().c_str());
}

void MainWindow::loadScene()
{
    QString fileName
        = QFileDialog::getOpenFileName(
            this, 
            tr("Load layout"),
            m_settings.contentPathAbsolute()
        );
    if (fileName.isEmpty())
        return;

    m_hierarchy->beginModelReset();
    m_engineWindow->engine().scene().loadFrom(fileName.toStdString().c_str());
    m_engineWindow->resetCameraSize();
    m_hierarchy->endModelReset();

    m_settings.lastLoadedScene(fileName);
}


/*void MainWindow::setDockOptions()
{
    DockOptions opts;
    QList<QAction*> actions = mainWindowMenu->actions();

    if (actions.at(0)->isChecked())
        opts |= AnimatedDocks;
    if (actions.at(1)->isChecked())
        opts |= AllowNestedDocks;
    if (actions.at(2)->isChecked())
        opts |= AllowTabbedDocks;
    if (actions.at(3)->isChecked())
        opts |= ForceTabbedDocks;
    if (actions.at(4)->isChecked())
        opts |= VerticalTabs;
    if (actions.at(5)->isChecked())
        opts |= GroupedDragging;

    QMainWindow::setDockOptions(opts);
}

void MainWindow::saveLayout()
{
    QString fileName
        = QFileDialog::getSaveFileName(this, tr("Save layout"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QString msg = tr("Failed to open %1\n%2")
            .arg(QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    QByteArray geo_data = saveGeometry();
    QByteArray layout_data = saveState();

    bool ok = file.putChar((uchar)geo_data.size());
    if (ok)
        ok = file.write(geo_data) == geo_data.size();
    if (ok)
        ok = file.write(layout_data) == layout_data.size();

    if (!ok) {
        QString msg = tr("Error writing to %1\n%2")
            .arg(QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

void MainWindow::loadLayout()
{
    QString fileName
        = QFileDialog::getOpenFileName(this, tr("Load layout"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QString msg = tr("Failed to open %1\n%2")
            .arg(QDir::toNativeSeparators(fileName), file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    uchar geo_size;
    QByteArray geo_data;
    QByteArray layout_data;

    bool ok = file.getChar((char*)&geo_size);
    if (ok) {
        geo_data = file.read(geo_size);
        ok = geo_data.size() == geo_size;
    }
    if (ok) {
        layout_data = file.readAll();
        ok = layout_data.size() > 0;
    }

    if (ok)
        ok = restoreGeometry(geo_data);
    if (ok)
        ok = restoreState(layout_data);

    if (!ok) {
        QString msg = tr("Error reading %1").arg(QDir::toNativeSeparators(fileName));
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

class DockWidgetAreaCornerFunctor {
public:
    explicit DockWidgetAreaCornerFunctor(QMainWindow *mw, Qt::Corner c, Qt::DockWidgetArea a)
        : m_mainWindow(mw), m_area(a), m_corner(c) {}

    void operator()() const { m_mainWindow->setCorner(m_corner, m_area); }

private:
    QMainWindow *m_mainWindow;
    Qt::DockWidgetArea m_area;
    Qt::Corner m_corner;
};

static QAction *addCornerAction(const QString &text, QMainWindow *mw, QMenu *menu, QActionGroup *group,
    Qt::Corner c, Qt::DockWidgetArea a)
{
    QAction *result = menu->addAction(text, mw, DockWidgetAreaCornerFunctor(mw, c, a));
    result->setCheckable(true);
    group->addAction(result);
    return result;
}

void MainWindow::setupDockWidgets(const CustomSizeHintMap &customSizeHints)
{
    qRegisterMetaType<QDockWidget::DockWidgetFeatures>();

    QMenu *cornerMenu = dockWidgetMenu->addMenu(tr("Top left corner"));
    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    QAction *cornerAction = addCornerAction(tr("Top dock area"), this, cornerMenu, group, Qt::TopLeftCorner, Qt::TopDockWidgetArea);
    cornerAction->setChecked(true);
    addCornerAction(tr("Left dock area"), this, cornerMenu, group, Qt::TopLeftCorner, Qt::LeftDockWidgetArea);

    cornerMenu = dockWidgetMenu->addMenu(tr("Top right corner"));
    group = new QActionGroup(this);
    group->setExclusive(true);
    cornerAction = addCornerAction(tr("Top dock area"), this, cornerMenu, group, Qt::TopRightCorner, Qt::TopDockWidgetArea);
    cornerAction->setChecked(true);
    addCornerAction(tr("Right dock area"), this, cornerMenu, group, Qt::TopRightCorner, Qt::RightDockWidgetArea);

    cornerMenu = dockWidgetMenu->addMenu(tr("Bottom left corner"));
    group = new QActionGroup(this);
    group->setExclusive(true);
    cornerAction = addCornerAction(tr("Bottom dock area"), this, cornerMenu, group, Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    cornerAction->setChecked(true);
    addCornerAction(tr("Left dock area"), this, cornerMenu, group, Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    cornerMenu = dockWidgetMenu->addMenu(tr("Bottom right corner"));
    group = new QActionGroup(this);
    group->setExclusive(true);
    cornerAction = addCornerAction(tr("Bottom dock area"), this, cornerMenu, group, Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    cornerAction->setChecked(true);
    addCornerAction(tr("Right dock area"), this, cornerMenu, group, Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    dockWidgetMenu->addSeparator();

    static const struct Set {
        const char * name;
        uint flags;
        Qt::DockWidgetArea area;
    } sets[] = {
#ifndef Q_OS_MAC
    { "Black", 0, Qt::LeftDockWidgetArea },
#else
    { "Black", Qt::Drawer, Qt::LeftDockWidgetArea },
#endif
    { "White", 0, Qt::RightDockWidgetArea },
    { "Red", 0, Qt::TopDockWidgetArea },
    { "Green", 0, Qt::TopDockWidgetArea },
    { "Blue", 0, Qt::BottomDockWidgetArea },
    { "Yellow", 0, Qt::BottomDockWidgetArea }
    };
    const int setCount = sizeof(sets) / sizeof(Set);

    const QIcon qtIcon(QPixmap(":/res/qt.png"));
    for (int i = 0; i < setCount; ++i) {

        ColorSwatch *swatch = new ColorSwatch(tr(sets[i].name), this, Qt::WindowFlags(sets[i].flags));
            
        if (i % 2)
            swatch->setWindowIcon(qtIcon);

        QString name = QString::fromLatin1(sets[i].name);
        if (customSizeHints.contains(name))
            swatch->setCustomSizeHint(customSizeHints.value(name));

        if (i == 0)
        {
            addDockWidget(sets[i].area, swatch);
            dockWidgetMenu->addMenu(swatch->colorSwatchMenu());
        }
        else
            addDockWidget(sets[i].area, new Browser(m_projectPath, this, Qt::WindowFlags(sets[i].flags)));
    }

    destroyDockWidgetMenu = new QMenu(tr("Destroy dock widget"), this);
    destroyDockWidgetMenu->setEnabled(false);
    connect(destroyDockWidgetMenu, &QMenu::triggered, this, &MainWindow::destroyDockWidget);

    dockWidgetMenu->addSeparator();
    dockWidgetMenu->addAction(tr("Add dock widget..."), this, &MainWindow::createDockWidget);
    dockWidgetMenu->addMenu(destroyDockWidgetMenu);
}

void MainWindow::switchLayoutDirection()
{
    if (layoutDirection() == Qt::LeftToRight)
        QApplication::setLayoutDirection(Qt::RightToLeft);
    else
        QApplication::setLayoutDirection(Qt::LeftToRight);
}

class CreateDockWidgetDialog : public QDialog
{
public:
    explicit CreateDockWidgetDialog(QWidget *parent = Q_NULLPTR);

    QString enteredObjectName() const { return m_objectName->text(); }
    Qt::DockWidgetArea location() const;

private:
    QLineEdit *m_objectName;
    QComboBox *m_location;
};

CreateDockWidgetDialog::CreateDockWidgetDialog(QWidget *parent)
    : QDialog(parent)
    , m_objectName(new QLineEdit(this))
    , m_location(new QComboBox(this))
{
    setWindowTitle(tr("Add Dock Widget"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("Object name:")), 0, 0);
    layout->addWidget(m_objectName, 0, 1);

    layout->addWidget(new QLabel(tr("Location:")), 1, 0);
    m_location->setEditable(false);
    m_location->addItem(tr("Top"));
    m_location->addItem(tr("Left"));
    m_location->addItem(tr("Right"));
    m_location->addItem(tr("Bottom"));
    m_location->addItem(tr("Restore"));
    layout->addWidget(m_location, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::reject);
    layout->addWidget(buttonBox, 2, 0, 1, 2);
}

Qt::DockWidgetArea CreateDockWidgetDialog::location() const
{
    switch (m_location->currentIndex()) {
    case 0: return Qt::TopDockWidgetArea;
    case 1: return Qt::LeftDockWidgetArea;
    case 2: return Qt::RightDockWidgetArea;
    case 3: return Qt::BottomDockWidgetArea;
    default:
        break;
    }
    return Qt::NoDockWidgetArea;
}

void MainWindow::createDockWidget()
{
    CreateDockWidgetDialog dialog(this);
    if (dialog.exec() == QDialog::Rejected)
        return;

    QDockWidget *dw = new QDockWidget;
    const QString name = dialog.enteredObjectName();
    dw->setObjectName(name);
    dw->setWindowTitle(name);
    dw->setWidget(new QTextEdit);

    Qt::DockWidgetArea area = dialog.location();
    switch (area) {
    case Qt::LeftDockWidgetArea:
    case Qt::RightDockWidgetArea:
    case Qt::TopDockWidgetArea:
    case Qt::BottomDockWidgetArea:
        addDockWidget(area, dw);
        break;
    default:
        if (!restoreDockWidget(dw)) {
            QMessageBox::warning(this, QString(), tr("Failed to restore dock widget"));
            delete dw;
            return;
        }
        break;
    }

    extraDockWidgets.append(dw);
    destroyDockWidgetMenu->setEnabled(true);
    destroyDockWidgetMenu->addAction(new QAction(name, this));
}

void MainWindow::destroyDockWidget(QAction *action)
{
    int index = destroyDockWidgetMenu->actions().indexOf(action);
    delete extraDockWidgets.takeAt(index);
    destroyDockWidgetMenu->removeAction(action);
    action->deleteLater();

    if (destroyDockWidgetMenu->isEmpty())
        destroyDockWidgetMenu->setEnabled(false);
}
*/