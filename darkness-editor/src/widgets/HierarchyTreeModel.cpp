#include "HierarchyTreeModel.h"
#include "components/Transform.h"
#include "tools/AssetTools.h"
#include "tools/PathTools.h"
#include "../assets/AssetTools.h"
#include <QDataStream>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

using namespace engine;
using namespace engine;

HierarchyTreeModel::HierarchyTreeModel(
    Engine& engine, 
    const QString& contentPath,
    const QString& processedPath,
    QObject* parent)
    : QAbstractItemModel(parent)
    , m_engine{ engine }
    , m_contentPath{ contentPath }
    , m_processedPath{ processedPath }
{
}

Qt::DropActions HierarchyTreeModel::supportedDragActions() const
{
    return Qt::DropAction::CopyAction | Qt::DropAction::MoveAction;
}

Qt::DropActions HierarchyTreeModel::supportedDropActions() const
{
    return Qt::DropAction::CopyAction | Qt::DropAction::MoveAction;
}

QStringList HierarchyTreeModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

QMimeData* HierarchyTreeModel::mimeData(const QModelIndexList & indexes) const
{
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    foreach(const QModelIndex& index, indexes)
    {
        if (index.isValid())
        {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }
    QMimeData* mimeData = new QMimeData();
    mimeData->setData("text/uri-list", encodedData);
    return mimeData;
}

void HierarchyTreeModel::addNode(engine::shared_ptr<engine::SceneNode> node)
{
    beginResetModel();
    m_engine.scene().root()->addChild(node);
    endResetModel();
}

bool HierarchyTreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("text/uri-list"))
        return false;

    if (column >= columnCount(parent))
        return false;

    engine::shared_ptr<engine::SceneNode> parentNode;
    if(parent.isValid())
        parentNode = node(parent);
    if (!parentNode)
        parentNode = m_engine.scene().root();

    if (data->hasUrls())
    {
        beginResetModel();
        //beginInsertRows(parent, row, row + data->urls().size());

        for (auto&& url : data->urls())
        {
            if (engine::isModelFormat(QFileInfo(url.toLocalFile()).fileName().toStdString().c_str()))
            {
                engine::shared_ptr<engine::SceneNode> node = engine::make_shared<engine::SceneNode>();
                node->addComponent(engine::make_shared<Transform>());

                node->addComponent(engine::make_shared<MeshRendererComponent>(
                    assetFilePathUnderProcessed(
                        m_contentPath, m_processedPath,
                        url.toLocalFile()).toStdString().c_str(),
                        0
                    ));

                node->addComponent(engine::make_shared<MaterialComponent>());
                node->name(QFileInfo(url.toLocalFile()).fileName().toStdString().c_str());
                parentNode->addChild(node);

            }
            else if (engine::isPrefabFormat(QFileInfo(url.toLocalFile()).fileName().toStdString().c_str()))
            {
                engine::string scenePath = assetFilePathUnderContent(m_contentPath, m_processedPath, url.toLocalFile()).toStdString().c_str();
                engine::Scene scene;
                scene.loadFrom(scenePath);

                if (scene.root()->name() == "" && scene.root()->componentCount() == 0)
                {
                    // this is an actual scene file
                    // let's use it as prefab
                    engine::shared_ptr<engine::SceneNode> node = engine::make_shared<engine::SceneNode>();
                    node->addComponent(engine::make_shared<Transform>());
                    node->name(engine::pathExtractFilenameWithoutExtension(scenePath));

                    for (int i = 0; i < scene.root()->childCount(); ++i)
                    {
                        node->addChild(scene.root()->child(i));
                    }
                    parentNode->addChild(node);
                }
                else
                {
                    parentNode->addChild(std::move(scene.root()));
                }
            }

            
            
        }
        //endInsertRows();
        endResetModel();
    }

    return true;
}

QVariant HierarchyTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    SceneNode* item = static_cast<SceneNode*>(index.internalPointer());

    //return item->component(index.column());
    return QVariant(QString(item->name().data()));
}

engine::shared_ptr<engine::SceneNode> HierarchyTreeModel::node(const QModelIndex& index)
{
    return static_cast<SceneNode*>(index.internalPointer())->shared_from_this();
}

QModelIndex HierarchyTreeModel::node(engine::SceneNode* node)
{
    if (!node->parent())
        return QModelIndex();

    return createIndex(static_cast<int>(node->indexInParent()), 0, node);

    /*auto rows = rowCount();
    for (int i = 0; i < rows; ++i)
    {
        auto ind = index(i, 0);
        SceneNode* item = static_cast<SceneNode*>(ind.internalPointer());
        if (item->id() == node->id())
            return ind;
    }

    return QModelIndex();*/
}

Qt::ItemFlags HierarchyTreeModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
}

QVariant HierarchyTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant();
        //return m_scene.root()->component(section);

    return QVariant();
}

QModelIndex HierarchyTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SceneNode* parentItem;

    if (!parent.isValid())
        parentItem = m_engine.scene().root().get();
    else
        parentItem = static_cast<SceneNode*>(parent.internalPointer());

    SceneNode* childItem = parentItem->child(row).get();
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex HierarchyTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    SceneNode* childItem = static_cast<SceneNode*>(index.internalPointer());
    SceneNode* parentItem = childItem->parent();

    if (!parentItem || parentItem == m_engine.scene().root().get())
    {
        return QModelIndex();
    }

    return createIndex(static_cast<int>(parentItem->indexInParent()), 0, parentItem);
}

void HierarchyTreeModel::removeNode(const QModelIndex& index)
{
    if (index.isValid())
    {
        //beginResetModel();
        SceneNode* childItem = static_cast<SceneNode*>(index.internalPointer());
        SceneNode* parentItem = childItem->parent();
        parentItem->removeChild(childItem->shared_from_this());
        //endResetModel();
    }
}

int HierarchyTreeModel::rowCount(const QModelIndex &parent) const
{
    SceneNode* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_engine.scene().root().get();
    else
        parentItem = static_cast<SceneNode*>(parent.internalPointer());

    return static_cast<int>(parentItem->childCount());
}

int HierarchyTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
    /*if (parent.isValid())
        return static_cast<int>(static_cast<SceneNode*>(parent.internalPointer())->childCount());
    else
        return static_cast<int>(m_scene.root()->childCount());*/
}

void HierarchyTreeModel::setupModelData(const QStringList& lines, engine::SceneNode* parent)
{

}

