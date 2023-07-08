#include "engine/rendering/tools/ExtractAlphaMask.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
	ExtractAlphaMask::ExtractAlphaMask(Device& device)
		: m_device{ device }
		, m_extractAlphaMask{ device.createPipeline<shaders::ExtractAlbedoAlphaMask>() }
		, m_extractAlphaMaskRest{ device.createPipeline<shaders::ExtractAlbedoAlphaMaskRest>() }
	{
		m_extractAlphaMask.cs.threshold = 0.35f;
	}

	TextureSRVOwner ExtractAlphaMask::extract(TextureSRVOwner src)
	{
		TextureOwner result;
		auto cmd = m_device.createCommandList("Extract alpha mask cmd list");
		{
			CPU_MARKER(cmd.api(), "Extract alpha mask");
			GPU_MARKER(cmd, "Extract alpha mask");

			auto res = src.resource();
			auto mips = res.texture().mipLevels();
			result = m_device.createTexture(TextureDescription()
				.width(std::max(roundUpToMultiple(res.width(), 4ull) / 4ull, 1ull))
				.height(std::max(roundUpToMultiple(res.height(), 4ull) / 4ull, 1ull))
				.format(Format::R16_UINT)
				.mipLevels(res.texture().mipLevels())
				.name("AlphaMask")
				.usage(ResourceUsage::GpuReadWrite));
			auto resultSRV = m_device.createTextureSRV(src, SubResource{ 0, 1, 0u, 1 });

			auto size = Uint2{ static_cast<uint32_t>(res.width()), static_cast<uint32_t>(res.height()) };
			{
				// first level. ie. float4 -> uint8 bitmask
				m_extractAlphaMask.cs.size = { size.x, size.y };
				m_extractAlphaMask.cs.input = resultSRV;
				m_extractAlphaMask.cs.output = m_device.createTextureUAV(result, SubResource{ 0, 1, 0u, 1 });
				cmd.bindPipe(m_extractAlphaMask);
				cmd.dispatch(
					std::max(roundUpToMultiple(size.x, 8) / 8, 1u),
					std::max(roundUpToMultiple(size.y, 8) / 8, 1u),
					1);
			}

			size.x = std::max(size.x >> 2, 1u);
			size.y = std::max(size.y >> 2, 1u);
			for (auto mip = 1u; mip < mips; ++mip)
			{
				m_extractAlphaMaskRest.cs.input = m_device.createTextureSRV(result, SubResource{ mip - 1, 1, 0u, 1 });
				m_extractAlphaMaskRest.cs.output = m_device.createTextureUAV(result, SubResource{ mip, 1, 0u, 1 });
				cmd.bindPipe(m_extractAlphaMaskRest);
				cmd.dispatch(
					std::max(roundUpToMultiple(std::max(size.x >> 1, 1u), 8) / 8, 1u),
					std::max(roundUpToMultiple(std::max(size.y >> 1, 1u), 8) / 8, 1u),
					1);

				size.x = std::max(size.x >> 1, 1u);
				size.y = std::max(size.y >> 1, 1u);
			}
		}
		m_device.submitBlocking(cmd);

		return m_device.createTextureSRV(result);
	}
}
