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
    class Vector3fWidget : public QWidget
    {
        Q_OBJECT
    public:
        Vector3fWidget(Property& value, QWidget *parent = Q_NULLPTR,
            Qt::WindowFlags f = Qt::WindowFlags());

        virtual ~Vector3fWidget();
    private:
        Property& m_value;
        bool m_valueValid;
        bool m_canReceiveUpdate;
        engine::unique_ptr<QHBoxLayout> m_layout;

        engine::unique_ptr<QLabel> m_nameLabel;
        engine::unique_ptr<QLineEdit> m_xLine;
        engine::unique_ptr<DragableLabel> m_xLabel;
        engine::unique_ptr<QLineEdit> m_yLine;
        engine::unique_ptr<DragableLabel> m_yLabel;
        engine::unique_ptr<QLineEdit> m_zLine;
        engine::unique_ptr<DragableLabel> m_zLabel;

        void updateProperty();

    private slots:
        void onXEditFinished();
        void onYEditFinished();
        void onZEditFinished();
    };

    class Vector3fDrawer : public Drawer
    {
    public:
        Vector3fDrawer(Property& value);
        void setParent(void* parent) override;
        void* native() override;

    private:
        Property& m_value;
        engine::shared_ptr<QWidget> m_widget;
        QWidget* m_parent;
    };

    template <>
    engine::shared_ptr<Drawer> createDrawer<Vector3f>(Property& value);

}
