#include "engine/graphics/SamplerDescription.h"
#include "shaders/ShaderTypes.h"

namespace engine
{
	SamplerDescription::SamplerDescription()
		: desc{
		Filter::Bilinear,
		TextureAddressMode::Wrap,
		TextureAddressMode::Wrap,
		TextureAddressMode::Wrap,
		0.0f,
		16,
		ComparisonFunction::LessEqual,
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		0.0f,
		FLT_MAX
	}
	{}

	SamplerDescription& SamplerDescription::textureAddressMode(TextureAddressMode mode)
	{
		desc.addressU = mode;
		desc.addressV = mode;
		desc.addressW = mode;
		return *this;
	}

	SamplerDescription& SamplerDescription::filter(Filter samplerFilter)
	{
		desc.filter = samplerFilter;
		return *this;
	}

	SamplerDescription& SamplerDescription::addressU(TextureAddressMode mode)
	{
		desc.addressU = mode;
		return *this;
	}

	SamplerDescription& SamplerDescription::addressV(TextureAddressMode mode)
	{
		desc.addressV = mode;
		return *this;
	}

	SamplerDescription& SamplerDescription::addressW(TextureAddressMode mode)
	{
		desc.addressW = mode;
		return *this;
	}

	SamplerDescription& SamplerDescription::mipLODBias(float bias)
	{
		desc.mipLODBias = bias;
		return *this;
	}

	SamplerDescription& SamplerDescription::maxAnisotrophy(unsigned int anisotrophy)
	{
		desc.maxAnisotrophy = anisotrophy;
		return *this;
	}

	SamplerDescription& SamplerDescription::comparisonFunc(ComparisonFunction func)
	{
		desc.comparisonFunc = func;
		return *this;
	}

	SamplerDescription& SamplerDescription::minLOD(float lod)
	{
		desc.minLOD = lod;
		return *this;
	}

	SamplerDescription& SamplerDescription::maxLOD(float lod)
	{
		desc.maxLOD = lod;
		return *this;
	}

	SamplerDescription& SamplerDescription::borderColor(float r, float g, float b, float a)
	{
		desc.borderColor[0] = r;
		desc.borderColor[1] = g;
		desc.borderColor[2] = b;
		desc.borderColor[3] = a;
		return *this;
	}

	uint64_t SamplerDescription::hash() const
	{
		uint64_t hash = shaders::ResourceHashSeed;

		const uint64_t* rasterizerStatePtr = reinterpret_cast<const uint64_t*>(&desc);
		int qw = sizeof(SamplerDescription::description) / 8;
		int leftBytes = sizeof(SamplerDescription::description) - (qw * 8);
		for (int i = 0; i < qw; ++i)
		{
			hash = shaders::fnv1aHash(*rasterizerStatePtr, hash);
			++rasterizerStatePtr;
		}
		if (leftBytes == 1)
			hash = shaders::fnv1aHash(*reinterpret_cast<const uint8_t*>(rasterizerStatePtr), hash);
		else if (leftBytes == 2)
			hash = shaders::fnv1aHash(*reinterpret_cast<const uint16_t*>(rasterizerStatePtr), hash);
		else if (leftBytes == 4)
			hash = shaders::fnv1aHash(*reinterpret_cast<const uint32_t*>(rasterizerStatePtr), hash);
		return hash;
	}
}
