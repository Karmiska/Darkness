#pragma once

#include "ui/UiBaseObject.h"

namespace ui
{
    class UiBaseLayer : public UiBaseObject
    {
    public:
        void moveToTop();
        void moveUp();
        void moveDown();
        void moveToBottom();
    };
}
