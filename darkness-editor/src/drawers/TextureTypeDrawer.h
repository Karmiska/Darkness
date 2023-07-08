#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

#include "tools/Property.h"
#include "engine/Drawer.h"
#include "engine/rendering/Material.h"

namespace engine
{
	class TextureTypeWidget : public QWidget
	{
		Q_OBJECT
	public:
		TextureTypeWidget(Property& value, QWidget *parent = Q_NULLPTR,
			Qt::WindowFlags f = Qt::WindowFlags());

        virtual ~TextureTypeWidget();
	private:
		Property& m_value;
        bool m_valueValid;
        bool m_canReceiveUpdate;
		engine::unique_ptr<QHBoxLayout> m_layout;

		engine::unique_ptr<QLabel> m_nameLabel;
		engine::unique_ptr<QComboBox> m_values;
		engine::unique_ptr<QLabel> m_xLabel;

		void updateProperty();

		private slots:
		void onActivated(int index);
	};

	class TextureTypeDrawer : public Drawer
	{
	public:
		TextureTypeDrawer(Property& value);
		void setParent(void* parent) override;
		void* native() override;

	private:
		Property& m_value;
		engine::shared_ptr<QWidget> m_widget;
		QWidget* m_parent;
	};

	template <>
	engine::shared_ptr<Drawer> createDrawer<TextureType>(Property& value);

}
