#pragma once

#include "ecs/Ecs.h"
#include "engine/primitives/Vector4.h"

namespace game
{
    const float NormalGrowthRatePerDay = 0.254f;
    const float MaximumGrowthRatePerDay = NormalGrowthRatePerDay * 2.0f;
    const float GrowthRatePerSecond = MaximumGrowthRatePerDay / 24.0f / 60.0f / 60.0f;
    const uint64_t BladesPerSquareMeter = 110'000;

    using Transform = engine::Vector4f;

    class GrassBlade
    {
    public:
        void grow(
            float deltaSeconds, 
            float effectiveness, 
            const engine::Vector3f& sunDirection);

    private:
        engine::Vector3f m_tip{ 0.0f, 0.0f, 0.0f };
    };

    class GrassSystem
    {
    public:
        GrassSystem(size_t widthMeters, size_t heightMeters);
        void grow(float deltaSeconds);

    private:
        ecs::Ecs m_ecs;
    };
}
