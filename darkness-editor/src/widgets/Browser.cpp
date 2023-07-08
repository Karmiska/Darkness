#include "Browser.h"
#include "../assets/AssetTools.h"
#include "tools/AssetTools.h"
#include "tools/PathTools.h"
#include "platform/Directory.h"
#include "../widgets/ImportSettings.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QMainWindow>
#include <QMenu>
#include <QHeaderView>
#include <QStandardItem>
#include <QProcess>
#include <QDropEvent>
#include <QMimeData> 
#include <QImage>
#include <QApplication>
#include <QMessageBox>

#include <QMimeDatabase>

using namespace engine;

QIcon DarknessIconProvider::icon(IconType type) const
{
    return QFileIconProvider::icon(type);
}

QIcon DarknessIconProvider::icon(const QFileInfo &info) const
{
    return QIcon();
    QPixmap pixmap;
    if (pixmap.load(info.absoluteFilePath()))
    {
        QIcon icon;
        icon.addPixmap(pixmap);
        return icon;
    }
    return QFileIconProvider::icon(info);
}

QString	DarknessIconProvider::type(const QFileInfo &info) const
{
    return QFileIconProvider::type(info);
}

Browser::Browser(
    Settings& settings,
    QMainWindow* mainWindow,
    QWidget* parent,
    Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
    , m_settings{ settings }
    , m_contentPath{ settings.contentPathAbsolute() }
    , m_processedPath{ settings.processedAssetsPathAbsolute() }
    , m_mainWindow{ mainWindow }
    , m_iconProvider{ engine::make_unique<DarknessIconProvider>() }
    , m_fileSystemDirModel{ engine::make_unique<QFileSystemModel>() }
    , m_fileSystemFileModel{ engine::make_unique<QFileSystemModel>() }
    , m_mainWidget{ engine::make_unique<QSplitter>(this) }
    , m_directoryView{ engine::make_unique<DarknessTreeView>(m_mainWidget.get()) }
    , m_fileViewWidget{ engine::make_unique<QSplitter>(m_mainWidget.get()) }
    , m_fileView{ engine::make_unique<DarknessListView>(m_fileViewWidget.get()) }
    , m_toolbarFileView{ engine::make_unique<FileViewToolbar>(settings) }
{
    setWindowTitle("Project");
    setObjectName("Project");

    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    m_fileView->setFocusProxy(this);

    QObject::connect(
        m_fileSystemDirModel.get(), SIGNAL(directoryLoaded(const QString&)),
        this, SLOT(onDirectoryLoaded(const QString&)));

    m_fileSystemDirModel->setRootPath(m_contentPath);
    m_fileSystemDirModel->setReadOnly(false);
    m_fileSystemDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    m_fileSystemFileModel->setRootPath(m_contentPath);
    m_fileSystemFileModel->setReadOnly(false);
    m_fileSystemFileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    m_fileSystemFileModel->setIconProvider(m_iconProvider.get());


    m_directoryView->setModel(m_fileSystemDirModel.get());
    m_directoryView->setRootIndex(m_fileSystemDirModel->index(m_contentPath + "/"));
    m_directoryView->setRootIsDecorated(true);
    m_directoryView->setHeaderHidden(true);
    m_directoryView->hideColumn(1);
    m_directoryView->hideColumn(2);
    m_directoryView->hideColumn(3);
    m_directoryView->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
    m_directoryView->setDropIndicatorShown(true);

    m_fileView->setModel(m_fileSystemFileModel.get());
    m_fileView->setRootIndex(m_fileSystemFileModel->index(m_contentPath + "/"));
    m_fileView->setViewMode(QListView::ViewMode::IconMode);
    m_fileView->setResizeMode(QListView::ResizeMode::Adjust);
    m_fileView->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    m_fileView->setDropIndicatorShown(true);
    m_fileView->setGridSize(QSize(60, 60));
    m_fileView->setContentsMargins(QMargins(0, 0, 0, 0));
    m_fileView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
    m_fileView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_fileViewWidget->setOrientation(Qt::Orientation::Vertical);
    m_fileViewWidget->setContentsMargins(QMargins(0, 0, 0, 0));
    m_fileViewWidget->addWidget(m_fileView.get());
    m_fileViewWidget->addWidget(m_toolbarFileView.get());

    m_mainWidget->addWidget(m_directoryView.get());
    m_mainWidget->addWidget(m_fileViewWidget.get());

    setWidget(m_mainWidget.get());
    m_mainWindow->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, this);

    QObject::connect(
        m_directoryView.get(), SIGNAL(clicked(QModelIndex)),
        this, SLOT(treeDirClicked(QModelIndex)));

    QObject::connect(
        m_fileView.get(), SIGNAL(clicked(QModelIndex)),
        this, SLOT(fileClicked(QModelIndex)));

    /*QObject::connect(
        m_fileView.get(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(selectionChanged(const QModelIndex&, const QModelIndex&)));*/
    

    m_directoryView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_directoryView.get(), SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

    m_fileView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_fileView.get(), SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenuFiles(const QPoint&)));

    QObject::connect(
        m_directoryView.get(), SIGNAL(expanded(const QModelIndex&)),
        this, SLOT(expanded(const QModelIndex&)));

    QObject::connect(
        m_directoryView.get(), SIGNAL(onDropEvent(QDropEvent*)),
        this, SLOT(dropEventDirectory(QDropEvent*)));

    QObject::connect(
        m_fileView.get(), SIGNAL(onDropEvent(QDropEvent*)),
        this, SLOT(dropEventFiles(QDropEvent*)));

    QObject::connect(
        m_toolbarFileView.get(), SIGNAL(thumbnailSizeChanged(int)),
        this, SLOT(thumbnailSizeChanged(int)));

    thumbnailSizeChanged(m_toolbarFileView->thumbnailSize());
}

void Browser::keyPressEvent(QKeyEvent* keyEvent)
{
    //auto focus = this->focusWidget();
    if (hasFocus() && keyEvent->key() == Qt::Key::Key_Delete)
    {
        QList<QString> filePathsToRemove;
        for (auto selectedIndex : m_fileView->selectionModel()->selectedIndexes())
        {
            filePathsToRemove.append(m_fileSystemFileModel->filePath(selectedIndex));
        }
        removeFiles(filePathsToRemove);
    }
    else if (m_directoryView->hasFocus() && keyEvent->key() == Qt::Key::Key_Delete)
    {
        QList<QString> filePathsToRemove;
        QList<QModelIndex> indexes;
        for (auto selectedIndex : m_directoryView->selectionModel()->selectedRows())
        {
            indexes.append(selectedIndex);
            filePathsToRemove.append(m_fileSystemDirModel->filePath(selectedIndex));
        }
        QString deleteMsg = "You are about to delete\n";
        for (auto toDelete : filePathsToRemove)
        {
            deleteMsg += QFileInfo(toDelete).fileName();
            deleteMsg += "\n";
        }
        auto confirmation = QMessageBox::critical(this, "Are you sure you want to delete files?", deleteMsg, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
        if (confirmation == QMessageBox::StandardButton::Yes)
        {
            for (auto folder : indexes)
            {
                QString folderToRemove = m_fileSystemDirModel->filePath(folder);
                if (QFileInfo(folderToRemove).exists())
                {
                    engine::Directory dir(folderToRemove.toStdString().c_str());
                    dir.remove(true);
                }

                QString filePathUnderProcessed = assetFilePathUnderProcessed(m_contentPath, m_processedPath, folderToRemove);
                if (QFileInfo(filePathUnderProcessed).exists())
                {
                    engine::Directory dir(filePathUnderProcessed.toStdString().c_str());
                    dir.remove(true);
                }

                m_fileSystemDirModel->rmdir(folder);
            }
        }
    }
}

void Browser::removeFiles(QList<QString> files)
{
    if (files.size() > 0)
    {
        QString deleteMsg = "You are about to delete\n";
        for (auto toDelete : files)
        {
            deleteMsg += QFileInfo(toDelete).fileName();
            deleteMsg += "\n";
        }
        auto confirmation = QMessageBox::critical(this, "Are you sure you want to delete files?", deleteMsg, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
        if (confirmation == QMessageBox::StandardButton::Yes)
        {
            for (const auto& pathToRemove : files)
            {
                if (QFileInfo(pathToRemove).exists())
                    QFile::remove(pathToRemove);

                QString filePathUnderProcessed = assetFilePathUnderProcessed(m_contentPath, m_processedPath, pathToRemove);
                if (QFileInfo(filePathUnderProcessed).exists())
                    QFile::remove(filePathUnderProcessed);

                QString settingsFilePathUnderProcessed = QString::fromStdString(engine::pathReplaceExtension(filePathUnderProcessed.toStdString().c_str(), "json").c_str());
                if (QFileInfo(settingsFilePathUnderProcessed).exists())
                    QFile::remove(settingsFilePathUnderProcessed);
            }
        }
    }
}

void Browser::onDirectoryLoaded(const QString &/*path*/)
{
    /*QItemSelectionModel* selection = new QItemSelectionModel(m_fileSystemDirModel.get());
    QModelIndex index;
    for (int col = 0; col < 1; ++col)
    {
        selection->select(index, QItemSelectionModel::Select);
    }
    m_directoryView->setSelectionModel(selection);*/

    //m_directoryView->topLevelWidget();
    //m_directoryView->setCurrentIndex(m_fileSystemDirModel->index(0, 0));
}

void Browser::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    auto sizes = m_mainWidget->sizes();
    int combinedWidth = sizes[0] + sizes[1];
    sizes[0] = m_settings.browserSettings.splitterPosition();
    sizes[1] = combinedWidth - sizes[0];
    m_mainWidget->setSizes(sizes);
}

Browser::~Browser()
{
    auto sizes = m_mainWidget->sizes();
    m_settings.browserSettings.splitterPosition(sizes[0]);
}

void Browser::dropEventDirectory(QDropEvent* dropEvent)
{
    QModelIndex index = m_directoryView->indexAt(dropEvent->pos());
    if (!index.isValid())
        return;

    QList<QPair<QString, QString>> images;
    QList<QPair<QString, QString>> models;

    QList<QPair<QString, QString>> movedFiles;

    if (dropEvent->mimeData()->hasUrls())
    {
        for (auto&& url : dropEvent->mimeData()->urls())
        {
            QString targetFilePath = m_fileSystemDirModel->filePath(index) + QDir::separator() + QFileInfo(url.toLocalFile()).fileName();
            QString nativeSourceFilePath = QDir::toNativeSeparators(url.toLocalFile());
            QString nativeDestinationFilePath = QDir::toNativeSeparators(targetFilePath);

            enum class operation
            {
                COPY,
                MOVE
            };
            operation op{ operation::COPY };

            if (commonPath(nativeSourceFilePath, nativeDestinationFilePath) == m_contentPath)
            {
                op = operation::MOVE;
            }

            // check if we dragged a file from same folder to same folder
            if (nativeSourceFilePath != nativeDestinationFilePath)
            {
                if (engine::isImageFormat(url.toLocalFile().toStdString().c_str()))
                {
                    if(op == operation::COPY)
                        images.append({ url.toLocalFile(), m_fileSystemDirModel->filePath(index) });
                    else if(op == operation::MOVE)
                        movedFiles.append({ url.toLocalFile(), m_fileSystemDirModel->filePath(index) });
                }

                if (engine::isModelFormat(url.toLocalFile().toStdString().c_str()))
                {
                    if (op == operation::COPY)
                        models.append({ url.toLocalFile(), m_fileSystemDirModel->filePath(index) });
                    else if (op == operation::MOVE)
                        movedFiles.append({ url.toLocalFile(), m_fileSystemDirModel->filePath(index) });
                }
            }
        }
    }

    for (auto move : movedFiles)
    {
        if (QFileInfo(move.first).exists())
            QFile::rename(move.first, move.second + QDir::separator() + QFileInfo(move.first).fileName());

        QString filePathUnderProcessedFirst = assetFilePathUnderProcessed(m_contentPath, m_processedPath, move.first);
        QString filePathUnderProcessedSecond = assetFilePathUnderProcessed(m_contentPath, m_processedPath, move.second);

        if (QFileInfo(filePathUnderProcessedFirst).exists())
            QFile::rename(filePathUnderProcessedFirst, filePathUnderProcessedSecond + QDir::separator() + QFileInfo(filePathUnderProcessedFirst).fileName());
    }

    engine::Vector3f scale;
    engine::Quaternionf rotation;
    engine::string preferredEncoding;
	bool flipNormal = false;
	bool alphaClipped = false;
	bool performCopy = true;
    if (images.size() > 0 || models.size() > 0)
    {
        // show import options
        auto importOptions = engine::make_unique<ImportSettings>(images, models, m_mainWindow);
        importOptions->exec();
        scale = importOptions->scale();
        rotation = importOptions->rotation();
        preferredEncoding = importOptions->preferredEncoding();
		flipNormal = importOptions->flipNormal();
		alphaClipped = importOptions->alphaClipped();
    }

    QList<QString> sources;
    QString destination = m_fileSystemDirModel->filePath(index);

    for (auto image : images)
    {
        sources.append(image.first);
        /*emit processDroppedItems(
            image.first,
            image.second,
            scale,
            rotation,
            preferredEncoding);*/
    }

    for (auto model : models)
    {
        sources.append(model.first);
        /*emit processDroppedItem(
            model.first,
            model.second,
            scale,
            rotation,
            preferredEncoding);*/
    }

    emit processDroppedItems(sources, destination, scale, rotation, preferredEncoding, flipNormal, alphaClipped, performCopy);

}

void Browser::dropEventFiles(QDropEvent* dropEvent)
{
    QModelIndex index = m_directoryView->currentIndex();
    QList<QPair<QString, QString>> images;
    QList<QPair<QString, QString>> models;

    QList<QString> sources;
    QString destination = m_fileSystemDirModel->filePath(index);

    if (dropEvent->mimeData()->hasUrls())
    {
        for (auto&& url : dropEvent->mimeData()->urls())
        {
            sources.append(url.toLocalFile());
        }
    }

    engine::Vector3f scale;
    engine::Quaternionf rotation;
    engine::string preferredEncoding;
	bool flipNormal = false;
	bool alphaClipped = false;
	bool performCopy = true;
    //if (images.size() > 0 || models.size() > 0)
    {
        // show import options
        auto importOptions = engine::make_unique<ImportSettings>(images, models, m_mainWindow);
        importOptions->exec();
        scale = importOptions->scale();
        rotation = importOptions->rotation();
        preferredEncoding = importOptions->preferredEncoding();
		flipNormal = importOptions->flipNormal();
		alphaClipped = importOptions->alphaClipped();
    }

    emit processDroppedItems(sources, destination, scale, rotation, preferredEncoding, flipNormal, alphaClipped, performCopy);
}

void Browser::ShowContextMenu(const QPoint& point)
{
    QModelIndex index = m_directoryView->indexAt(point);
    if (index.isValid())
    {
        QPoint globalPos = m_directoryView->mapToGlobal(point);

        QMenu myMenu;
        QMenu* createMenu = myMenu.addMenu("Create");
        createMenu->addAction("Folder");
        createMenu->addSeparator();
        createMenu->addAction("C++ Component");
        createMenu->addAction("Shader");
        createMenu->addSeparator();
        createMenu->addAction("Scene");
        myMenu.addAction("Show in Explorer");
        myMenu.addAction("Delete");

        QAction* selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            if (selectedItem->text().startsWith("Folder"))
            {
                if (!m_directoryView->isExpanded(index))
                {
                    m_expandIndex = index;
                    m_directoryView->expand(index);
                }
                else
                {
                    QModelIndex newFolder = m_fileSystemDirModel->mkdir(index, "New Folder");
                    m_directoryView->edit(newFolder);
                }
            }
            else if (selectedItem->text().startsWith("Delete"))
            {
                QList<QString> filePathsToRemove;
                QList<QModelIndex> indexes;
                for (auto selectedIndex : m_directoryView->selectionModel()->selectedRows())
                {
                    indexes.append(selectedIndex);
                    filePathsToRemove.append(m_fileSystemDirModel->filePath(selectedIndex));
                }
                QString deleteMsg = "You are about to delete\n";
                for (auto toDelete : filePathsToRemove)
                {
                    deleteMsg += QFileInfo(toDelete).fileName();
                    deleteMsg += "\n";
                }
                auto confirmation = QMessageBox::critical(this, "Are you sure you want to delete files?", deleteMsg, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
                if (confirmation == QMessageBox::StandardButton::Yes)
                {
                    for (auto folder : indexes)
                    {
                        QString folderToRemove = m_fileSystemDirModel->filePath(folder);
                        if (QFileInfo(folderToRemove).exists())
                        {
                            engine::Directory dir(folderToRemove.toStdString().c_str());
                            dir.remove(true);
                        }

                        QString filePathUnderProcessed = assetFilePathUnderProcessed(m_contentPath, m_processedPath, folderToRemove);
                        if (QFileInfo(filePathUnderProcessed).exists())
                        {
                            engine::Directory dir(filePathUnderProcessed.toStdString().c_str());
                            dir.remove(true);
                        }

                        m_fileSystemDirModel->rmdir(folder);
                    }
                }
            }
            else if (selectedItem->text().startsWith("Show in Explorer"))
            {
                QString path = m_fileSystemDirModel->filePath(index);
                QStringList args;
                args << "/select," << QDir::toNativeSeparators(path);
                QProcess* process = new QProcess(this);
                process->start("explorer.exe", args);
            }
        }
        else
        {
            qDebug() << "no selection";
        }
    }
    else
    {
        m_directoryView->clearSelection();

        QPoint globalPos = m_directoryView->mapToGlobal(point);

        QMenu myMenu;
        myMenu.addAction("Create New Folder");

        QAction* selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            if (selectedItem->text().startsWith("Create New Folder"))
            {
                QDir path(m_contentPath + QDir::separator() + "New Folder");
                if (!path.exists())
                {
                    path.mkpath(".");
                }
            }
        }
        else
        {
            qDebug() << "no selection";
        }
    }
}

void Browser::expanded(const QModelIndex &index)
{
    if (index == m_expandIndex)
    {
        QModelIndex newFolder = m_fileSystemDirModel->mkdir(index, "New Folder");
        m_directoryView->edit(newFolder);
    }
}

void Browser::treeDirClicked(QModelIndex index)
{
    QString path = m_fileSystemDirModel->fileInfo(index).absoluteFilePath();
    m_fileView->setRootIndex(m_fileSystemFileModel->setRootPath(path));
}

/*void Browser::selectionChanged(const QModelIndex& next, const QModelIndex& prev)
{
    if(!next.isValid())
        emit fileSelected("");
}*/

void Browser::fileClicked(QModelIndex index)
{
	QString path = m_fileSystemFileModel->fileInfo(index).absoluteFilePath();
	emit fileSelected(path);
}

void Browser::thumbnailSizeChanged(int size)
{
    if (size > 0)
    {
        m_fileView->setViewMode(QListView::ViewMode::IconMode);
        m_fileView->setGridSize(QSize(30 + (size * 20), 30 + (size * 20)));
        m_fileView->setIconSize(
            QSize(m_fileView->gridSize().width() - 32,
                m_fileView->gridSize().height() - 35));
        m_fileView->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    }
    else
    {
        m_fileView->setViewMode(QListView::ViewMode::ListMode);
        m_fileView->setGridSize(QSize(m_fileView->width(), 17));
        m_fileView->setIconSize(
            QSize(m_fileView->gridSize().width() - 2,
                m_fileView->gridSize().height() - 2));
        m_fileView->setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    }
}

void Browser::ShowContextMenuFiles(const QPoint& point)
{
    QModelIndex index = m_fileView->indexAt(point);
    if (index.isValid())
    {
        auto sizes = m_mainWidget->sizes();
        QPoint globalPos = m_fileView->mapToGlobal(point);

        QMenu myMenu;
        QMenu* createMenu = myMenu.addMenu("Create");
        createMenu->addAction("Folder");
        createMenu->addSeparator();
        createMenu->addAction("C++ Component");
        createMenu->addAction("Shader");
        createMenu->addSeparator();
        createMenu->addAction("Scene");
        myMenu.addSeparator();
        myMenu.addAction("Show in Explorer");
        myMenu.addAction("Delete");

        auto selectedIndexes = m_fileView->selectionModel()->selectedIndexes();
        if (selectedIndexes.size() == 6)
        {
            // are they all images?
            bool allImages = true;
            for (auto&& ind : selectedIndexes)
            {
                auto filePath = m_fileSystemFileModel->filePath(ind);
                if (!engine::isImageFormat(filePath.toStdString().c_str()))
                {
                    allImages = false;
                    break;
                }
            }
            if (allImages)
            {
                // this could be a cubemap
                myMenu.addAction("Create Cubemap");
            }
        }

        QAction* selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            if (selectedItem->text().startsWith("Show in Explorer"))
            {
                QString path = m_fileSystemFileModel->filePath(index);
                QStringList args;
                args << "/select," << QDir::toNativeSeparators(path);
                QProcess* process = new QProcess(this);
                process->start("explorer.exe", args);
            }
            else if (selectedItem->text().startsWith("Delete"))
            {
                QList<QString> filePathsToRemove;
                for (auto selectedIndex : m_fileView->selectionModel()->selectedIndexes())
                {
                    filePathsToRemove.append(m_fileSystemFileModel->filePath(selectedIndex));
                }
                removeFiles(filePathsToRemove);
            }
            else if (selectedItem->text().startsWith("Create Cubemap"))
            {
                QList<QString> filesForCubemap;
                QList<QString> filesForCubemapProcessed;
                for (auto&& ind : selectedIndexes)
                {
                    auto fileUnderContent = m_fileSystemFileModel->filePath(ind);
                    filesForCubemap.append(fileUnderContent);

                    QString fileUnderProcessed = assetFilePathUnderProcessed(m_contentPath, m_processedPath, fileUnderContent);
                    filesForCubemapProcessed.append(fileUnderProcessed);
                }
                emit createCubemap(filesForCubemap, filesForCubemapProcessed);
            }
            qDebug() << "selected: " << selectedItem->text();
        }
        else
        {
            qDebug() << "no selection";
        }
    }
    else
    {
        QPoint globalPos = m_fileView->mapToGlobal(point);

        QMenu myMenu;
        QMenu* createMenu = myMenu.addMenu("Create");
        createMenu->addAction("Folder");
        createMenu->addSeparator();
        createMenu->addAction("C++ Component");
        createMenu->addAction("Shader");
        createMenu->addSeparator();
        createMenu->addAction("Scene");

        QAction* selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            qDebug() << "selected: " << selectedItem->text();
        }
        else
        {
            qDebug() << "no selection";
        }
    }
}
