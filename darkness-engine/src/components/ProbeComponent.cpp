#include "components/ProbeComponent.h"
#include "components/Camera.h"
#include "components/PostprocessComponent.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/rendering/ViewportRenderer.h"
#include <algorithm>

namespace engine
{
    ProbeComponent::ProbeComponent()
        : size{ this, "Size", 1024 }
        , m_range{ this, "Range", 5.0f }
        , updateButton{ this, "Update", ButtonPush{ ButtonPush::NotPressed }, [this]()
        {
            if (this->updateButton.value<ButtonPush>() == ButtonPush::Pressed)
                this->m_updateNow = true;
        } }
        , m_updateNow{ false }
    {
        m_name = "Probe";
    };

    void ProbeComponent::initialize()
    {
        //m_transform = engine::make_shared<Transform>();
        m_transform = getComponent<Transform>();
        m_camera = engine::make_shared<Camera>(m_transform);
        m_postprocess = engine::make_shared<PostprocessComponent>();

        m_camera->nearPlane(0.1f);
        m_camera->farPlane(1000.0f);
        m_camera->fieldOfView(90.0f);
        m_camera->projection(Projection::Perspective);
        m_camera->pbrShadingModel(true);

        auto settings = m_postprocess->settings();
        settings.bloom.enabled = true;
        m_postprocess->settings(settings);
    }

    engine::shared_ptr<engine::EngineComponent> ProbeComponent::clone() const
    {
        auto res = engine::make_shared<ProbeComponent>();
        return res;
    };

    static const char* pass_msg[6] = { "Pass POSX", "Pass NEGX", "Pass POSY", "Pass NEGY", "Pass POSZ", "Pass NEGZ" };

    Vector3f ProbeComponent::position()
    {
        if (!m_transform)
            initialize();
        return m_transform->position();
    }

    float ProbeComponent::range() const
    {
        return m_range.value<float>();
    }

    CubePass::CubePass(Device& device, int width, int height)
    {
		m_cubemapUAV = device.createTextureUAV(TextureDescription()
            .format(Format::R16G16B16A16_FLOAT)
            .width(width)
            .height(height)
            .arraySlices(6)
            .dimension(ResourceDimension::TextureCube)
            .name("Probe cubemap")
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .optimizedClearValue(Float4(0.0f, 0.0f, 0.0f, 0.0f)));
		m_cubemap = device.createTextureSRV(m_cubemapUAV);

        // create render targets
        TextureDescription desc = { m_cubemap.resource().texture().description() };
        desc.dimension(ResourceDimension::Texture2D);
        m_rtvs[0] = device.createTextureRTV(m_cubemapUAV, desc, SubResource{ 0, 1, 0, 1 });
        m_rtvs[1] = device.createTextureRTV(m_cubemapUAV, desc, SubResource{ 0, 1, 1, 1 });
        m_rtvs[2] = device.createTextureRTV(m_cubemapUAV, desc, SubResource{ 0, 1, 2, 1 });
        m_rtvs[3] = device.createTextureRTV(m_cubemapUAV, desc, SubResource{ 0, 1, 3, 1 });
        m_rtvs[4] = device.createTextureRTV(m_cubemapUAV, desc, SubResource{ 0, 1, 4, 1 });
        m_rtvs[5] = device.createTextureRTV(m_cubemapUAV, desc, SubResource{ 0, 1, 5, 1 });
    }

    void CubePass::render(
        Device& /*device*/,
        CommandList& cmd, 
        ViewportRenderer& viewportRenderer,
        Camera& camera,
        FlatScene& scene)
    {
		cmd.clearTexture(m_cubemapUAV, Vector4f{ 0.0f, 0.0f, 0.0f, 0.0f });
        Matrix4f m_cameraRotations[6];
        m_cameraRotations[0] = Camera::lookAt(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
        m_cameraRotations[1] = Camera::lookAt(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
        m_cameraRotations[2] = Camera::lookAt(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f));
        m_cameraRotations[3] = Camera::lookAt(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f));
        m_cameraRotations[4] = Camera::lookAt(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 1.0f, 0.0f));
        m_cameraRotations[5] = Camera::lookAt(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f));

        // render cubemap
        for (int i = 0; i < 6; ++i)
        {
            CPU_MARKER(cmd.api(), pass_msg[i]);
            GPU_MARKER(cmd, pass_msg[i]);
            //cmd.clearRenderTargetView(m_rtvs[i], { 0.0f, 0.0f, 0.0f, 0.0f });
            camera.rotation(Quaternionf::fromMatrix(m_cameraRotations[i]));
            viewportRenderer.render(cmd, scene, m_rtvs[i], 0, 0);
        }
    }

    TextureSRV ProbeComponent::cubemap()
    {
        return m_secondPass->cubemap();
    }

    TextureSRV ProbeComponent::brdf()
    {
        return m_camera->environmentBrdfLUT();
    }

    TextureSRV ProbeComponent::irradiance()
    {
        return m_camera->environmentIrradiance();
    }

    TextureSRV ProbeComponent::specular()
    {
        return m_camera->environmentSpecular();
    }

    bool ProbeComponent::update(Device& device, CommandList& cmd, FlatScene& scene)
    {
        if (!m_updateNow)
            return false;
        m_updateNow = false;

        CPU_MARKER(cmd.api(), "Probe render");
        GPU_MARKER(cmd, "Probe render");

        auto width = size.value<int>();
        auto height = size.value<int>();

        if (!m_transform)
            initialize();

        m_camera->width(width);
        m_camera->height(height);

        if (!m_transform)
        {
            m_transform = getComponent<Transform>();
        }

        if (!m_viewportRenderer)
            m_viewportRenderer = engine::make_shared<ViewportRenderer>(device, width, height);

        // create passes
        if (m_lastWidth != width || m_lastHeight != height)
        {
            m_firstPass = engine::make_unique<CubePass>(device, width, height);
            m_secondPass = engine::make_unique<CubePass>(device, width, height);
            m_lastWidth = width;
            m_lastHeight = height;
        }

        auto prevSelectedCamera = scene.selectedCamera;
        auto prevPostprocess = scene.postprocess;
        scene.postprocess = m_postprocess;
        scene.cameras.emplace_back(m_camera);
        scene.selectedCamera = static_cast<int>(scene.cameras.size() - 1);
        m_camera->position(m_transform->position());
        
        LOG("probe camera pos: [%f, %f, %f]",
            m_camera->position().x,
            m_camera->position().y,
            m_camera->position().z);

        Camera& cam = *scene.cameras[scene.selectedCamera];
        cam.environmentMapPath("");
        cam.environmentMap(TextureSRVOwner());
        m_firstPass->render(device, cmd, *m_viewportRenderer, *m_camera, scene);

        cam.environmentMap(m_firstPass->cubemap());
        m_viewportRenderer->cubemapRenderer().cubemap(cam.environmentMap());
        m_viewportRenderer->cubemapRenderer().createIrradianceCubemap(device, cam.environmentMap(), cmd);
        m_viewportRenderer->cubemapRenderer().prefilterConvolution(device, cam.environmentMap(), cmd);
        m_viewportRenderer->cubemapRenderer().brdfConvolution(device, cmd);

        /*m_secondPass->render(cmd, *m_viewportRenderer, *m_camera, scene);
        cam.environmentMap(m_secondPass->cubemap());
        m_viewportRenderer->cubemapRenderer().cubemap(cam.environmentMap());
        m_viewportRenderer->cubemapRenderer().createIrradianceCubemap(device, cam.environmentMap(), cmd);
        m_viewportRenderer->cubemapRenderer().prefilterConvolution(device, cam.environmentMap(), cmd);
        m_viewportRenderer->cubemapRenderer().brdfConvolution(cmd);*/
        
        cam.environmentBrdfLUT(m_viewportRenderer->cubemapRenderer().brdfConvolution());
        cam.environmentIrradiance(m_viewportRenderer->cubemapRenderer().irradiance());
        cam.environmentSpecular(m_viewportRenderer->cubemapRenderer().prefilteredEnvironmentMap());

        scene.selectedCamera = prevSelectedCamera;
        scene.cameras.erase(scene.cameras.end() - 1);
        scene.postprocess = prevPostprocess;
        return true;
    };
}
