#pragma once

#include "engine/primitives/Vector3.h"

namespace engine
{
	struct TerrainSettings
	{
		Vector3f worldSize;
		Vector3f sectorCount;
		Vector3f cellCount;
		Vector3f nodeCount;
		Vector3f sectorSize;
	};
}
