#pragma once

#include "engine/primitives/Rect.h"
#include "containers/string.h"
#include <functional>
#include "containers/memory.h"

class Visual
{
public:
	virtual ~Visual() {};
    virtual void draw(engine::Rect) = 0;
};

struct DrawerCallbacks
{
    void* propertyWidget;
    std::function<void(void* propertyWidget, engine::Rect, const engine::string&)> drawString;
};

template<typename T>
engine::shared_ptr<Visual> createDrawer(const T& item, DrawerCallbacks callbacks);
