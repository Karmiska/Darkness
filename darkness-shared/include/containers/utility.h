#pragma once

#include <utility>
//#include "EASTL/string.h"

namespace engine
{
	//using string = eastl::string;
	template<typename T, typename A>
	using pair = std::pair<T, A>;
}
