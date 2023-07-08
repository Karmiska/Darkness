#pragma once

#include "engine/graphics/Resources.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/rendering/LightData.h"
#include "shaders/core/particle/Particle.h"
#include "containers/vector.h"
#include "containers/string.h"
#include <chrono>

namespace engine
{
    class Device;
    class ParticleTest
    {
    public:
        ParticleTest(Device& device);
        void createParticles(Device& device, const engine::vector<Vector3f>& positions);
        void render(
            Device& device,
            CommandList& cmd, 
            TextureRTV rtv, 
            TextureSRV dsvSRV,
            Camera& camera,
            const Matrix4f& cameraProjectionMatrix,
            const Matrix4f& cameraViewMatrix,
            const Matrix4f& jitterMatrix,
            LightData& lights,
            TextureSRV shadowMap,
            BufferSRV shadowVP);
    private:
        engine::Pipeline<shaders::Particle> m_particlePipeline;
        void loadTextures(Device& device);
        TextureSRVOwner loadTexture(Device& device, engine::string filepath);
        engine::string createFilename(engine::string basename, engine::string extension, int number, int digits);
        BufferSRVOwner m_particles;
        std::chrono::high_resolution_clock::time_point m_lastTime;
        float m_frameTimeIncrease;
        int m_currentFrame;

        engine::vector<TextureSRVOwner> m_diffuse;
        engine::vector<TextureSRVOwner> m_normal;
        engine::vector<TextureSRVOwner> m_alpha;
        engine::vector<TextureSRVOwner> m_top;
        engine::vector<TextureSRVOwner> m_left;
        engine::vector<TextureSRVOwner> m_bottom;
        engine::vector<TextureSRVOwner> m_right;
    };
}
