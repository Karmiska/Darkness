#include "engine/graphics/dx12/DX12ResourceDebug.h"

namespace engine
{
	ResourceDebug resourceDebug;

	void ResourceDebug::registerResource(const char* name, void* ptr)
	{
		m_map[ptr] = name;
	}

	void ResourceDebug::unregisterResource(void* ptr)
	{
		m_map.erase(ptr);
	}

	const char* ResourceDebug::name(void* ptr) const
	{
		
		auto found = m_map.find(ptr);
		if (found != m_map.end())
			return found->second;
		else
			return "";
	}
}
