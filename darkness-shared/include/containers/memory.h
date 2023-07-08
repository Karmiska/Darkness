#pragma once

#include <memory>
//#include "containers/RombautsSharedPtr.h"
//#include "containers/RombautsUniquePtr.h"

namespace engine
{
	//template<typename T>
	//using fast_shared_ptr = ::shared_ptr<T>;

	template<typename T>
	using shared_ptr = std::shared_ptr<T>;

	template<typename T, class Deleter = std::default_delete<T>>
	using unique_ptr = std::unique_ptr<T, Deleter>;

	template<typename T, typename... Args>
	auto make_shared(Args&& ... args) -> decltype(std::make_shared<T>(std::forward<Args>(args)...))
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	auto make_unique(Args&& ... args) -> decltype(std::make_unique<T>(std::forward<Args>(args)...))
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}
