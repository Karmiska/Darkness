#pragma once

#include <algorithm>

namespace engine
{
	template<typename... Args>
	auto find_if(Args&& ... args) -> decltype(find_if(std::forward<Args>(args)...))
	{
		return std::find_if(std::forward<Args>(args)...);
	}
}
