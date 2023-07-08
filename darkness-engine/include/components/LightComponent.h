#pragma once

#include "engine/EngineComponent.h"
#include "tools/Property.h"
#include "containers/memory.h"

namespace engine
{
    enum class LightType
    {
        Directional,
        Spot,
        Point
    };

    engine::string lightTypeToString(const LightType& type);
    LightType stringToLightType(const engine::string& type);

    class LightComponent : public EngineComponent
    {
    private:
        Property m_range;
        Property m_color;
        Property m_intensity;
        Property m_type;
        Property m_shadowCaster;
        LightType m_oldType;

        bool m_rangeChanged;
        bool m_colorChanged;
        bool m_intensityChanged;
        bool m_typeChanged;

        bool m_innerConeChanged;
        bool m_outerConeChanged;
        bool m_casterStatusChanged;

        engine::vector<engine::shared_ptr<Property>> m_optionals;
    public:
        LightComponent()
            : m_range{ this, "range", float(10.0f), [this]() { this->m_rangeChanged = true; } }
            , m_color{ this, "color", Vector3f{ 1.0f, 1.0f, 1.0f }, [this]() { this->m_colorChanged = true; } }
            , m_intensity{ this, "intensity", float(1.0f), [this]() { this->m_intensityChanged = true; } }
            , m_type{ this, "type", LightType::Directional, [this]()
            {
                if (this->m_oldType != this->m_type.value<LightType>())
                {
                    this->m_oldType = this->m_type.value<LightType>();
                    this->m_typeChanged = true;
                    this->populateOptionals();
                }
            } }
            , m_shadowCaster{ this, "shadowcaster", engine::ButtonToggle::NotPressed, [this]() { this->m_casterStatusChanged = true; } }
            , m_oldType{ m_type.value<LightType>() }
            , m_rangeChanged{ true }
            , m_colorChanged{ true }
            , m_intensityChanged{ true }
            , m_typeChanged{ true }
            , m_innerConeChanged{ true }
            , m_outerConeChanged{ true }
            , m_casterStatusChanged{ true }
            , m_optionals{}
        {
            m_name = "LightComponent";
            populateOptionals();
        }

        void populateOptionals()
        {
            m_optionals.clear();
            switch (m_type.value<LightType>())
            {
                case LightType::Directional:
                {
                    break;
                }
                case LightType::Spot:
                {
                    m_optionals.emplace_back(engine::make_shared<Property>( this, "inner cone angle", 30.0f, [this]() { this->m_innerConeChanged = true; } ));
                    m_optionals.emplace_back(engine::make_shared<Property>( this, "outer cone angle", 33.0f, [this]() { this->m_outerConeChanged = true; } ));

                    m_optionals[0]->setRangeCheck<float>([](float val, float& closest) { if (val < 0.0f) closest = 0.0f; if (val > 90.0f) closest = 90.0f; return val >= 0.0f && val <= 90.0f; });
                    m_optionals[1]->setRangeCheck<float>([](float val, float& closest) { if (val < 0.0f) closest = 0.0f; if (val > 90.0f) closest = 90.0f; return val >= 0.0f && val <= 90.0f; });
                    break;
                }
                case LightType::Point:
                {
                    break;
                }
            }
        }

        engine::shared_ptr<engine::EngineComponent> clone() const override
        {
            auto l = engine::make_shared<LightComponent>();
            l->range(range());
            l->color(color());
            l->intensity(intensity());
            l->lightType(lightType());
			l->shadowCaster(shadowCaster());
			l->innerConeAngle(innerConeAngle());
			l->outerConeAngle(outerConeAngle());
            return l;
        }

        float range() const
        {
            return m_range.value<float>();
        }

        void range(float val)
        {
            m_range.value<float>(val);
        }

        bool rangeChanged(bool clear = false)
        {
            bool res = m_rangeChanged;
            if (clear) m_rangeChanged = false;
            return res;
        }

        bool shadowCaster() const
        {
            return static_cast<bool>(m_shadowCaster.value<engine::ButtonToggle>());
        }

        void shadowCaster(bool caster)
        {
            m_shadowCaster.value<engine::ButtonToggle>(static_cast<engine::ButtonToggle>(caster));
        }

        Vector3f color() const
        {
            return m_color.value<Vector3f>();
        }

        void color(const Vector3f& col)
        {
            m_color.value<Vector3f>(col);
        }

        bool colorChanged(bool clear = false)
        {
            bool res = m_colorChanged;
            if (clear) m_colorChanged = false;
            return res;
        }

        float intensity() const
        {
            return m_intensity.value<float>();
        }

        void intensity(float val)
        {
            m_intensity.value<float>(val);
        }

        bool intensityChanged(bool clear = false)
        {
            bool res = m_intensityChanged;
            if (clear) m_intensityChanged = false;
            return res;
        }

        LightType lightType() const
        {
            return m_type.value<LightType>();
        }

        void lightType(LightType type)
        {
            m_type.value<LightType>(type);
        }

        bool lightTypeChanged(bool clear = false)
        {
            bool res = m_typeChanged;
            if (clear) m_typeChanged = false;
            return res;
        }

        bool lightParametersChanged(bool clear = false)
        {
            bool res = m_innerConeChanged || m_outerConeChanged || m_casterStatusChanged;
            if (clear)
            {
                m_innerConeChanged = false;
                m_outerConeChanged = false;
                m_casterStatusChanged = false;
            }
            return res;
        }

        float outerConeAngle() const
        {
            if (m_optionals.size() > 1)
            {
                return m_optionals[1]->value<float>();
            }
            return 0.0f;
        }

        float innerConeAngle() const
        {
            if (m_optionals.size() > 0)
            {
                return m_optionals[0]->value<float>();
            }
            return 0.0f;
        }

		void outerConeAngle(float value)
		{
			if (m_optionals.size() > 1)
			{
				m_optionals[1]->value<float>(value);
			}
		}

		void innerConeAngle(float value)
		{
			if (m_optionals.size() > 0)
			{
				m_optionals[0]->value<float>(value);
			}
		}

        Vector4f parameters() const
        {
            if (m_optionals.size() == 0)
                return { 0.0f, 0.0f, shadowCaster() ? 1.0f : 0.0f, 0.0f};
            else if (m_optionals.size() == 2)
            {
                float inner = m_optionals[0]->value<float>();
                float outer = m_optionals[1]->value<float>();
                if (inner > outer)
                    inner = outer;
                return {
                    inner, 
                    outer,
                    shadowCaster() ? 1.0f : 0.0f,
                    0.0f
                };
            }
            return {};
        }
    };
}
