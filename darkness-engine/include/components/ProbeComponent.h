#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"

#include "engine/graphics/ResourceOwners.h"
#include "engine/primitives/Matrix4.h"

namespace engine
{
    class Transform;
    class Camera;
    class Device;
    class CommandList;
    struct FlatScene;
    class ViewportRenderer;
    class PostprocessComponent;

    class CubePass
    {
    public:
        CubePass(Device& device, int width, int height);
        void render(
            Device& device,
            CommandList& cmd,
            ViewportRenderer& viewportRenderer,
            Camera& camera,
            FlatScene& scene);
        TextureSRV cubemap()
        {
            return m_cubemap;
        }
    private:
        TextureSRVOwner m_cubemap;
		TextureUAVOwner m_cubemapUAV;
        TextureRTVOwner m_rtvs[6];
    };

    class ProbeComponent : public EngineComponent
    {
        Property size;
        Property m_range;
        Property updateButton;
    public:
        ProbeComponent();
        //ProbeComponent(engine::shared_ptr<engine::Transform> transform);
        engine::shared_ptr<engine::EngineComponent> clone() const override;

        bool update(Device& device, CommandList& cmd, FlatScene& scene);

        TextureSRV cubemap();
        TextureSRV brdf();
        TextureSRV irradiance();
        TextureSRV specular();

        Vector3f position();
        float range() const;
    private:
        void initialize();
        bool m_updateNow;
        engine::shared_ptr<Transform> m_transform;
        engine::shared_ptr<Camera> m_camera;

        int m_lastWidth;
        int m_lastHeight;
        engine::unique_ptr<CubePass> m_firstPass;
        engine::unique_ptr<CubePass> m_secondPass;

        engine::shared_ptr<ViewportRenderer> m_viewportRenderer;
        engine::shared_ptr<PostprocessComponent> m_postprocess;
    };
}
