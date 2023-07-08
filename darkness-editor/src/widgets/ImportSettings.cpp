#include "ImportSettings.h"

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

using namespace engine;

ImportSettings::ImportSettings(
    QList<QPair<QString, QString>>& images,
    QList<QPair<QString, QString>>& models,
    QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_layout{ engine::make_unique<QVBoxLayout>() }
    
    , m_modelScaleProperty{ nullptr, "ModelScale", engine::Vector3f{ 1.0f, 1.0f, 1.0f } }
    , m_modelRotationProperty{ nullptr, "ModelRotation", engine::Quaternionf::fromEulerAngles(0.0f, 0.0f, 0.0f) }
    , m_preferredEncodingProperty{ nullptr, "Preferred encoding", engine::TextureType::Albedo }
	, m_flipNormalPropery{ nullptr, "Flip normal", bool(false) }
	, m_alphaClippedProperty{ nullptr, "Alphaclip", bool(false) }
    , m_okButton{ nullptr, "OK", engine::ButtonPush::NotPressed, [this]() { this->close(); } }
{
    setWindowTitle("Import settings");
    setObjectName("ImportSettings");

    this->setLayout(m_layout.get());

    // model
    {
        // scale
        m_drawers.emplace_back(m_modelScaleProperty.createVariantDrawer());
        m_drawers.back()->setParent(this);
        m_layout->addWidget(static_cast<QWidget*>(m_drawers.back()->native()));

        // rotation
        m_drawers.emplace_back(m_modelRotationProperty.createVariantDrawer());
        m_drawers.back()->setParent(this);
        m_layout->addWidget(static_cast<QWidget*>(m_drawers.back()->native()));
    }

    // texture
    {
        // encoding
        m_drawers.emplace_back(m_preferredEncodingProperty.createVariantDrawer());
        m_drawers.back()->setParent(this);
        m_layout->addWidget(static_cast<QWidget*>(m_drawers.back()->native()));

		m_drawers.emplace_back(m_flipNormalPropery.createVariantDrawer());
		m_drawers.back()->setParent(this);
		m_layout->addWidget(static_cast<QWidget*>(m_drawers.back()->native()));

		m_drawers.emplace_back(m_alphaClippedProperty.createVariantDrawer());
		m_drawers.back()->setParent(this);
		m_layout->addWidget(static_cast<QWidget*>(m_drawers.back()->native()));
    }

    // OK Button
    m_drawers.emplace_back(m_okButton.createVariantDrawer());
    m_drawers.back()->setParent(this);
    m_layout->addWidget(static_cast<QWidget*>(m_drawers.back()->native()));
}