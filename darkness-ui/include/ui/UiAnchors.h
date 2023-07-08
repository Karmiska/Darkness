#pragma once

#include "containers/vector.h"
#include "ui/UiRect.h"
#include "ui/UiArea.h"
#include "ui/UiLayer.h"
#include "ui/UiEvents.h"
#include "ui/UiRestrictions.h"

namespace ui
{
    class UiArea;
    enum class AnchorType
    {
        Top = 0x1,
        Left = 0x2,
        Right = 0x4,
        Bottom = 0x8,
        TopLeft = 0x3,
        TopRight = 0x5,
        BottomLeft = 0xA,
        BottomRight = 0xC
    };

    struct UiAnchorMargins
    {
        int horizontal;
        int vertical;

        UiAnchorMargins()
            : horizontal{ 0 }
            , vertical{ 0 }
        {}
        UiAnchorMargins(int _horizontal, int _vertical)
            : horizontal{ _horizontal }
            , vertical{ _vertical }
        {}
    };

    class UiAnchors;
    struct UiAnchor
    {
        UiAnchors* target;
        AnchorType targetAnchor;
        AnchorType anchor;
        UiAnchorMargins margins;
    };

    class UiAnchors : public UiArea,
                      public UiBaseLayer,
                      public UiEvents,
                      public UiRestrictions
    {
    public:
        UiPoint position() const;
        UiPoint size() const;

        int x() const;
        int y() const;

        int width() const;
        int height() const;

        int left() const;
        int top() const;
        int right() const;
        int bottom() const;

        void position(const UiPoint& point);
        void position(int x, int y);

        void size(const UiPoint& size);
        void x(int val);
        void y(int val);
        void width(int val);
        void height(int val);
        void left(int val);
        void top(int val);
        void right(int val);
        void bottom(int val);

        void addAnchor(UiAnchor anchor);
        void removeAnchor(UiAnchor anchor);

    private:
        engine::vector<UiAnchor> m_anchors;
        void propagateChanges(unsigned int changes);
    };
}
