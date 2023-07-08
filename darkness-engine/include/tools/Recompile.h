#pragma once

#include "engine/graphics/ShaderSupport.h"
#include "containers/string.h"
#include "containers/vector.h"

namespace engine
{
    engine::string permutationName(const engine::string& filename, int permutationId);
    engine::string permutationName(int permutationId);
    engine::string recompile(const implementation::ShaderSupport& support, int permutationId, const engine::vector<engine::string>& defines);
}
