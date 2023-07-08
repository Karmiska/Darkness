#include "Hierarchy.h"
#include "../Tools.h"
#include "containers/unordered_map.h"
#include "components/TerrainComponent.h"

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
#include <QApplication>

using namespace engine;

void HierarchyTreeView::keyPressEvent(QKeyEvent* keyEvent)
{
    if (hasFocus())
    {
        auto engineKey = qtKeyToEngineKey(static_cast<Qt::Key>(keyEvent->key()));

        if (engineKey == engine::Key::Unknown)
        {
            auto nativeMods = interpretKeyEvent(keyEvent);

            if ((nativeMods & VK_LSHIFT) == VK_LSHIFT)
                m_modState[engine::KeyModifier::ShiftLeft] = true;

            if ((nativeMods & VK_RSHIFT) == VK_RSHIFT)
                m_modState[engine::KeyModifier::ShiftRight] = true;

            if ((nativeMods & VK_LMENU) == VK_LMENU)
                m_modState[engine::KeyModifier::AltLeft] = true;

            if ((nativeMods & VK_RMENU) == VK_RMENU)
                m_modState[engine::KeyModifier::AltRight] = true;

            if ((nativeMods & VK_LCONTROL) == VK_LCONTROL)
                m_modState[engine::KeyModifier::CtrlLeft] = true;

            if ((nativeMods & VK_RCONTROL) == VK_RCONTROL)
                m_modState[engine::KeyModifier::CtrlRight] = true;
        }


        if (keyEvent->key() == Qt::Key::Key_Delete)
            emit deleteSelected();

        if ((m_modState[engine::KeyModifier::ShiftLeft] || m_modState[engine::KeyModifier::ShiftRight]) 
            && (engineKey == engine::Key::D))
        {
            emit duplicateSelected();
        }
    }
}

void HierarchyTreeView::keyReleaseEvent(QKeyEvent* keyEvent)
{
    if (!hasFocus())
        return;

    auto engineKey = qtKeyToEngineKey(static_cast<Qt::Key>(keyEvent->key()));

    if (engineKey == engine::Key::Unknown)
    {
        auto nativeMods = interpretKeyEvent(keyEvent);

        if ((nativeMods & VK_LSHIFT) == VK_LSHIFT)
            m_modState[engine::KeyModifier::ShiftLeft] = false;

        if ((nativeMods & VK_RSHIFT) == VK_RSHIFT)
            m_modState[engine::KeyModifier::ShiftRight] = false;

        if ((nativeMods & VK_LMENU) == VK_LMENU)
            m_modState[engine::KeyModifier::AltLeft] = false;

        if ((nativeMods & VK_RMENU) == VK_RMENU)
            m_modState[engine::KeyModifier::AltRight] = false;

        if ((nativeMods & VK_LCONTROL) == VK_LCONTROL)
            m_modState[engine::KeyModifier::CtrlLeft] = false;

        if ((nativeMods & VK_RCONTROL) == VK_RCONTROL)
            m_modState[engine::KeyModifier::CtrlRight] = false;
    }
}

Hierarchy::Hierarchy(
    const Settings& settings,
    Engine& engine,
    QMainWindow* mainWindow,
    QWidget* parent,
    Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
    , m_engine{ engine }
    , m_contentPath{ settings.contentPathAbsolute() }
    , m_mainWindow{ mainWindow }
    , m_sceneModel{ engine::make_unique<HierarchyTreeModel>(m_engine, settings.contentPathAbsolute(), settings.processedAssetsPathAbsolute()) }
    , m_sceneView{ engine::make_unique<HierarchyTreeView>(this) }
{
    //manager.addFolder("C:\\work\\darkness\\darkness-coreplugins\\bin\\x64\\Debug\\");

    setWindowTitle("Hierarchy");
    setObjectName("Hierarchy");
    setWidget(m_sceneView.get());
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);

    m_sceneView->setModel(m_sceneModel.get());
    m_sceneView->setRootIsDecorated(true);
    m_sceneView->setHeaderHidden(true);
    m_sceneView->hideColumn(1);
    m_sceneView->hideColumn(2);
    m_sceneView->hideColumn(3);
    m_sceneView->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
    m_sceneView->setDropIndicatorShown(true);
    m_sceneView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
    m_sceneView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_mainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, this);

    QObject::connect(
        m_sceneView.get(), SIGNAL(clicked(QModelIndex)),
        this, SLOT(treeDirClicked(QModelIndex)));

    m_sceneView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_sceneView.get(), SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

    connect(
        m_sceneView.get(), SIGNAL(deleteSelected()),
        this, SLOT(deleteSelected()));

    connect(
        m_sceneView.get(), SIGNAL(duplicateSelected()),
        this, SLOT(duplicateSelected()));

    QObject::connect(
        m_sceneView.get(), SIGNAL(expanded(const QModelIndex&)),
        this, SLOT(expanded(const QModelIndex&)));

}

void Hierarchy::dropEvent(QDropEvent* dropEvent)
{
    QModelIndex index = m_sceneView->currentIndex();
    if (dropEvent->mimeData()->hasUrls())
    {
        for (auto&& url : dropEvent->mimeData()->urls())
        {
            auto sceneIndex = m_sceneModel->index(index.row(), index.column());
            auto node = m_sceneModel->node(sceneIndex);

            auto newNode = engine::make_shared<engine::SceneNode>();
            newNode->name("droppedNode");
            node->addChild(newNode);
            
            m_sceneView->update(index);
            /*emit processDroppedItem(
                url.toLocalFile(),
                m_fileSystemDirModel->filePath(index));*/
        }
    }
}

void Hierarchy::ShowContextMenu(const QPoint& point)
{
    QModelIndex index = m_sceneView->indexAt(point);
    QPoint globalPos = m_sceneView->mapToGlobal(point);

    QMenu myMenu;
    auto selectedIndexes = m_sceneView->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() > 1)
    {
        // multiple selection
        myMenu.addAction("Copy");
        myMenu.addAction("Paste");
        myMenu.addSeparator();
        myMenu.addAction("Duplicate");
        myMenu.addAction("Delete");
        myMenu.addSeparator();
        myMenu.addAction("Group");
    }
    else if (index.isValid())
    {
        // single selection
        myMenu.addAction("Copy");
        myMenu.addAction("Paste");
        myMenu.addSeparator();
        myMenu.addAction("Rename");
        myMenu.addAction("Duplicate");
        myMenu.addAction("Delete");
        myMenu.addSeparator();
        myMenu.addAction("Create Empty");
        QMenu* objects3d = myMenu.addMenu("3D Object");
        objects3d->addAction("Cube");
        objects3d->addAction("Sphere");
        objects3d->addAction("Capsule");
        objects3d->addAction("Cylinder");
        objects3d->addAction("Plane");
        objects3d->addAction("Quad");
        objects3d->addAction("Terrain");
        QMenu* objects2d = myMenu.addMenu("2D Object");
        objects2d->addAction("Sprite");
        QMenu* lights = myMenu.addMenu("Light");
        lights->addAction("Directional Light");
        lights->addAction("Point Light");
        lights->addAction("Spot Light");
        myMenu.addAction("Camera");
        myMenu.addAction("Probe");
    }
    else
    {
        // no selection
        myMenu.addAction("Create Empty");
        QMenu* objects3d = myMenu.addMenu("3D Object");
        objects3d->addAction("Cube");
        objects3d->addAction("Sphere");
        objects3d->addAction("Capsule");
        objects3d->addAction("Cylinder");
        objects3d->addAction("Plane");
        objects3d->addAction("Quad");
        objects3d->addAction("Terrain");
        QMenu* objects2d = myMenu.addMenu("2D Object");
        objects2d->addAction("Sprite");
        QMenu* lights = myMenu.addMenu("Light");
        lights->addAction("Directional Light");
        lights->addAction("Point Light");
        lights->addAction("Spot Light");
        myMenu.addAction("Camera");
        myMenu.addAction("Probe");
    }
    QAction* selectedItem = myMenu.exec(globalPos);

    if (selectedItem)
    {
        auto newNode = engine::make_shared<engine::SceneNode>();
        bool nodeAdded = false;
        if (selectedItem->text().startsWith("Camera"))
        {
            newNode->name("Camera");
            newNode->addComponent(engine::make_shared<engine::Transform>());
            newNode->addComponent(engine::make_shared<engine::Camera>());
            newNode->addComponent(engine::make_shared<engine::PostprocessComponent>());

            //newNode->addComponent(manager.createType("Transform"));

            m_sceneModel->addNode(newNode);
            m_sceneView->update(index);

            nodeAdded = true;
        }
        else if (selectedItem->text().startsWith("Probe"))
        {
            newNode->name("Probe");
            newNode->addComponent(engine::make_shared<engine::Transform>());
            m_sceneModel->addNode(newNode);
            
            newNode->addComponent(engine::make_shared<engine::ProbeComponent>());

            m_sceneView->update(index);

            nodeAdded = true;
        }
		else if (selectedItem->text().startsWith("Terrain"))
		{
			newNode->name("Terrain");
			auto trans = engine::make_shared<engine::Transform>();
			newNode->addComponent(trans);
			m_sceneModel->addNode(newNode);

			newNode->addComponent(engine::make_shared<engine::TerrainComponent>(trans));

			m_sceneView->update(index);

			nodeAdded = true;
		}
        else if (selectedItem->text().startsWith("Directional Light"))
        {
            newNode->name("Directional Light");
            newNode->addComponent(engine::make_shared<engine::Transform>());
            auto light = engine::make_shared<engine::LightComponent>();
            light->lightType(engine::LightType::Directional);
            newNode->addComponent(light);

            m_sceneModel->addNode(newNode);
            m_sceneView->update(index);

            nodeAdded = true;
        }
        else if (selectedItem->text().startsWith("Spot Light"))
        {
            newNode->name("Spot Light");
            newNode->addComponent(engine::make_shared<engine::Transform>());
            auto light = engine::make_shared<engine::LightComponent>();
            light->lightType(engine::LightType::Spot);
            newNode->addComponent(light);

            m_sceneModel->addNode(newNode);
            m_sceneView->update(index);

            nodeAdded = true;
        }
        else if (selectedItem->text().startsWith("Point Light"))
        {
            newNode->name("Point Light");
            newNode->addComponent(engine::make_shared<engine::Transform>());
            auto light = engine::make_shared<engine::LightComponent>();
            light->lightType(engine::LightType::Point);
            newNode->addComponent(light);

            m_sceneModel->addNode(newNode);
            m_sceneView->update(index);

            nodeAdded = true;
        }

        if (selectedItem->text().startsWith("Delete"))
        {
            engine::vector<int64_t> toRemove;
            for (auto&& ind : selectedIndexes)
            {
                toRemove.emplace_back(reinterpret_cast<engine::SceneNode*>(ind.internalPointer())->id());
            }
            for (auto&& rm : toRemove)
            {
                auto node = m_engine.scene().find(rm);
                auto mindex = m_sceneModel->node(node.get());
                if (node->parent())
                {
                    m_sceneModel->startRemoveRows(m_sceneModel->node(node->parent()), mindex.row(), mindex.row());
                }
                m_sceneModel->removeNode(mindex);
                if (node->parent())
                {
                    m_sceneModel->stopRemoveRows();
                }
            }
        }

        if (selectedItem->text().startsWith("Duplicate"))
        {
            engine::vector<int64_t> toDuplicate;
            for (auto&& ind : selectedIndexes)
            {
                toDuplicate.emplace_back(reinterpret_cast<engine::SceneNode*>(ind.internalPointer())->id());
            }
            for (auto&& dup : toDuplicate)
            {
                auto node = m_engine.scene().find(dup);
                auto mindex = m_sceneModel->node(node.get());
                if (node->parent())
                {
                    m_sceneModel->startInsertRows(m_sceneModel->node(node->parent()), mindex.row(), mindex.row());
                }

                node->parent()->addChild(duplicate(node));

                if (node->parent())
                {
                    m_sceneModel->stopInsertRows();
                }
            }
        }

        if (selectedItem->text().startsWith("Group"))
        {
            if (selectedIndexes.size() == 0)
                return;
            
            // gather parents
            engine::vector<int64_t> parents;
            for (auto&& ind : selectedIndexes)
            {
                auto node = m_engine.scene().find(reinterpret_cast<engine::SceneNode*>(ind.internalPointer())->id());
                if (node->parent())
                {
                    parents.emplace_back(node->parent()->id());
                }
            }
            
            // find the parent with the highest hits
            engine::unordered_map<int64_t, int> hits;
            for(auto&& parent : parents)
            {
                hits[parent]++;
            }
            int64_t currentId = (*hits.begin()).first;
            int currentCount = (*hits.begin()).second;
        
            for (auto&& hit : hits)
            {
                if (hit.second > currentCount)
                {
                    currentId = hit.first;
                    currentCount = hit.second;
                }
            }
            
            // get that parent and create a new node for it
            auto masterParentNode = m_engine.scene().find(currentId);
            
            auto newGroupNode = engine::make_shared<engine::SceneNode>();
            newGroupNode->name("Group");
            newGroupNode->addComponent(engine::make_shared<engine::Transform>());
            masterParentNode->addChild(newGroupNode);
            
            // grab all the selected nodes
            engine::vector<engine::shared_ptr<engine::SceneNode>> nodes;
            for (auto&& ind : selectedIndexes)
            {
                auto node = m_engine.scene().find(reinterpret_cast<engine::SceneNode*>(ind.internalPointer())->id());
                nodes.emplace_back(node);
            }
            
            // remove all selected from model
            engine::vector<int64_t> toRemove;
            for (auto&& ind : selectedIndexes)
            {
                toRemove.emplace_back(reinterpret_cast<engine::SceneNode*>(ind.internalPointer())->id());
            }
            for (auto&& rm : toRemove)
            {
                auto node = m_engine.scene().find(rm);
                auto mindex = m_sceneModel->node(node.get());
                if (node->parent())
                {
                    m_sceneModel->startRemoveRows(m_sceneModel->node(node->parent()), mindex.row(), mindex.row());
                }
                m_sceneModel->removeNode(mindex);
                if (node->parent())
                {
                    m_sceneModel->stopRemoveRows();
                }
            }
            
            // add the selected nodes back to the model under new parent
            for (auto&& node : nodes)
            {
                newGroupNode->addChild(node);
            }
        }

        if (nodeAdded && index.isValid())
        {
            auto sceneIndex = m_sceneModel->index(index.row(), index.column());
            auto node = m_sceneModel->node(sceneIndex);
            node->addChild(newNode);
        }

        qDebug() << "selected: " << selectedItem->text();
    }
    else
    {
        qDebug() << "no selection";
    }
}

void Hierarchy::deleteSelected()
{
    auto selectedIndexes = m_sceneView->selectionModel()->selectedIndexes();
    engine::vector<int64_t> toRemove;
    for (auto&& ind : selectedIndexes)
    {
        toRemove.emplace_back(reinterpret_cast<engine::SceneNode*>(ind.internalPointer())->id());
    }
    for (auto&& rm : toRemove)
    {
        auto node = m_engine.scene().find(rm);
        auto mindex = m_sceneModel->node(node.get());
        if (node->parent())
        {
            m_sceneModel->startRemoveRows(m_sceneModel->node(node->parent()), mindex.row(), mindex.row());
        }
        m_sceneModel->removeNode(mindex);
        if (node->parent())
        {
            m_sceneModel->stopRemoveRows();
        }
    }
}

void Hierarchy::duplicateSelected()
{
    auto selectedIndexes = m_sceneView->selectionModel()->selectedIndexes();
    engine::vector<int64_t> toDuplicate;
    for (auto&& ind : selectedIndexes)
    {
        toDuplicate.emplace_back(reinterpret_cast<engine::SceneNode*>(ind.internalPointer())->id());
    }
    for (auto&& dup : toDuplicate)
    {
        auto node = m_engine.scene().find(dup);
        auto mindex = m_sceneModel->node(node.get());
        if (node->parent())
        {
            m_sceneModel->startInsertRows(m_sceneModel->node(node->parent()), mindex.row(), mindex.row());
        }

        node->parent()->addChild(duplicate(node));

        if (node->parent())
        {
            m_sceneModel->stopInsertRows();
        }
    }
}

engine::shared_ptr<engine::SceneNode> Hierarchy::duplicate(engine::shared_ptr<engine::SceneNode> src)
{
    auto node = engine::make_shared<engine::SceneNode>();
    node->name(src->name());
    for (int i = 0; i < src->componentCount(); ++i)
    {
        node->addComponent(src->component(i)->clone());
    }
    

    for (int i = 0; i < src->childCount(); ++i)
    {
        auto newNode = engine::make_shared<engine::SceneNode>();
        node->addChild(duplicate(src->child(i)));
    }
    return node;
}

void Hierarchy::expanded(const QModelIndex &index)
{
    if (index == m_expandIndex)
    {
        /*QModelIndex newFolder = m_fileSystemDirModel->mkdir(index, "New Folder");
        m_sceneView->edit(newFolder);*/
    }
}

void Hierarchy::onNodeSelected(engine::shared_ptr<engine::SceneNode> node)
{
    auto nodePath = m_engine.scene().path(node->id());
    if (nodePath.size() > 0)
    {
        for (auto&& n : nodePath)
        {
            QModelIndex modelIndex = m_sceneModel->node(n.get());
            if (modelIndex.isValid())
            {
                m_sceneView->expand(modelIndex);
                QApplication::processEvents();
            }
        }

        QModelIndex modelIndex = m_sceneModel->node(node.get());
        auto selectionModel = m_sceneView->selectionModel();
        selectionModel->select(modelIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        //m_sceneView->expand(modelIndex);
        emit nodeSelected(node);
    }
}

void Hierarchy::treeDirClicked(QModelIndex index)
{
    auto node = m_sceneModel->node(index);
    emit nodeSelected(node);
    m_engine.setSelected(node);
}

