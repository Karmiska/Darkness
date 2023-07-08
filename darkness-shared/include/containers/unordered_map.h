#pragma once

#include "containers/utility.h"
//#include "containers/bytell_hash_map.h"
#include <unordered_map>

namespace engine
{
	template<typename T, typename A, typename Hasher = std::hash<T>>
	using unordered_map = std::unordered_map<T, A, Hasher>;
	//using unordered_map = ska::bytell_hash_map<T, A>;
}
