#include "tools/Octree.h"

using namespace engine;

namespace engine
{
#ifdef DEBUG_OCTREE
    engine::vector<engine::BoundingBox> usedBoundingBoxes;
    engine::vector<engine::BoundingBox> searchBoundingBoxes;
    engine::vector<engine::BoundingBox> initialSearchBoundingBoxes;
#endif
}
