#pragma once

#include <QDockWidget>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include "containers/memory.h"
#include "../settings/Settings.h"
#include "HierarchyTreeModel.h"
#include "engine/Engine.h"
#include "plugins/PluginManager.h"

class HierarchyTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit HierarchyTreeView(QWidget* parent = Q_NULLPTR)
        : QTreeView(parent)
    {}
protected:
    void keyPressEvent(QKeyEvent* keyEvent) override;
    void keyReleaseEvent(QKeyEvent* keyEvent) override;

signals:
    void deleteSelected();
    void duplicateSelected();
private:
    engine::ModifierState m_modState;
};

class Hierarchy : public QDockWidget
{
    Q_OBJECT
public:
    explicit Hierarchy(
        const Settings& settings,
        Engine& engine,
        QMainWindow* mainWindow,
        QWidget *parent = Q_NULLPTR,
        Qt::WindowFlags flags = Qt::WindowFlags());

    HierarchyTreeModel& model()
    {
        return *m_sceneModel;
    }

    void beginModelReset()
    {
        m_sceneModel->beginModelReset();
    }
    void endModelReset()
    {
        m_sceneModel->endModelReset();
    }

public slots:
    void treeDirClicked(QModelIndex index);
    void ShowContextMenu(const QPoint& point);
    void expanded(const QModelIndex &index);
    void onNodeSelected(engine::shared_ptr<engine::SceneNode> node);
    void deleteSelected();
    void duplicateSelected();

protected:
    void dropEvent(QDropEvent *dropEvent) override;

signals:
    void processDroppedItem(const QString& sourceFilePath, const QString& targetPath);
    void nodeSelected(engine::shared_ptr<engine::SceneNode> node);

private:
    Engine& m_engine;
    QString m_contentPath;
    QMainWindow* m_mainWindow;
    engine::unique_ptr<HierarchyTreeModel> m_sceneModel;

    engine::unique_ptr<HierarchyTreeView> m_sceneView;

    QModelIndex m_expandIndex;
    QModelIndex m_newFolderIndex;

    engine::PluginManager manager;

    engine::shared_ptr<engine::SceneNode> duplicate(
        engine::shared_ptr<engine::SceneNode> src);
};
