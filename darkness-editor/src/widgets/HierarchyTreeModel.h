#pragma once

#include <QAbstractItemModel>
#include "engine/Scene.h"
#include "engine/Engine.h"
#include "containers/memory.h"

class HierarchyTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit HierarchyTreeModel(
        Engine& engine, 
        const QString& contentPath,
        const QString& processedPath,
        QObject* parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
        const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    engine::shared_ptr<engine::SceneNode> node(const QModelIndex& index);
    QModelIndex node(engine::SceneNode* node);
    void removeNode(const QModelIndex& index);

    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList & indexes) const override;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

    void addNode(engine::shared_ptr<engine::SceneNode> node);

    void beginModelReset()
    {
        beginResetModel();
    }
    void endModelReset()
    {
        endResetModel();
    }

    void startRemoveRows(const QModelIndex& parent, int first, int last)
    {
        beginRemoveRows(parent, first, last);
    }
    void stopRemoveRows()
    {
        endRemoveRows();
    }

    void startInsertRows(const QModelIndex& parent, int first, int last)
    {
        beginInsertRows(parent, first, last);
    }
    void stopInsertRows()
    {
        endInsertRows();
    }

private:
    void setupModelData(const QStringList& lines, engine::SceneNode* parent);

    Engine& m_engine;
    QString m_contentPath;
    QString m_processedPath;
};
