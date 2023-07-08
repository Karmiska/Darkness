#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

#include "tools/Property.h"
#include "engine/Drawer.h"
#include "DragableLabel.h"
#include "engine/primitives/Vector3.h"
#include "components/Camera.h"

namespace engine
{
    class ButtonPushWidget : public QWidget
    {
        Q_OBJECT
    public:
        ButtonPushWidget(Property& value, QWidget *parent = Q_NULLPTR,
            Qt::WindowFlags f = Qt::WindowFlags());

        virtual ~ButtonPushWidget();
    private:
        Property& m_value;
        bool m_valueValid;
        bool m_canReceiveUpdate;
        engine::unique_ptr<QHBoxLayout> m_layout;

        engine::unique_ptr<QLabel> m_nameLabel;
        engine::unique_ptr<QPushButton> m_button;

        void updateProperty();

    private slots:
        void onPressed();
    };

    class ButtonToggleWidget : public QWidget
    {
        Q_OBJECT
    public:
        ButtonToggleWidget(Property& value, QWidget *parent = Q_NULLPTR,
            Qt::WindowFlags f = Qt::WindowFlags());

        virtual ~ButtonToggleWidget();
    private:
        Property& m_value;
        bool m_canReceiveUpdate;
        engine::unique_ptr<QHBoxLayout> m_layout;

        engine::unique_ptr<QLabel> m_nameLabel;
        engine::unique_ptr<QCheckBox> m_button;

        void updateProperty(bool checked);

    private slots:
        void onToggled(bool checked);
    };

    class ButtonPushDrawer : public Drawer
    {
    public:
        ButtonPushDrawer(Property& value);
        void setParent(void* parent) override;
        void* native() override;

    private:
        Property& m_value;
        engine::shared_ptr<QWidget> m_widget;
        QWidget* m_parent;
    };

    class ButtonToggleDrawer : public Drawer
    {
    public:
        ButtonToggleDrawer(Property& value);
        void setParent(void* parent) override;
        void* native() override;

    private:
        Property& m_value;
        engine::shared_ptr<QWidget> m_widget;
        QWidget* m_parent;
    };

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::ButtonPush>(Property& value);

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::ButtonToggle>(Property& value);

}
