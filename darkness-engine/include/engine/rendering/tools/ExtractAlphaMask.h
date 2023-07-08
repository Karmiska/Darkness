#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/ResourceOwners.h"
#include "shaders/core/tools/ExtractAlbedoAlphaMask.h"
#include "shaders/core/tools/ExtractAlbedoAlphaMaskRest.h"

namespace engine
{
	class CommandList;
	class ExtractAlphaMask
	{
	public:
		ExtractAlphaMask(Device& device);
		TextureSRVOwner extract(TextureSRVOwner src);
	private:
		Device& m_device;
		engine::Pipeline<shaders::ExtractAlbedoAlphaMask> m_extractAlphaMask;
		engine::Pipeline<shaders::ExtractAlbedoAlphaMaskRest> m_extractAlphaMaskRest;
	};
}
