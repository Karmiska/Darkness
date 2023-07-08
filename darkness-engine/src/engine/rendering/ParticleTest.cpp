#include "engine/rendering/ParticleTest.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/SamplerDescription.h"
#include "components/Camera.h"
#include "tools/image/Image.h"
#include "containers/string.h"
#include <sstream>
#include <iomanip>

using namespace engine;

namespace engine
{
    ParticleTest::ParticleTest(Device& device)
        : m_particlePipeline{ device.createPipeline<shaders::Particle>() }
        , m_frameTimeIncrease{ 0.0f }
        , m_currentFrame{ 0 }
    {
        m_particlePipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
        m_particlePipeline.setRasterizerState(RasterizerDescription()
            .frontCounterClockwise(false)
            .cullMode(CullMode::Back));

        m_particlePipeline.ps.particleSampler = device.createSampler(SamplerDescription().filter(Filter::Bilinear));
        m_particlePipeline.ps.triSampler = device.createSampler(SamplerDescription().filter(Filter::Trilinear));
        m_particlePipeline.setBlendState(BlendDescription().renderTarget(
            0, RenderTargetBlendDescription()
            .blendEnable(true)

            .srcBlend(Blend::SrcAlpha)
            .dstBlend(Blend::InvSrcAlpha)
            .blendOp(BlendOperation::Add)

            .srcBlendAlpha(Blend::InvSrcAlpha)
            .dstBlendAlpha(Blend::Zero)
            .blendOpAlpha(BlendOperation::Add)

            .renderTargetWriteMask(1 | 2 | 4 | 8)
        ));
        m_particlePipeline.ps.shadowSampler = device.createSampler(SamplerDescription()
            .addressU(TextureAddressMode::Mirror)
            .addressV(TextureAddressMode::Mirror)
            .filter(Filter::Comparison));

        engine::vector<Vector3f> particlePositions;
        particlePositions.emplace_back(Vector3f{ 0.0f, 0.0f, 0.0f });
        for (int i = 0; i < 1000; ++i)
        {
            particlePositions.emplace_back(Vector3f{
                (((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f) - 1.0f) * 80.0f,
                (((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f) - 1.0f) * 80.0f,
                (((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f) - 1.0f) * 80.0f
                });
        }


        m_lastTime = std::chrono::high_resolution_clock::now();
        createParticles(device, particlePositions);
        loadTextures(device);
    }

    engine::string ParticleTest::createFilename(engine::string basename, engine::string extension, int number, int digits)
    {
		std::stringstream ss;
        ss << std::setw(digits) << std::setfill('0') << number;
        return basename + engine::string(ss.str().c_str()) + extension;
    }

    void ParticleTest::createParticles(Device& device, const engine::vector<Vector3f>& positions)
    {
        m_particles = device.createBufferSRV(BufferDescription()
            .name("Particle buffer")
            .format(Format::R32G32B32_FLOAT)
            .setInitialData(BufferDescription::InitialData(positions)));
    }

    void ParticleTest::loadTextures(Device& device)
    {
        string formatExtension = ".tga";

        //string dataPath = "C:\\Users\\Aleksi Jokinen\\Documents\\TestDarknessProject\\processed\\particle_data\\";
        string dataPath = "C:\\Users\\aleks\\Documents\\TestDarknessProject\\processed\\particle_data\\";
        string diffuseBaseName = "diffuse_";
        string normalBaseName = "normal_";
        string alphaBaseName = "alpha_";

        string topBaseName = "render_billow_smoke_hd_top_";
        string leftBaseName = "render_billow_smoke_hd_left_";
        string bottomBaseName = "render_billow_smoke_hd_bottom_";
        string rightBaseName = "render_billow_smoke_hd_right_";

        for (int i = 0; i < 40; ++i)
        {
            auto fullDiffusePath = dataPath + createFilename(diffuseBaseName, formatExtension, i, 5);
            m_diffuse.emplace_back(loadTexture(device, fullDiffusePath));

            auto fullNormalPath = dataPath + createFilename(normalBaseName, formatExtension, i, 5);
            m_normal.emplace_back(loadTexture(device, fullNormalPath));

            auto fullAlphaPath = dataPath + createFilename(alphaBaseName, formatExtension, i, 5);
            m_alpha.emplace_back(loadTexture(device, fullAlphaPath));

            auto topPath = dataPath + createFilename(topBaseName, formatExtension, i, 5);
            m_top.emplace_back(loadTexture(device, topPath));

            auto leftPath = dataPath + createFilename(leftBaseName, formatExtension, i, 5);
            m_left.emplace_back(loadTexture(device, leftPath));

            auto bottomPath = dataPath + createFilename(bottomBaseName, formatExtension, i, 5);
            m_bottom.emplace_back(loadTexture(device, bottomPath));

            auto rightPath = dataPath + createFilename(rightBaseName, formatExtension, i, 5);
            m_right.emplace_back(loadTexture(device, rightPath));
        }

        m_particlePipeline.ps.diffuse = device.createBindlessTextureSRV();
        m_particlePipeline.ps.normal = device.createBindlessTextureSRV();
        m_particlePipeline.ps.alpha = device.createBindlessTextureSRV();
        m_particlePipeline.ps.top = device.createBindlessTextureSRV();
        m_particlePipeline.ps.left = device.createBindlessTextureSRV();
        m_particlePipeline.ps.bottom = device.createBindlessTextureSRV();
        m_particlePipeline.ps.right = device.createBindlessTextureSRV();

        for (int i = 0; i < 40; ++i)
        {
            m_particlePipeline.ps.diffuse.push(m_diffuse[i]);
            m_particlePipeline.ps.normal.push(m_normal[i]);
            m_particlePipeline.ps.alpha.push(m_alpha[i]);
            m_particlePipeline.ps.top.push(m_top[i]);
            m_particlePipeline.ps.left.push(m_left[i]);
            m_particlePipeline.ps.bottom.push(m_bottom[i]);
            m_particlePipeline.ps.right.push(m_right[i]);
        }
    }

    TextureSRVOwner ParticleTest::loadTexture(Device& device, string filepath)
    {
        auto image = image::Image::createImage(filepath, image::ImageType::DDS);
        return device.createTextureSRV(TextureDescription()
            .name("Particle texture")
            .width(static_cast<uint32_t>(image->width()))
            .height(static_cast<uint32_t>(image->height()))
            .format(srgbFormat(image->format()))
            .arraySlices(static_cast<uint32_t>(image->arraySlices()))
            .mipLevels(static_cast<uint32_t>(image->mipCount()))
            .setInitialData(TextureDescription::InitialData(
                tools::ByteRange(image->data(), image->data() + image->bytes()),
                static_cast<uint32_t>(image->width()), static_cast<uint32_t>(image->width() * image->height()))));
    }

    void ParticleTest::render(
        Device& /*device*/,
        CommandList& cmd, 
        TextureRTV rtv, 
        TextureSRV dsvSRV, 
        Camera& camera,
        const Matrix4f& cameraProjectionMatrix,
        const Matrix4f& cameraViewMatrix,
        const Matrix4f& jitterMatrix,
        LightData& lights,
        TextureSRV shadowMap,
        BufferSRV shadowVP)
    {
        CPU_MARKER(cmd.api(), "Particles");
        GPU_MARKER(cmd, "Particles");

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastTime).count()) / 1000000000.0;
        m_lastTime = now;
        m_frameTimeIncrease += static_cast<float>(duration);
        auto frameChangeTime = 1.0f / 60.0f;
        auto wholeFrames = m_frameTimeIncrease / frameChangeTime;
        auto integerFrames = static_cast<int>(wholeFrames);
        if (integerFrames > 0)
        {
            m_frameTimeIncrease -= integerFrames * frameChangeTime;
            m_currentFrame += integerFrames;
            if (m_currentFrame > 39)
            {
                m_currentFrame = 0;
            }
        }

        const auto cameraTransform = cameraViewMatrix * Matrix4f::identity();

        m_particlePipeline.ps.lightCount.x = static_cast<unsigned int>(lights.count());
        if (lights.count() > 0)
        {
            m_particlePipeline.ps.lightWorldPosition = lights.worldPositions();
            m_particlePipeline.ps.lightDirection = lights.directions();
            m_particlePipeline.ps.lightColor = lights.colors();
            m_particlePipeline.ps.lightIntensity = lights.intensities();
            m_particlePipeline.ps.lightRange = lights.ranges();
            m_particlePipeline.ps.lightType = lights.types();
            m_particlePipeline.ps.lightParameters = lights.parameters();
        }

        m_particlePipeline.ps.shadowSize = Float2{ 1.0f / static_cast<float>(ShadowMapWidth), 1.0f / static_cast<float>(ShadowMapHeight) };
        m_particlePipeline.ps.shadowMap = shadowMap;
        m_particlePipeline.ps.shadowVP = shadowVP;

        m_particlePipeline.vs.jitterModelViewProjectionMatrix = fromMatrix(jitterMatrix * (cameraProjectionMatrix * cameraTransform));
        m_particlePipeline.vs.modelMatrix = fromMatrix(Matrix4f::identity());
        m_particlePipeline.vs.cameraPosition = Vector4f(camera.position(), 1.0f);
        m_particlePipeline.vs.particleSize.x = 10.0f;
        m_particlePipeline.vs.particleSize.y = 10.0f;
        m_particlePipeline.vs.positions = m_particles;

        m_particlePipeline.ps.cameraInverseProjectionMatrix = fromMatrix(cameraProjectionMatrix.inverse());
        m_particlePipeline.ps.cameraInverseViewMatrix = fromMatrix(cameraViewMatrix.inverse());
        m_particlePipeline.ps.cameraPosition = Vector4f(camera.position(), 1.0f);
        m_particlePipeline.ps.inverseSize.x = 1.0f / static_cast<float>(camera.width());
        m_particlePipeline.ps.inverseSize.y = 1.0f / static_cast<float>(camera.height());
        m_particlePipeline.ps.animationIndex.x = m_currentFrame % 40;
        m_particlePipeline.ps.depth = dsvSRV;

        // environment
        m_particlePipeline.ps.environmentStrength = camera.environmentMapStrength();
        if (camera.environmentIrradiance().valid() && camera.environmentIrradiance().texture().arraySlices() == 1)
        {
            m_particlePipeline.ps.environmentIrradiance = camera.environmentIrradiance();
            m_particlePipeline.ps.environmentIrradianceCubemap = TextureSRV();
            m_particlePipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(false);
            m_particlePipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(true);
        }
        else
        {
            m_particlePipeline.ps.environmentIrradiance = TextureSRV();
            m_particlePipeline.ps.environmentIrradianceCubemap = camera.environmentIrradiance();
            m_particlePipeline.ps.hasEnvironmentIrradianceCubemap.x = static_cast<unsigned int>(true);
            m_particlePipeline.ps.hasEnvironmentIrradianceEquirect.x = static_cast<unsigned int>(false);
        }
        m_particlePipeline.ps.environmentSpecular = camera.environmentSpecular();
        m_particlePipeline.ps.environmentBrdfLut = camera.environmentBrdfLUT();
        m_particlePipeline.ps.hasEnvironmentSpecular.x = camera.environmentSpecular().valid();


        cmd.setRenderTargets({ rtv });
        cmd.bindPipe(m_particlePipeline);
        cmd.draw(m_particles.resource().desc().elements * 6);
    }

}
