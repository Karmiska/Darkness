#pragma once

#include "containers/string.h"
#include "containers/unordered_map.h"

namespace engine
{
	class ResourceDebug
	{
	public:
		void registerResource(const char* name, void* ptr);
		void unregisterResource(void* ptr);

		const char* name(void* ptr) const;
	private:
		engine::unordered_map<void*, const char*> m_map;
	};

	extern ResourceDebug resourceDebug;
}
