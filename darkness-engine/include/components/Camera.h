#pragma once

#include "engine/EngineComponent.h"
#include "engine/primitives/Quaternion.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "tools/image/Image.h"
#include "tools/Property.h"
#include "components/Transform.h"
#include "components/PostprocessComponent.h"
#include "tools/hash/Hash.h"
#include "containers/memory.h"

namespace engine
{
    struct ViewCornerRays
    {
        Vector3f topLeft;
        Vector3f topRight;
        Vector3f bottomLeft;
        Vector3f bottomRight;
    };

    enum class Projection
    {
        Perspective,
        Orthographic
    };

    engine::string projectionToString(const Projection& projection);
    Projection stringToProjection(const engine::string& projection);

    constexpr int HaltonValueCount = 16;

    class Camera : public EngineComponent
    {
        Property m_width;
        Property m_height;
        Property m_nearPlane;
        Property m_farPlane;
        Property m_fieldOfView;
        Property m_projection;
        Property m_followSpeed;
        Property m_environmentMap;
        Property m_environmentMapStrength;
        Property m_exposure;
        Property m_pbrShadingModel;

        Vector3f m_smoothPosition;
        Vector3f m_forward;
        Vector3f m_up;
        Vector3f m_right;
        engine::vector<Vector2<double>> m_haltonValues;

        engine::shared_ptr<Transform> m_transform;
        engine::shared_ptr<image::ImageIf> m_environmentMapImage;
        TextureSRVOwner m_environmentMapOwnerSRV;
        TextureSRVOwner m_environmentIrradianceOwnerSRV;
        TextureSRVOwner m_environmentBrdfLUTOwnerSRV;
        TextureSRVOwner m_environmentSpecularOwnerSRV;

		TextureSRV m_environmentMapOverrideSRV;
		TextureSRV m_environmentIrradianceOverrideSRV;
		TextureSRV m_environmentBrdfLUTOverrideSRV;
		TextureSRV m_environmentSpecularOverrideSRV;

        bool m_cpuDirty;
        bool m_gpuDirty;

        Vector3f m_target;
    public:

        engine::shared_ptr<EngineComponent> clone() const override
        {
            auto res = engine::make_shared<engine::Camera>();
            res->name(m_name);
            res->width(width());
            res->height(height());
            res->nearPlane(nearPlane());
            res->farPlane(farPlane());
            res->fieldOfView(fieldOfView());
            res->projection(projection());
            res->followSpeed(followSpeed());
            res->environmentMapPath(environmentMapPath());
            res->environmentMapStrength(environmentMapStrength());
            res->exposure(exposure());
            res->pbrShadingModel(pbrShadingModel());
            res->smoothPosition(smoothPosition());
            res->target(target());
            return res;
        }

        Camera()
            : m_width{ this, "width", int(1024) }
            , m_height{ this, "height", int(1024) }
            , m_nearPlane{ this, "near", float(0.1f) }
            , m_farPlane{ this, "far", float(1000.0f) }
            , m_fieldOfView{ this, "fov", float(60.0f) }
            , m_projection{ this, "projection", Projection::Perspective }
            , m_followSpeed{ this, "followspeed", float(1.0f) }
            , m_environmentMap{ this, "environment map", engine::string(""), [this]() { this->m_cpuDirty = true; } }
            , m_environmentMapStrength{ this, "environment strength", float(1.0f) }
            , m_exposure{ this, "exposure", 1.0f }
            , m_pbrShadingModel{ this, "PBR Shading Model", engine::ButtonToggle::NotPressed }

            , m_smoothPosition{ 0.0f, 0.0f, 0.0f }
            , m_forward{ 0.0f, 0.0f, 1.0f }
            , m_up{ 0.0f, 1.0f, 0.0f }
            , m_right{ 1.0f, 0.0f, 0.0f }
            , m_haltonValues{}

            , m_transform{ nullptr }
            , m_environmentMapImage{ nullptr }
            , m_environmentMapOwnerSRV{}

            , m_environmentIrradianceOwnerSRV{}
            , m_environmentBrdfLUTOwnerSRV{}
            , m_environmentSpecularOwnerSRV{}

			, m_environmentMapOverrideSRV{}
			, m_environmentIrradianceOverrideSRV{}
			, m_environmentBrdfLUTOverrideSRV{}
			, m_environmentSpecularOverrideSRV{}
            
            , m_cpuDirty{ false }
            , m_gpuDirty{ false }
            , m_target{ }
            , m_jitteringEnabled{ true }
        {
            m_name = "Camera";
            createHaltonValues();

            m_transform = engine::make_shared<Transform>();
        }

        Camera(engine::shared_ptr<Transform> transform)
            : m_width{ this, "width", int(1024) }
            , m_height{ this, "height", int(1024) }
            , m_nearPlane{ this, "near", float(0.1f) }
            , m_farPlane{ this, "far", float(1000.0f) }
            , m_fieldOfView{ this, "fov", float(60.0f) }
            , m_projection{ this, "projection", Projection::Perspective }
            , m_followSpeed{ this, "followspeed", float(1.0f) }
            , m_environmentMap{ this, "environment map", engine::string(""), [this]() { this->m_cpuDirty = true; } }
            , m_environmentMapStrength{ this, "environment strength", float(1.0f) }
            , m_exposure{ this, "exposure", 1.0f }
            , m_pbrShadingModel{ this, "PBR Shading Model", engine::ButtonToggle::NotPressed }

            , m_smoothPosition{ 0.0f, 0.0f, 0.0f }
            , m_forward{ 0.0f, 0.0f, 1.0f }
            , m_up{ 0.0f, 1.0f, 0.0f }
            , m_right{ 1.0f, 0.0f, 0.0f }
            , m_haltonValues{}

            , m_transform{ transform }
            , m_environmentMapImage{ nullptr }
            , m_environmentMapOwnerSRV{}

            , m_environmentIrradianceOwnerSRV{}
            , m_environmentBrdfLUTOwnerSRV{}
            , m_environmentSpecularOwnerSRV{}

			, m_environmentMapOverrideSRV{}
			, m_environmentIrradianceOverrideSRV{}
			, m_environmentBrdfLUTOverrideSRV{}
			, m_environmentSpecularOverrideSRV{}

            , m_cpuDirty{ false }
            , m_gpuDirty{ false }
            , m_target{ }
            , m_jitteringEnabled{ true }
        {
            m_name = "Camera";
            createHaltonValues();
        }

        Transform& transform() const { return *m_transform.get(); }

        void invalidateGpu()
        {
            m_gpuDirty = true;
        }

        void cpuRefresh(Device& device)
        {
            if (m_cpuDirty)
            {
                m_cpuDirty = false;
                if (m_environmentMap.value<engine::string>() != "")
                    m_environmentMapImage = device.createImage(
                        tools::hash(m_environmentMap.value<engine::string>()),
                        m_environmentMap.value<engine::string>());
                else
                    m_environmentMapImage = nullptr;

                m_gpuDirty = true;
            }
        }

        void gpuRefresh(Device& device)
        {
            if (m_gpuDirty)
            {
                m_gpuDirty = false;
                auto path = m_environmentMap.value<engine::string>();
                if (path != "" && m_environmentMapImage)
                {
                    m_environmentMapOwnerSRV = device.createTextureSRV(
                        tools::hash(path),
                        TextureDescription()
                        .name("Environment Cubemap")
                        .width(m_environmentMapImage->width())
                        .height(m_environmentMapImage->height())
                        .format(m_environmentMapImage->format())
                        .arraySlices(m_environmentMapImage->arraySlices())
                        .dimension(m_environmentMapImage->arraySlices() == 6 ? ResourceDimension::TextureCube : ResourceDimension::Texture2D)
                        .mipLevels(m_environmentMapImage->mipCount())
                        .setInitialData(TextureDescription::InitialData(
                            tools::ByteRange(m_environmentMapImage->data(), m_environmentMapImage->data() + m_environmentMapImage->bytes()),
                            static_cast<uint32_t>(m_environmentMapImage->width()), static_cast<uint32_t>(m_environmentMapImage->width() * m_environmentMapImage->height()))));
                }
                else
                {
                    m_environmentMapOwnerSRV = TextureSRVOwner();
                }
            }
        }

        void start() override
        {
            m_transform = getComponent<Transform>();
        }

        int width() const;
        void width(int _width);

        int height() const;
        void height(int _height);

        float nearPlane() const;
        void nearPlane(float _near);

        float farPlane() const;
        void farPlane(float _far);

        float fieldOfView() const;
        void fieldOfView(float _fov);

        PostprocessSettings postProcessSettings()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->settings();
        }

        void postProcessSettings(PostprocessSettings settings)
        {
            auto post = getComponent<PostprocessComponent>();
            post->settings(settings);
        }

        /*bool bloomEnabled()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->bloomEnabled();
        }

        void bloomEnabled(bool value)
        {
            auto post = getComponent<PostprocessComponent>();
            post->bloomEnabled(value);
        }

        float bloomStrength()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->bloomStrength();
        }

        void bloomStrength(float value)
        {
            auto post = getComponent<PostprocessComponent>();
            post->bloomStrength(value);
        }

        float bloomThreshold()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->bloomThreshold();
        }

        void bloomThreshold(float value)
        {
            auto post = getComponent<PostprocessComponent>();
            post->bloomThreshold(value);
        }

        void targetLuminance(float value)
        {
            auto post = getComponent<PostprocessComponent>();
            post->targetLuminance(value);
        }

        float targetLuminance()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->targetLuminance();
        }

        void adaptationRate(float value)
        {
            auto post = getComponent<PostprocessComponent>();
            post->adaptationRate(value);
        }

        float adaptationRate()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->adaptationRate();
        }

        void minExposure(float value)
        {
            auto post = getComponent<PostprocessComponent>();
            post->minExposure(value);
        }

        float minExposure()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->minExposure();
        }

        void maxExposure(float value)
        {
            auto post = getComponent<PostprocessComponent>();
            post->maxExposure(value);
        }

        float maxExposure()
        {
            auto post = getComponent<PostprocessComponent>();
            return post->maxExposure();
        }*/

        Projection projection() const;
        void projection(Projection _projection);

        Quaternionf rotation() const;
        void rotation(Quaternionf _rotation);

        Vector3f position() const;
        void position(Vector3f _position);

        void forward(Vector3f forward);
        Vector3f forward() const;

        void up(Vector3f up);
        Vector3f up() const;

        void right(Vector3f right);
        Vector3f right() const;

        void updateDelta(float delta);
        Vector3f smoothPosition() const;
        void smoothPosition(const Vector3f& smooth)
        {
            m_smoothPosition = smooth;
        }

        Vector3f target() const;
        void target(Vector3f target);

        Matrix4f viewMatrix() const;
        Matrix4f viewMatrix(Vector3f withPosition) const;
        Matrix4f projectionMatrix() const;
        Matrix4f projectionMatrix(Vector2<int> screenSize) const;

        Matrix4f jitterMatrix(uint64_t frameNumber, Vector2<int> screenSize) const;
        Vector2f jitterValue(uint64_t frameNumber) const;

        static Matrix4f lookAt(
            const Vector3f& position,
            const Vector3f& target,
            const Vector3f& up = Vector3f{ 0.0f, 1.0f, 0.0f });

        TextureSRV environmentMap()
        {
			if (m_environmentMapOverrideSRV.valid())
				return m_environmentMapOverrideSRV;
            return m_environmentMapOwnerSRV;
        }

        TextureSRV environmentIrradiance()
        {
			if (m_environmentIrradianceOverrideSRV.valid())
				return m_environmentIrradianceOverrideSRV;
            return m_environmentIrradianceOwnerSRV;
        }

        TextureSRV environmentBrdfLUT()
        {
			if (m_environmentBrdfLUTOverrideSRV.valid())
				return m_environmentBrdfLUTOverrideSRV;
            return m_environmentBrdfLUTOwnerSRV;
        }

        TextureSRV environmentSpecular()
        {
			if (m_environmentSpecularOverrideSRV.valid())
				return m_environmentSpecularOverrideSRV;
            return m_environmentSpecularOwnerSRV;
        }

        bool jitteringEnabled() const { return m_jitteringEnabled; }
        void jitteringEnabled(bool enabled) { m_jitteringEnabled = enabled; }

		void environmentMap(const TextureSRV srv)
		{
			m_environmentMapOverrideSRV = srv;
		}

		void environmentIrradiance(TextureSRV srv)
		{
			m_environmentIrradianceOverrideSRV = srv;
		}

		void environmentBrdfLUT(TextureSRV srv)
		{
			m_environmentBrdfLUTOverrideSRV = srv;
		}

        void environmentSpecular(TextureSRV tex)
        {
			m_environmentSpecularOverrideSRV = tex;
        }

        const engine::string& environmentMapPath() const
        {
            return m_environmentMap.value<engine::string>();
        }
        void environmentMapPath(const engine::string& path)
        {
            m_environmentMap.value<engine::string>(path);
        }

        float environmentMapStrength() const
        {
            return m_environmentMapStrength.value<float>();
        }

        void environmentMapStrength(float val)
        {
            m_environmentMapStrength.value<float>(val);
        }

        float followSpeed() const
        {
            return m_followSpeed.value<float>();
        }

        void followSpeed(float val)
        {
            m_followSpeed.value<float>(val);
        }

        float exposure() const { return m_exposure.value<float>(); }
        void exposure(float val) { m_exposure.value<float>(val); }

        bool pbrShadingModel() const { return static_cast<bool>(m_pbrShadingModel.value<engine::ButtonToggle>()); }
        void pbrShadingModel(bool val) { m_pbrShadingModel.value<engine::ButtonToggle>(static_cast<engine::ButtonToggle>(val)); }

        ViewCornerRays viewRays() const;
		ViewCornerRays nearPlaneCorners() const;
    private:
        Matrix4f orthographicMatrix() const;
        Matrix4f perspectiveMatrix() const;

        Matrix4f orthographicMatrix(Vector2<int> screenSize) const;
        Matrix4f perspectiveMatrix(Vector2<int> screenSize) const;

        void createHaltonValues();
        bool m_jitteringEnabled;
    };
}
