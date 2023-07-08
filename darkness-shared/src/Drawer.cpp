#include "Drawer.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"

template<>
engine::shared_ptr<Visual> createDrawer<int>(const int& item, DrawerCallbacks callbacks)
{
    return engine::make_shared<IntDrawer>(item, callbacks);
}

template<>
engine::shared_ptr<Visual> createDrawer<float>(const float& item, DrawerCallbacks callbacks)
{
    return engine::make_shared<FloatDrawer>(item, callbacks);
}

template<>
engine::shared_ptr<Visual> createDrawer<engine::Vector3f>(const engine::Vector3f& item, DrawerCallbacks callbacks)
{
    return engine::make_shared<Vector3fDrawer>(item, callbacks);
}

template<>
engine::shared_ptr<Visual> createDrawer<engine::Matrix4f>(const engine::Matrix4f& item, DrawerCallbacks callbacks)
{
    return engine::make_shared<Matrix4fDrawer>(item, callbacks);
}

template<>
engine::shared_ptr<Visual> createDrawer<engine::string>(const engine::string& item, DrawerCallbacks callbacks)
{
    return engine::make_shared<StdStringDrawer>(item, callbacks);
}
