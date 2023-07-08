#include "PlaceholderDrawers.h"

#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector4.h"
#include "engine/primitives/Matrix3.h"

namespace engine
{
    template <>
    engine::shared_ptr<Drawer> createDrawer<bool>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<char>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<short>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<unsigned char>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<unsigned short>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<unsigned int>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<double>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::Vector2f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::Vector4f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::Matrix3f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }
}
