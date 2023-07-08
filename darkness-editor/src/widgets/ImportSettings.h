#pragma once

#include <QDockWidget>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include "containers/memory.h"
#include "../settings/Settings.h"
#include "HierarchyTreeModel.h"
#include "engine/Engine.h"
#include "plugins/PluginManager.h"
#include "tools/Property.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Quaternion.h"

class ImportSettings : public QDialog
{
    Q_OBJECT
public:
    explicit ImportSettings(
        QList<QPair<QString, QString>>& images,
        QList<QPair<QString, QString>>& models,
        QWidget *parent = Q_NULLPTR,
        Qt::WindowFlags flags = Qt::WindowFlags());

    engine::Vector3f scale() const
    {
        return m_modelScaleProperty.value<engine::Vector3f>();
    }

    engine::Quaternionf rotation() const
    {
        return m_modelRotationProperty.value<engine::Quaternionf>();
    }

    engine::string preferredEncoding() const
    {
        return textureTypeToString(m_preferredEncodingProperty.value<engine::TextureType>());
    }

	bool flipNormal() const
	{
		return m_flipNormalPropery.value<bool>();
	}

	bool alphaClipped() const
	{
		return m_alphaClippedProperty.value<bool>();
	}
private:
    engine::unique_ptr<QVBoxLayout> m_layout;
    
    engine::Property m_modelScaleProperty;
    engine::Property m_modelRotationProperty;

    engine::Property m_preferredEncodingProperty;
	engine::Property m_flipNormalPropery;
	engine::Property m_alphaClippedProperty;

    engine::Property m_okButton;

    engine::vector<engine::shared_ptr<Drawer>> m_drawers;
};
