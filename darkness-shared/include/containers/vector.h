#pragma once

#include <vector>
//#include "EASTL/vector.h"

namespace engine
{
	template<typename T>
	//using vector = eastl::vector<T>;
	using vector = std::vector<T>;
}
