#pragma once

#include "engine/Drawer.h"
#include "containers/memory.h"

namespace engine
{
    class DrawerAbs : public Drawer
    {
    public:
        virtual ~DrawerAbs() {};
        virtual void* native() { return nullptr; };
        virtual void setParent(void* /*parent*/) {};
    };
}
