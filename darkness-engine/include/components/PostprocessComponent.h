#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"

namespace engine
{
    struct BloomSettings
    {
        bool enabled;
        float strength;
        float threshold;
    };

    struct AdaptiveExposureSettings
    {
        bool enabled;
        float targetLuminance;
        float adaptationRate;
        float minExposure;
        float maxExposure;
    };

    struct VignetteSettings
    {
        bool enabled;
        float innerRadius;
        float outerRadius;
        float opacity;
    };

    struct ChromaticAberrationSettings
    {
        bool enabled;
        float strength;
    };

    struct PostprocessSettings
    {
        BloomSettings bloom;
        AdaptiveExposureSettings adaptiveExposure;
        VignetteSettings vignette;
        ChromaticAberrationSettings chromaticAberration;
    };

    class PostprocessComponent : public EngineComponent
    {
        Property m_bloomEnabled;
        Property m_bloomStrength;
        Property m_bloomThreshold;
        
        Property m_adaptiveExposureEnabled;
        Property m_targetLuminance;
        Property m_adaptationRate;
        Property m_minExposure;
        Property m_maxExposure;

        Property m_vignetteEnabled;
        Property m_vignetteInnerRadius;
        Property m_vignetteOuterRadius;
        Property m_vignetteOpacity;

        Property m_chromaticAberrationEnabled;
        Property m_chromaticAberrationStrength;
    public:
        PostprocessComponent()
            : m_bloomEnabled{ this, "Bloom enabled", engine::ButtonToggle::NotPressed }
            , m_bloomStrength{ this, "Bloom strength", 1.0f }
            , m_bloomThreshold{ this, "Bloom threshold", 1.0f }

            , m_adaptiveExposureEnabled{ this, "Adaptive exposure enabled", engine::ButtonToggle::NotPressed }
            , m_targetLuminance{ this, "TargetLuminance", 0.58f }
            , m_adaptationRate{ this, "AdaptationRate", 0.05f }
            , m_minExposure{ this, "MinExposure", 1.0f / 64.0f }
            , m_maxExposure{ this, "MaxExposure", 64.0f }

            , m_vignetteEnabled{ this, "Vignette enabled", engine::ButtonToggle::NotPressed }
            , m_vignetteInnerRadius{ this, "Vignette inner radius", 0.2f }
            , m_vignetteOuterRadius{ this, "Vignette outer radius", 0.8f }
            , m_vignetteOpacity{ this, "Vignette opacity", 1.0f }

            , m_chromaticAberrationEnabled{ this, "Chromatic aberration enabled", engine::ButtonToggle::NotPressed }
            , m_chromaticAberrationStrength{ this, "Chromatic aberration strength", 4.0f }
        {
            m_name = "PostprocessComponent";
        }

        PostprocessSettings settings() const
        {
            PostprocessSettings settings;
            settings.bloom.enabled = static_cast<bool>(m_bloomEnabled.value<ButtonToggle>());
            settings.bloom.strength = m_bloomStrength.value<float>();
            settings.bloom.threshold = m_bloomThreshold.value<float>();

            settings.adaptiveExposure.enabled = static_cast<bool>(m_adaptiveExposureEnabled.value<ButtonToggle>());
            settings.adaptiveExposure.targetLuminance = m_targetLuminance.value<float>();
            settings.adaptiveExposure.adaptationRate = m_adaptationRate.value<float>();
            settings.adaptiveExposure.minExposure = m_minExposure.value<float>();
            settings.adaptiveExposure.maxExposure = m_maxExposure.value<float>();

            settings.vignette.enabled = static_cast<bool>(m_vignetteEnabled.value<ButtonToggle>());
            settings.vignette.innerRadius = m_vignetteInnerRadius.value<float>();
            settings.vignette.outerRadius = m_vignetteOuterRadius.value<float>();
            settings.vignette.opacity = m_vignetteOpacity.value<float>();

            settings.chromaticAberration.enabled = static_cast<bool>(m_chromaticAberrationEnabled.value<ButtonToggle>());
            settings.chromaticAberration.strength = m_chromaticAberrationStrength.value<float>();

            return settings;
        }

        void settings(PostprocessSettings settings)
        {
            m_bloomEnabled.value<ButtonToggle>(static_cast<ButtonToggle>(settings.bloom.enabled));
            m_bloomStrength.value(settings.bloom.strength);
            m_bloomThreshold.value(settings.bloom.threshold);

            m_adaptiveExposureEnabled.value<ButtonToggle>(static_cast<ButtonToggle>(settings.adaptiveExposure.enabled));
            m_targetLuminance.value(settings.adaptiveExposure.targetLuminance);
            m_adaptationRate.value(settings.adaptiveExposure.adaptationRate);
            m_minExposure.value(settings.adaptiveExposure.minExposure);
            m_maxExposure.value(settings.adaptiveExposure.maxExposure);

            m_vignetteEnabled.value<ButtonToggle>(static_cast<ButtonToggle>(settings.vignette.enabled));
            m_vignetteInnerRadius.value(settings.vignette.innerRadius);
            m_vignetteOuterRadius.value(settings.vignette.outerRadius);
            m_vignetteOpacity.value(settings.vignette.opacity);

            m_chromaticAberrationEnabled.value<ButtonToggle>(static_cast<ButtonToggle>(settings.chromaticAberration.enabled));
            m_chromaticAberrationStrength.value(settings.chromaticAberration.strength);
        }

        /*bool bloomEnabled() const
        {
            return static_cast<bool>(m_bloomEnabled.value<ButtonToggle>());
        }

        void bloomEnabled(bool value)
        {
            m_bloomEnabled.value<ButtonToggle>(static_cast<ButtonToggle>(value));
        }

        float bloomStrength() const
        {
            return m_bloomStrength.value<float>();
        }

        void bloomStrength(float strength)
        {
            m_bloomStrength.value(strength);
        }

        float bloomThreshold() const
        {
            return m_bloomThreshold.value<float>();
        }

        void bloomThreshold(float value)
        {
            m_bloomThreshold.value(value);
        }

        void targetLuminance(float value)
        {
            m_targetLuminance.value(value);
        }

        float targetLuminance() const
        {
            return m_targetLuminance.value<float>();
        }

        void adaptationRate(float value)
        {
            m_adaptationRate.value(value);
        }

        float adaptationRate() const
        {
            return m_adaptationRate.value<float>();
        }

        void minExposure(float value)
        {
            m_minExposure.value(value);
        }

        float minExposure() const
        {
            return m_minExposure.value<float>();
        }

        void maxExposure(float value)
        {
            m_maxExposure.value(value);
        }

        float maxExposure() const
        {
            return m_maxExposure.value<float>();
        }*/

        engine::shared_ptr<engine::EngineComponent> clone() const override
        {
            auto postProcessComponent = engine::make_shared<engine::PostprocessComponent>();
            postProcessComponent->settings(settings());
            return postProcessComponent;
        }
    };
}
