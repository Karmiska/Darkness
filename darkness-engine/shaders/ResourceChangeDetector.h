#pragma once

#include "engine/graphics/Resources.h"

namespace engine
{
	namespace shaders
	{
		template<typename T>
		class ResourceChangeDetector
		{
		public:
			ResourceChangeDetector& operator=(T resource)
			{
				m_resource = resource;
			}

			operator T()
			{
				return m_resource;
			}
		private:
			T m_resource;
		};
	}
}
