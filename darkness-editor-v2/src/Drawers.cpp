#include "Drawers.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"
#include "components/Camera.h"
#include "components/CollisionShapeComponent.h"
#include "components/LightComponent.h"

namespace engine
{
    template <>
    engine::shared_ptr<Drawer> createDrawer<int>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<float>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<double>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Vector2f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Vector3f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Vector4f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Matrix3f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Matrix4f>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<Projection>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<CollisionShape>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::string>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::Quaternionf>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<LightType>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }

    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::ButtonToggle>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }
    
    template <>
    engine::shared_ptr<Drawer> createDrawer<engine::ButtonPush>(Property& /*value*/) { return engine::make_shared<DrawerAbs>(); }
    
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
}
