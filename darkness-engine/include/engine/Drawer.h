#pragma once

#include "containers/memory.h"
#include "containers/string.h"

class Drawer
{
public:
    virtual ~Drawer() {};
    virtual void* native() = 0;
    virtual void setParent(void* parent) = 0;
    virtual void setPaths(const engine::string& /*contentPath*/, const engine::string& /*processedContentPath*/) {};
};

namespace engine
{
    class Property;
    template <typename T>
    engine::shared_ptr<Drawer> createDrawer(Property& value);
}
