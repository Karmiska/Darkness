#include "ui/UiAnchors.h"
#include "ui/Frame.h"

namespace ui
{
    void UiAnchors::addAnchor(UiAnchor anchor)
    {
        m_anchors.emplace_back(anchor);
        propagateChanges(0xffffffff);
    }

    void UiAnchors::removeAnchor(UiAnchor _anchor)
    {
        for (auto anchor = m_anchors.begin(); anchor != m_anchors.end(); ++anchor)
        {
            if (((*anchor).target == _anchor.target) &&
                ((*anchor).targetAnchor == _anchor.targetAnchor) &&
                ((*anchor).anchor == _anchor.anchor))
            {
                m_anchors.erase(anchor);
                return;
            }
        }
        ASSERT(false, "Tried to remove non-existing anchor");
    }

    UiPoint UiAnchors::position() const { return area().position(); }
    UiPoint UiAnchors::size() const { return area().size(); }

    int UiAnchors::x() const { return area().x(); }
    int UiAnchors::y() const { return area().y(); }

    int UiAnchors::width() const { return area().width(); }
    int UiAnchors::height() const { return area().height(); }

    int UiAnchors::left() const { return area().left(); }
    int UiAnchors::top() const { return area().top(); }
    int UiAnchors::right() const { return area().right(); }
    int UiAnchors::bottom() const { return area().bottom(); }

    void UiAnchors::propagateChanges(unsigned int changes)
    {
        for (auto&& anchor : m_anchors)
        {
            unsigned int myAnchor = static_cast<unsigned int>(anchor.anchor);
            unsigned int targetAnchor = static_cast<unsigned int>(anchor.targetAnchor);

            // for example: My top is in changes and we have an anchor that binds something to my top
            if (myAnchor & changes)
            {
                // my one side is anchored to some clients some side
                if (myAnchor & static_cast<unsigned int>(AnchorType::Bottom))
                {
                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Top))
                        anchor.target->top(height() + anchor.margins.vertical);
                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Bottom))
                        anchor.target->bottom(height() + anchor.margins.vertical);
                }
                if (myAnchor & static_cast<unsigned int>(AnchorType::Top))
                {
                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Top))
                        anchor.target->top(anchor.margins.vertical);
                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Bottom))
                        anchor.target->bottom(anchor.margins.vertical);
                }
                if (myAnchor & static_cast<unsigned int>(AnchorType::Right))
                {
                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Right))
                        anchor.target->right(width() + anchor.margins.horizontal);
                    
                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Left))
                        anchor.target->left(width() + anchor.margins.horizontal);
                }
                if (myAnchor & static_cast<unsigned int>(AnchorType::Left))
                {
                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Right))
                        anchor.target->right(anchor.margins.horizontal);

                    if (targetAnchor & static_cast<unsigned int>(AnchorType::Left))
                        anchor.target->left(anchor.margins.horizontal);
                }
            }
        }
    }

    void UiAnchors::position(const UiPoint& point)
    {
        bool positionChange = 
            area().position().x != point.x ||
            area().position().y != point.y;
        area().position(point);
        if(positionChange)
            onMove(point.x, point.y);
    }

    void UiAnchors::position(int x, int y)
    {
        bool positionChange =
            area().position().x != x ||
            area().position().y != y;
        area().position(x, y);
        if(positionChange)
            onMove(x, y);
    }

    void UiAnchors::size(const UiPoint& size)
    {
        auto localSize = size;
        if (localSize.x < m_minimumSize.x)
            localSize.x = m_minimumSize.x;
        if (localSize.y < m_minimumSize.y)
            localSize.y = m_minimumSize.y;

        area().size(localSize);
        
        propagateChanges(
            static_cast<unsigned int>(AnchorType::Right) |
            static_cast<unsigned int>(AnchorType::Bottom) |
            static_cast<unsigned int>(AnchorType::Left) |
            static_cast<unsigned int>(AnchorType::Top));

        if (dynamic_cast<Frame*>(this)->window())
            dynamic_cast<Frame*>(this)->window()->resize(width(), height());
        onResize(width(), height());
    }

    void UiAnchors::x(int val)
    {
        bool positionChange = area().position().x != val;
        area().x(val);

        if (positionChange)
            onMove(val, area().position().y);
    }

    void UiAnchors::y(int val)
    {
        bool positionChange = area().position().y != val;
        area().y(val);

        if (positionChange)
            onMove(area().position().x, val);
    }

    void UiAnchors::width(int val)
    {
        area().width(val);
        propagateChanges(
            static_cast<unsigned int>(AnchorType::Right) |
            static_cast<unsigned int>(AnchorType::Left));

        if (dynamic_cast<Frame*>(this)->window())
            dynamic_cast<Frame*>(this)->window()->resize(width(), height());
        onResize(width(), height());
    }

    void UiAnchors::height(int val)
    {
        area().height(val);
        propagateChanges(
            static_cast<unsigned int>(AnchorType::Bottom) |
            static_cast<unsigned int>(AnchorType::Top));

        if (dynamic_cast<Frame*>(this)->window())
            dynamic_cast<Frame*>(this)->window()->resize(width(), height());
        onResize(width(), height());
    }

    void UiAnchors::left(int val)
    {
        area().left(val);
        //propagateChanges(static_cast<unsigned int>(AnchorType::Left));
    }

    void UiAnchors::top(int val)
    {
        area().top(val);
        //propagateChanges(static_cast<unsigned int>(AnchorType::Top));
    }

    void UiAnchors::right(int val)
    {
        area().right(val);
        propagateChanges(
            static_cast<unsigned int>(AnchorType::Right) |
            static_cast<unsigned int>(AnchorType::Left));

        if (dynamic_cast<Frame*>(this)->window())
            dynamic_cast<Frame*>(this)->window()->resize(width(), height());
        onResize(width(), height());
    }

    void UiAnchors::bottom(int val)
    {
        area().bottom(val);
        propagateChanges(
            static_cast<unsigned int>(AnchorType::Bottom) |
            static_cast<unsigned int>(AnchorType::Top));

        if (dynamic_cast<Frame*>(this)->window())
            dynamic_cast<Frame*>(this)->window()->resize(width(), height());
        onResize(width(), height());
    }
}
