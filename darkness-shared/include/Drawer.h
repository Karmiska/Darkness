#pragma once

#include "containers/vector.h"
#include "containers/memory.h"
#include <string>
#include <functional>
#include "DrawerCallbacks.h"
#include "Property.h"
#include "engine/primitives/Rect.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"

class TextBox : public Visual
{
    const Property& m_property;
    DrawerCallbacks& m_call;
public:
    TextBox(DrawerCallbacks& call, const Property& prop)
        : m_property{ prop }
        , m_call{ call }
    {}
    void draw(engine::Rect rect) override
    {
        m_call.drawString(m_call.propertyWidget, rect, m_property.value<engine::string>());
    }
};

enum class LayoutType
{
    Horizontal,
    Vertical
};

class VisualContainer
{
public:
	virtual ~VisualContainer() {};
    void addItem(engine::shared_ptr<Visual> visual)
    {
        m_childs.emplace_back(visual);
    }
    engine::vector<engine::shared_ptr<Visual>>& items()
    {
        return m_childs;
    }
protected:
    engine::vector<engine::shared_ptr<Visual>> m_childs;
};

class Layout : public Visual,
               public VisualContainer
{
    LayoutType m_type;
public:
    Layout(LayoutType type)
        : m_type{ type }
    {}

    void setLayoutType(LayoutType type)
    {
        m_type = type;
    }

    void draw(engine::Rect rect) override
    {
        int childCount = static_cast<int>(m_childs.size());
        int width = m_type == LayoutType::Horizontal ? rect.width / childCount : rect.width;
        int height = m_type == LayoutType::Vertical ? rect.height / childCount : rect.height;

        int correctLastWidth = 0;
        int correctLastHeight = 0;
        if (m_type == LayoutType::Horizontal && width * childCount != rect.width)
            correctLastWidth = rect.width - (width * childCount);
        if (m_type == LayoutType::Vertical && height * childCount != rect.height)
            correctLastHeight = rect.height - (height * childCount);

        int currentX = rect.x;
        int currentY = rect.y;
        for (int i = 0; i < childCount; ++i)
        {
            engine::Rect childRect{ currentX, currentY, width, height };
            if (i == childCount - 1)
            {
                if (m_type == LayoutType::Horizontal)
                    childRect.width += correctLastWidth;
                if (m_type == LayoutType::Vertical)
                    childRect.height += correctLastHeight;
            }
            m_childs[i]->draw(childRect);

            if(m_type == LayoutType::Horizontal)
                currentX += width;
            if(m_type == LayoutType::Vertical)
                currentY += height;
        }
    }
};

class IntDrawer : public Visual
{
    DrawerCallbacks m_callbacks;
    const int& m_value;
public:
    IntDrawer(const int& value, DrawerCallbacks callbacks)
        : m_callbacks{ callbacks }
        , m_value{ value }
    {}
    void draw(engine::Rect rect) override
    {
        m_callbacks.drawString(m_callbacks.propertyWidget, rect, std::to_string(m_value).c_str());
    }
};

class FloatDrawer : public Visual
{
    DrawerCallbacks m_callbacks;
    const float& m_value;
public:
    FloatDrawer(const float& value, DrawerCallbacks callbacks)
        : m_callbacks{ callbacks }
        , m_value{ value }
    {}
    void draw(engine::Rect rect) override
    {
        m_callbacks.drawString(m_callbacks.propertyWidget, rect, std::to_string(m_value).c_str());
    }
};

class StdStringDrawer : public Visual
{
    DrawerCallbacks m_callbacks;
    const engine::string& m_value;
public:
    StdStringDrawer(const engine::string& value, DrawerCallbacks callbacks)
        : m_callbacks{ callbacks }
        , m_value{ value }
    {}
    void draw(engine::Rect rect) override
    {
        m_callbacks.drawString(m_callbacks.propertyWidget, rect, m_value);
    }
};


class DragableValueDrawer : public Visual
{
    DrawerCallbacks m_callbacks;
    const engine::string& m_value;
public:
    DragableValueDrawer(const engine::string& value, DrawerCallbacks callbacks)
        : m_callbacks{ callbacks }
        , m_value{ value }
    {}
    void draw(engine::Rect rect) override
    {
        m_callbacks.drawString(m_callbacks.propertyWidget, rect, m_value);
    }
};

class TextEditDrawer : public Visual
{
    DrawerCallbacks m_callbacks;
    const engine::string& m_value;
public:
    TextEditDrawer(const engine::string& value, DrawerCallbacks callbacks)
        : m_callbacks{ callbacks }
        , m_value{ value }
    {}
    void draw(engine::Rect rect) override
    {
        m_callbacks.drawString(m_callbacks.propertyWidget, rect, m_value);
    }
};

class Vector3fDrawer : public Layout
{
private:
    DrawerCallbacks m_callbacks;

public:
    Vector3fDrawer(const engine::Vector3f& /*value*/, DrawerCallbacks callbacks)
        : Layout{ LayoutType::Horizontal }
        , m_callbacks{ callbacks }
    {
        auto labelLayout = engine::make_shared<Layout>(LayoutType::Horizontal);
        auto valueLayout = engine::make_shared<Layout>(LayoutType::Horizontal);
        addItem(labelLayout);
        addItem(valueLayout);

        labelLayout->addItem(engine::make_shared<StdStringDrawer>("vector name", callbacks));
        
        valueLayout->addItem(engine::make_shared<DragableValueDrawer>(" ", callbacks));
        valueLayout->addItem(engine::make_shared<TextEditDrawer>("", callbacks));

        valueLayout->addItem(engine::make_shared<DragableValueDrawer>(" ", callbacks));
        valueLayout->addItem(engine::make_shared<TextEditDrawer>("", callbacks));

        valueLayout->addItem(engine::make_shared<DragableValueDrawer>(" ", callbacks));
        valueLayout->addItem(engine::make_shared<TextEditDrawer>("", callbacks));
    }
};

class Matrix4fDrawer : public Visual
{
    DrawerCallbacks m_callbacks;
public:
    Matrix4fDrawer(const engine::Matrix4f& /*value*/, DrawerCallbacks callbacks)
        : m_callbacks{ callbacks }
    {}
    void draw(engine::Rect /*rect*/) override
    {
    }
};


