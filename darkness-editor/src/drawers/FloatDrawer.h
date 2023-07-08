#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>

#include "tools/Property.h"
#include "engine/Drawer.h"
#include "DragableLabel.h"
#include "engine/primitives/Vector3.h"

namespace engine
{
    class FloatWidget : public QWidget
    {
        Q_OBJECT
    public:
        FloatWidget(Property& value, QWidget *parent = Q_NULLPTR,
            Qt::WindowFlags f = Qt::WindowFlags());

        virtual ~FloatWidget();
    private:
        Property& m_value;
        bool m_valueValid;
        bool m_canReceiveUpdate;
        engine::unique_ptr<QHBoxLayout> m_layout;

        engine::unique_ptr<QLabel> m_nameLabel;
        engine::unique_ptr<QLineEdit> m_xLine;
        engine::unique_ptr<DragableLabel> m_xLabel;

        void updateProperty();

    private slots:
        void onEditFinished();
    };

    class FloatDrawer : public Drawer
    {
    public:
        FloatDrawer(Property& value);
        void setParent(void* parent) override;
        void* native() override;

    private:
        Property& m_value;
        engine::shared_ptr<QWidget> m_widget;
        QWidget* m_parent;
    };

    template <>
    engine::shared_ptr<Drawer> createDrawer<float>(Property& value);

}
