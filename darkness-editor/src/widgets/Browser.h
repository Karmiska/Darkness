#pragma once

#include <QDockWidget>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QFileIconProvider>
#include <QIcon>
#include <QSlider>
#include <QHBoxLayout>
#include <QMouseEvent>
#include "containers/memory.h"
#include "../settings/Settings.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Quaternion.h"

QT_FORWARD_DECLARE_CLASS(QTimer)

class DarknessIconProvider : public QObject,
                             public QFileIconProvider
{
    Q_OBJECT
public:
    QIcon	icon(IconType type) const override;
    QIcon	icon(const QFileInfo &info) const override;
    QString	type(const QFileInfo &info) const override;
};

class DarknessListView : public QListView
{
    Q_OBJECT
public:
    DarknessListView(QWidget* parent = Q_NULLPTR)
        : QListView{parent}
    {
    };

signals:
    void onDropEvent(QDropEvent *dropEvent);

protected:
    void dropEvent(QDropEvent *dropEvent) override
    {
        emit onDropEvent(dropEvent);
    }
};

class DarknessTreeView : public QTreeView
{
    Q_OBJECT
public:
    DarknessTreeView(QWidget* parent = Q_NULLPTR)
        : QTreeView{ parent }
    {
    };
signals:
    void onDropEvent(QDropEvent *dropEvent);

protected:
    void dropEvent(QDropEvent *dropEvent) override
    {
        emit onDropEvent(dropEvent);
    }
};

class FileViewToolbar : public QWidget
{
    Q_OBJECT
public:
    FileViewToolbar(Settings& settings, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags())
        : QWidget(parent, f)
        , m_settings{ settings }
        , m_spacer{ engine::make_unique<QWidget>() }
        , m_layout{ engine::make_unique<QHBoxLayout>() }
        , m_slider{ engine::make_unique<QSlider>() }
        , m_spacerRight{ engine::make_unique<QWidget>() }
    {
        setLayout(m_layout.get());
        m_layout->setSpacing(0);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(QMargins(0, 0, 0, 0));
        m_layout->addWidget(m_spacer.get());
        m_layout->addWidget(m_slider.get());
        m_layout->addWidget(m_spacerRight.get());

        setFixedHeight(15);
        m_slider->setOrientation(Qt::Orientation::Horizontal);
        m_slider->setFixedSize(QSize(120, 12));
        m_spacerRight->setFixedWidth(7);

        m_slider->setMinimum(0);
        m_slider->setMaximum(10);
        m_slider->setValue(m_settings.browserSettings.thumbnailSize());

        QObject::connect(
            m_slider.get(), SIGNAL(valueChanged(int)),
            this, SLOT(onValueChanged(int)));
    }
    ~FileViewToolbar()
    {
        m_settings.browserSettings.thumbnailSize(m_slider->value());
    }

    int thumbnailSize() const
    {
        return m_slider->value();
    }

private slots:
    void onValueChanged(int value)
    {
        emit thumbnailSizeChanged(value);
    }

signals:
    void thumbnailSizeChanged(int size);

private:
    Settings& m_settings;
    engine::unique_ptr<QWidget> m_spacer;
    engine::unique_ptr<QHBoxLayout> m_layout;
    engine::unique_ptr<QSlider> m_slider;
    engine::unique_ptr<QWidget> m_spacerRight;
};

class Browser : public QDockWidget
{
    Q_OBJECT
public:
    explicit Browser(
        Settings& settings,
        QMainWindow* mainWindow,
        QWidget *parent = Q_NULLPTR,
        Qt::WindowFlags flags = Qt::WindowFlags());
    ~Browser();

    void showEvent(QShowEvent* event);

private slots:
    void treeDirClicked(QModelIndex index);
	void fileClicked(QModelIndex index);
    void ShowContextMenu(const QPoint& point);
    void ShowContextMenuFiles(const QPoint& point);
    void expanded(const QModelIndex &index);
    void dropEventFiles(QDropEvent* dropEvent);
    void dropEventDirectory(QDropEvent* dropEvent);
    void thumbnailSizeChanged(int size);
    void onDirectoryLoaded(const QString &path);
    
signals:
    void processDroppedItems(
        const QList<QString>& sourceFilePath, 
        const QString& targetPath,
        const engine::Vector3f& scale,
        const engine::Quaternionf& rotation,
        const engine::string& preferredEncoding,
		bool flipNormal,
		bool alphaClipped,
		bool performCopy);
    void createCubemap(
        const QList<QString>& sourceFilePath,
        const QList<QString>& targetFilePath);
	void fileSelected(const QString& filePath);

protected:
    void keyPressEvent(QKeyEvent* keyEvent);

private:
    Settings& m_settings;
    QString m_contentPath;
    QString m_processedPath;
    QMainWindow* m_mainWindow;
    engine::unique_ptr<DarknessIconProvider> m_iconProvider;
    engine::unique_ptr<QFileSystemModel> m_fileSystemDirModel;
    engine::unique_ptr<QFileSystemModel> m_fileSystemFileModel;

    engine::unique_ptr<QSplitter> m_mainWidget;
        engine::unique_ptr<DarknessTreeView> m_directoryView;
        engine::unique_ptr<QSplitter> m_fileViewWidget;
            engine::unique_ptr<DarknessListView> m_fileView;
            engine::unique_ptr<FileViewToolbar> m_toolbarFileView;

    QModelIndex m_expandIndex;
    QModelIndex m_newFolderIndex;

    void removeFiles(QList<QString> files);
};
