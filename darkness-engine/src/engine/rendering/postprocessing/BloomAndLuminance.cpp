#include "engine/rendering/postprocessing/BloomAndLuminance.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    BloomAndLuminance::BloomAndLuminance(Device& device)
        : m_device{ device }
        , m_extractBloomLuminance{ device.createPipeline<shaders::ExtractBloomLuminance>() }
        , m_extractLuminance{ device.createPipeline<shaders::ExtractLuminance>() }
        , m_downSampleBloom4{ device.createPipeline<shaders::DownsampleBloom4>() }
        , m_upsampleAndBlur{ device.createPipeline<shaders::UpsampleAndBlur>() }
        , m_blur{ device.createPipeline<shaders::BloomBlur>() }
    {
        m_extractBloomLuminance.cs.BiLinearClamp = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));
        m_extractLuminance.cs.BiLinearClamp = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));
        m_downSampleBloom4.cs.BiLinearClamp = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear).textureAddressMode(TextureAddressMode::Clamp));
        m_upsampleAndBlur.cs.LinearBorder = device.createSampler(SamplerDescription().filter(engine::Filter::Trilinear).textureAddressMode(TextureAddressMode::Border).borderColor(0.0f, 0.0f, 0.0f, 0.0f));
    }

    void BloomAndLuminance::extract(
        CommandList& cmd,
        TextureSRV src,
        Exposure& exposure,
        bool bloomEnabled,
        float threshold)
    {
        refreshInternalTextures(src, bloomEnabled);

        if (bloomEnabled)
        {
            {
                CPU_MARKER(cmd.api(), "Extract bloom and luminance");
                GPU_MARKER(cmd, "Extract bloom and luminance");

                auto bloomResource = m_bloomUAV[0][0].resource();
                m_extractBloomLuminance.cs.source = src;
                m_extractBloomLuminance.cs.Exposure = exposure.exposureSRV();
                m_extractBloomLuminance.cs.BloomResult = bloomResource;
                m_extractBloomLuminance.cs.LumaResult = m_lumaLRUAV;
                m_extractBloomLuminance.cs.inverseOutputSize = Vector2f{ 1.0f / (float)bloomResource.width(), 1.0f / (float)bloomResource.height() };
                m_extractBloomLuminance.cs.bloomThreshold = threshold;

                cmd.bindPipe(m_extractBloomLuminance);
                cmd.dispatch(
                    roundUpToMultiple(bloomResource.width(), 8u) / 8u,
                    roundUpToMultiple(bloomResource.height(), 8u) / 8u,
                    1u);
            }

            {
                CPU_MARKER(cmd.api(), "Downsample bloom");
                GPU_MARKER(cmd, "Downsample bloom");

                auto bloomResource = m_bloomUAV[0][0].resource();
                m_downSampleBloom4.cs.BloomBuf = m_bloomSRV[0][0];
                m_downSampleBloom4.cs.Result1 = m_bloomUAV[1][0];
                m_downSampleBloom4.cs.Result2 = m_bloomUAV[2][0];
                m_downSampleBloom4.cs.Result3 = m_bloomUAV[3][0];
                m_downSampleBloom4.cs.Result4 = m_bloomUAV[4][0];
                m_downSampleBloom4.cs.g_inverseDimensions = { 1.0f / (float)bloomResource.width(), 1.0f / (float)bloomResource.height() };
                cmd.bindPipe(m_downSampleBloom4);
                cmd.dispatch(
                    roundUpToMultiple(bloomResource.width() / 2u, 8u) / 8u,
                    roundUpToMultiple(bloomResource.height() / 2u, 8u) / 8u,
                    1u);
            }

            {
                CPU_MARKER(cmd.api(), "Bloom blur pass");
                GPU_MARKER(cmd, "Bloom blur pass");

                auto blur = [&](TextureSRV textureRead, TextureUAV textureWrite, TextureSRV lowRes)
                {
                    if (textureRead == lowRes)
                    {
                        CPU_MARKER(cmd.api(), "Bloom blur");
                        GPU_MARKER(cmd, "Bloom blur");

                        m_blur.cs.InputBuf = textureRead;
                        m_blur.cs.Result = textureWrite;
                        m_blur.cs.g_inverseDimensions = { 1.0f / (float)textureRead.width(), 1.0f / (float)textureRead.height() };
                        cmd.bindPipe(m_blur);
                        cmd.dispatch(
                            roundUpToMultiple(textureRead.width(), 8u) / 8u,
                            roundUpToMultiple(textureRead.height(), 8u) / 8u,
                            1u);
                    }
                    else
                    {
                        CPU_MARKER(cmd.api(), "Bloom blur and upsample");
                        GPU_MARKER(cmd, "Bloom blur and upsample");

                        m_upsampleAndBlur.cs.HigherResBuf = textureRead;
                        m_upsampleAndBlur.cs.LowerResBuf = lowRes;
                        m_upsampleAndBlur.cs.Result = textureWrite;
                        m_upsampleAndBlur.cs.g_inverseDimensions = { 1.0f / (float)textureRead.width(), 1.0f / (float)textureRead.height() };
                        m_upsampleAndBlur.cs.g_upsampleBlendFactor = 0.65f;

                        cmd.bindPipe(m_upsampleAndBlur);
                        cmd.dispatch(
                            roundUpToMultiple(textureRead.width(), 8u) / 8u,
                            roundUpToMultiple(textureRead.height(), 8u) / 8u,
                            1u);
                    }
                };

                blur(m_bloomSRV[4][0], m_bloomUAV[4][1], m_bloomSRV[4][0]);
                blur(m_bloomSRV[3][0], m_bloomUAV[3][1], m_bloomSRV[4][1]);
                blur(m_bloomSRV[2][0], m_bloomUAV[2][1], m_bloomSRV[3][1]);
                blur(m_bloomSRV[1][0], m_bloomUAV[1][1], m_bloomSRV[2][1]);
                blur(m_bloomSRV[0][0], m_bloomUAV[0][1], m_bloomSRV[1][1]);

            }
        }
        else
        {
            CPU_MARKER(cmd.api(), "Extract luminance");
            GPU_MARKER(cmd, "Extract luminance");

            auto resource = m_lumaLRUAV.resource();
            m_extractLuminance.cs.source = src;
            m_extractLuminance.cs.Exposure = exposure.exposureSRV();
            m_extractLuminance.cs.LumaResult = resource;
            m_extractLuminance.cs.inverseOutputSize = Vector2f{ 1.0f / (float)resource.width(), 1.0f / (float)resource.height() };

            cmd.bindPipe(m_extractLuminance);
            cmd.dispatch(
                roundUpToMultiple(resource.width(), 8u) / 8u,
                roundUpToMultiple(resource.height(), 8u) / 8u,
                1u);
        }
    }

    TextureUAV BloomAndLuminance::lumaLRUAV()
    {
        return m_lumaLRUAV;
    }

    TextureSRV BloomAndLuminance::lumaLRSRV()
    {
        return m_lumaLRSRV;
    }

    /*engine::vector<engine::vector<TextureUAV>>& BloomAndLuminance::bloomUAV()
    {
        return m_bloomUAVHandles;
    }

    engine::vector<engine::vector<TextureSRV>>& BloomAndLuminance::bloomSRV()
    {
        return m_bloomSRVHandles;
    }*/

    TextureSRV BloomAndLuminance::bloom()
    {
        if(m_bloomSRV.size() > 0)
            return m_bloomSRV[0][1];
        return TextureSRV();
    }

    void BloomAndLuminance::refreshInternalTextures(TextureSRV src, bool bloomEnabled)
    {
        auto bloomWidth =  std::max(src.width()  >> 1ull, 1ull);// > 2560 ? 1280 : 640;
        auto bloomHeight = std::max(src.height() >> 1ull, 1ull);// > 1440 ? 768 : 384;

        if (!m_lumaLRUAV.resource().valid() ||
            m_lumaLRUAV.resource().width() != bloomWidth ||
            m_lumaLRUAV.resource().height() != bloomHeight)
        {
            // create luma texture
            m_lumaLRUAV = m_device.createTextureUAV(TextureDescription()
                .width(bloomWidth)
                .height(bloomHeight)
                .format(Format::R8_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("Luminance texture")
                .dimension(ResourceDimension::Texture2D));
            m_lumaLRSRV = m_device.createTextureSRV(m_lumaLRUAV);
        }

        if (bloomEnabled && (m_bloomUAV.size() == 0 ||
            !m_bloomUAV[0][0].resource().valid() ||
            m_bloomUAV[0][0].resource().width() != bloomWidth ||
            m_bloomUAV[0][0].resource().height() != bloomHeight))
        {
            {
                // create bloom textures
                m_bloomUAV.clear();
                m_bloomSRV.clear();
                m_bloomUAVHandles.clear();
                m_bloomSRVHandles.clear();

                m_bloomUAV.resize(5);
                m_bloomSRV.resize(5);
                m_bloomUAVHandles.resize(5);
                m_bloomSRVHandles.resize(5);

                auto bloomDesc = TextureDescription()
                    .width(bloomWidth)
                    .height(bloomHeight)
                    .format(Format::R11G11B10_FLOAT)
                    .usage(ResourceUsage::GpuReadWrite)
                    .name("Bloom Texture")
                    .dimension(ResourceDimension::Texture2D);

                m_bloomUAV[0].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 1a")));
                m_bloomUAV[0].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 1b")));
                m_bloomUAV[1].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 2a").width(std::max(bloomWidth / 2ull, 1ull)).height(std::max(bloomHeight / 2ull, 1ull))));
                m_bloomUAV[1].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 2b").width(std::max(bloomWidth / 2ull, 1ull)).height(std::max(bloomHeight / 2ull, 1ull))));
                m_bloomUAV[2].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 3a").width(std::max(bloomWidth / 4ull, 1ull)).height(std::max(bloomHeight / 4ull, 1ull))));
                m_bloomUAV[2].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 3b").width(std::max(bloomWidth / 4ull, 1ull)).height(std::max(bloomHeight / 4ull, 1ull))));
                m_bloomUAV[3].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 4a").width(std::max(bloomWidth / 8ull, 1ull)).height(std::max(bloomHeight / 8ull, 1ull))));
                m_bloomUAV[3].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 4b").width(std::max(bloomWidth / 8ull, 1ull)).height(std::max(bloomHeight / 8ull, 1ull))));
                m_bloomUAV[4].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 5a").width(std::max(bloomWidth / 16ull, 1ull)).height(std::max(bloomHeight / 16ull, 1ull))));
                m_bloomUAV[4].emplace_back(m_device.createTextureUAV(bloomDesc.name("Bloom Texture 5b").width(std::max(bloomWidth / 16ull, 1ull)).height(std::max(bloomHeight / 16ull, 1ull))));

                m_bloomUAVHandles[0].emplace_back(m_bloomUAV[0][0]);
                m_bloomUAVHandles[0].emplace_back(m_bloomUAV[0][1]);
                m_bloomUAVHandles[1].emplace_back(m_bloomUAV[1][0]);
                m_bloomUAVHandles[1].emplace_back(m_bloomUAV[1][1]);
                m_bloomUAVHandles[2].emplace_back(m_bloomUAV[2][0]);
                m_bloomUAVHandles[2].emplace_back(m_bloomUAV[2][1]);
                m_bloomUAVHandles[3].emplace_back(m_bloomUAV[3][0]);
                m_bloomUAVHandles[3].emplace_back(m_bloomUAV[3][1]);
                m_bloomUAVHandles[4].emplace_back(m_bloomUAV[4][0]);
                m_bloomUAVHandles[4].emplace_back(m_bloomUAV[4][1]);

                m_bloomSRV[0].emplace_back(m_device.createTextureSRV(m_bloomUAV[0][0]));
                m_bloomSRV[0].emplace_back(m_device.createTextureSRV(m_bloomUAV[0][1]));
                m_bloomSRV[1].emplace_back(m_device.createTextureSRV(m_bloomUAV[1][0]));
                m_bloomSRV[1].emplace_back(m_device.createTextureSRV(m_bloomUAV[1][1]));
                m_bloomSRV[2].emplace_back(m_device.createTextureSRV(m_bloomUAV[2][0]));
                m_bloomSRV[2].emplace_back(m_device.createTextureSRV(m_bloomUAV[2][1]));
                m_bloomSRV[3].emplace_back(m_device.createTextureSRV(m_bloomUAV[3][0]));
                m_bloomSRV[3].emplace_back(m_device.createTextureSRV(m_bloomUAV[3][1]));
                m_bloomSRV[4].emplace_back(m_device.createTextureSRV(m_bloomUAV[4][0]));
                m_bloomSRV[4].emplace_back(m_device.createTextureSRV(m_bloomUAV[4][1]));

                m_bloomSRVHandles[0].emplace_back(m_bloomSRV[0][0]);
                m_bloomSRVHandles[0].emplace_back(m_bloomSRV[0][1]);
                m_bloomSRVHandles[1].emplace_back(m_bloomSRV[1][0]);
                m_bloomSRVHandles[1].emplace_back(m_bloomSRV[1][1]);
                m_bloomSRVHandles[2].emplace_back(m_bloomSRV[2][0]);
                m_bloomSRVHandles[2].emplace_back(m_bloomSRV[2][1]);
                m_bloomSRVHandles[3].emplace_back(m_bloomSRV[3][0]);
                m_bloomSRVHandles[3].emplace_back(m_bloomSRV[3][1]);
                m_bloomSRVHandles[4].emplace_back(m_bloomSRV[4][0]);
                m_bloomSRVHandles[4].emplace_back(m_bloomSRV[4][1]);
            }
        }
        
        if(!bloomEnabled)
        {
            m_bloomUAV.clear();
            m_bloomSRV.clear();
            m_bloomUAVHandles.clear();
            m_bloomSRVHandles.clear();
        }
    }
}
